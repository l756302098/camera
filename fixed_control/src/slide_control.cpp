/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-22 13:05:41
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 18:15:06
 */
#include "fixed_control/slide_control.hpp"

slide_control::slide_control(const ros::NodeHandle &nh):nh_(nh),clear_task_flag(false),watch_flag(false),task_pause(false)
{
    //get param
    std::string test_file,service_str;
    nh_.param<int>("robot_id", robot_id, 1);
    nh_.param<std::string>("test_file", test_file, "");
    nh_.param<std::string>("service_str", service_str, "");
    std::cout << "robot_id:" << robot_id << std::endl;
    std::cout << "test_file:" << test_file << std::endl;
    std::cout << "service_str:" << service_str << std::endl;
    transfer_pub = nh_.advertise<fixed_msg::platform_transfer>("/fixed/platform/transfer", 1);
    control_mode_pub = nh_.advertise<fixed_msg::control_mode>("/fixed/control/mode", 1);
    task_status_pub = nh_.advertise<fixed_msg::task_status>("/fixed/control/task_status", 1);
    calc_client = nh_.serviceClient<localize_msgs::TransformMapToRail>("service_str");
    if(!test_file.empty()){
        load_from_file(test_file);
    }
}

slide_control::~slide_control()
{
    road_tasks.clear();
}

void slide_control::meter_cb(const std_msgs::String msg)
{
	ROS_INFO("meter_cb ...");
    watch_flag = true;
}

bool slide_control::task_srv(fixed_msg::task::Request &req, fixed_msg::task::Response &res){
	if (!task_running) //所有任务执行完毕状态
	{
		std::string list = req.plan;
		std::cout << req.plan << std::endl;
        //std::ifstream in(req.plan,ios::in|ios::binary);
        int r = load_from_stream(list);
        if(r==-1){
            res.status = false;
        }else{
            res.status = true;
        }
        return true;
	}
	else
	{
		res.status = false;
	}
	return true;
}

bool slide_control::task_clear_srv(fixed_msg::task_control::Request &req, fixed_msg::task_control::Response &res)
{
	res.success = 0;
	if (req.flag == 1) //暂停当前任务
	{
		ROS_INFO("clear task ***");
        task_pause = true;
		res.success = 1;
	}
	else if (req.flag == 2) //清除当前任务
	{
		ROS_INFO("pause task *** ");
		clear_task_flag = true;
		res.success = 1;
	}
	else if (req.flag == 3) //继续当前任务
	{
		ROS_INFO("continue task ***");
		task_pause = false;
		res.success = 1;
	}
	return true;
}

void slide_control::tick(const ros::TimerEvent &event){
    //ROS_INFO("tick ...");
    update();
}

void slide_control::reset(){
    task_running = false;
    task_pause = false;
    clear_task_flag = false;
}

void slide_control::pub_task_status(int task_id, int task_status)
{
    //任务状态 0:已执行\1:终止\2:暂停\3:正在执行\4:未执行\5:超期\6:预执行\7:超时
	fixed_msg::task_status task_msg;
    task_msg.device_id = 0;
	task_msg.task_id = task_id;
	task_msg.task_status = task_status;
	task_status_pub.publish(task_msg);
    std::cout << "pub task status" << task_status <<  std::endl; 
}

void slide_control::transfer(float locX,float locY,float locZ, vector<string> lists, vector<string> task_camera, string trans_id, char flg)
{
    float rail_x = 0;
    float rail_y = 0;
    float rail_z = 0;
    //calc position
    geometry_msgs::Point32 pos;
    pos.x = locX;
    pos.y = locY;
    pos.z = locZ;

    localize_msgs::TransformMapToRail cmd;
    cmd.request.map_point = pos;
    if (calc_client.call(cmd))
    {
        std::cout << "call calc position success!" << std::endl;
        std::cout << "code:" << cmd.response.status.code << " message:" << cmd.response.status.message << std::endl;
        rail_x = cmd.response.rail_point.x;
        rail_y = cmd.response.rail_point.y;
        rail_z = cmd.response.rail_point.z;
    }else{
        std::cout << "call calc position failure!" << std::endl;
        return;
    }
    

	    string s_time;
	    fixed_msg::platform_transfer data;
	    double secs = ros::Time::now().toSec();
	    time_t t1 = int(secs);
	    char now[64];
	    struct tm *ttime;
	    ttime = localtime(&t1);
	    strftime(now, 64, "%Y-%m-%d %H:%M:%S", ttime);
	    s_time = now;
	    data.flag = flg;
	    data.data = s_time + "/";
	    data.data += (trans_id + "/");
	    data.data += (std::to_string(rail_x) + "," + std::to_string(rail_y)+ "," + std::to_string(rail_z) + "/");
	    std::ostringstream s;
	    if (task_camera.size() >= 2)
	    {
		    if (lists.size() == 4)																									
			    s << data.data << task_camera[0] << "/" << task_camera[1] << "/" << lists[1] << "/" << lists[2] << "/" << lists[3];
		    else if (lists.size() == 5)
			    s << data.data << task_camera[0] << "/" << task_camera[1] << "/" << lists[1] << "/" << lists[2] << "/" << lists[3] << "/" << lists[4];
		    else ROS_WARN("lists info error");
		    data.data = s.str();
	    }
        std::cout << "transfer_pub data:" << data << std::endl;
	    transfer_pub.publish(data);
}

void slide_control::pub_control_mode(){
    fixed_msg::control_mode msg;
    msg.device_id = robot_id;
	msg.mode = ctr_mode;
	control_mode_pub.publish(msg);
}

void slide_control::update(){
    //ROS_INFO("update ...");
    int state = 0;
    int cpoint = 0;
    bool is_finish = false;
    ros::Time last;
    //ROS_INFO("task size %i",road_tasks.size());
    if(road_tasks.size()==0) pub_control_mode();
    for (size_t i = 0; i < road_tasks.size(); i++){
        if(clear_task_flag) break;
        pub_control_mode();
        if(task_pause){
            pub_task_status(task_id,3);
            break;
        }
        task_running = true;
        std::shared_ptr<ROAD_PLAN> ctask = road_tasks[i];
        ROS_INFO("run %i task",i);
        state = 0;
        cpoint = 0;
        watch_flag = false;
        is_finish = false;
        if(ctask->nType == (int)TASK_TYPE::TASK_TYPE_TRANSFER){
            // "CameraPose" : "120000:5/21.4207859039,6.13516521454,-131.956115723/10010012/2/4869",
            std::vector<std::string> lists = split_str(ctask->cameraPose[cpoint], "/");
			std::vector<std::string> task_camera = split_str(lists[0], ":");
			int point_type = stoi(task_camera[1]);
            int watch_point = ctask->cameraPose.size();
            while (!is_finish)
            {
                if(cpoint<watch_point){
                    if(state==0){
                        /* 如果是着色点 */
                        if(point_type==5){}
                        /* 按角度旋转 */
                        if(point_type==7){}
                        sleep(1);
					    transfer(ctask->LocX,ctask->LocY,ctask->LocZ, lists, task_camera, ctask->transfer_id, 0);
                        last = ros::Time::now();
                        state++;
                    }else{
                        if (clear_task_flag) break;
                        if(watch_flag){
                            state = 0;
                            cpoint ++;
                            watch_flag = false;
                            continue;
                        }
                        ros::Time now = ros::Time::now();
                        if(now-last>ros::Duration(120)){
                            ROS_INFO("time out:wait for watch");
                            watch_flag = true;
                        }
                    }
                }else{
                    is_finish = true;
                }
            }
        }
    }
    if(task_running && !task_pause){
        pub_task_status(task_id,0);
        //reset flag
        reset();
        //reset cloudplatform
        fixed_msg::platform_transfer data;
        data.flag = 1;
        transfer_pub.publish(data);
        //finish ok
        if(road_tasks.size()>0)
            road_tasks.clear();
    }
    if (clear_task_flag) //清除当前任务
    {
        ROS_INFO("clear task");
        road_tasks.clear();
        reset();
    }
}