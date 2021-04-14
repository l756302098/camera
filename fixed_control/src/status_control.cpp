/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-22 13:05:41
 * @LastEditors: li
 * @LastEditTime: 2021-04-13 16:08:39
 */
#include "fixed_control/status_control.hpp"

status_control::status_control(const ros::NodeHandle &nh):nh_(nh),clear_task_flag(false),watch_flag(false)
{
    //get param
    nh_.param<int>("robot_id", robot_id, 1);
    std::cout << "robot_id:" << robot_id << std::endl;
    std::cout << "nh_.advertise" << std::endl;
    transfer_pub = nh_.advertise<yidamsg::transfer>("/transfer_pub", 1);
    control_mode_pub = nh_.advertise<yidamsg::ControlMode>("/control_mode", 1);
    task_status_pub = nh_.advertise<yidamsg::TaskExecuteStatus>("/task_execute_status", 1);
}

status_control::~status_control()
{
    road_tasks.clear();
}

void status_control::meter_cb(const std_msgs::String msg)
{
	ROS_INFO("meter_cb ...");
    watch_flag = true;
}

bool status_control::task_srv(yidamsg::TaskList::Request &req, yidamsg::TaskList::Response &res){
	if (!task_running) //所有任务执行完毕状态
	{
		std::string list = req.plan;
		std::cout << req.plan << std::endl;
        //std::ifstream in(req.plan,ios::in|ios::binary);
        load_from_stream(list);
		res.status = true;
	}
	else
	{
		res.status = false;
	}
	return true;
}

void status_control::tick(const ros::TimerEvent &event){
    //ROS_INFO("tick ...");
    update();
}

void status_control::taskStatusPub(int task_id, int task_status)
{
	yidamsg::TaskExecuteStatus task_msg;
	task_msg.task_history_id = task_id;
	task_msg.task_status = task_status;
	task_status_pub.publish(task_msg);
}

    void status_control::transfer(vector<string> lists, vector<string> task_camera, string trans_id, char flg)
    {
	    string s_time;
	    yidamsg::transfer data;
	    double secs = ros::Time::now().toSec();
	    time_t t1 = int(secs);
	    char now[64];
	    struct tm *ttime;
	    ttime = localtime(&t1);
	    strftime(now, 64, "%Y-%m-%d %H:%M:%S", ttime);
	    s_time = now;
	    data.flag = flg;
	    data.data = s_time + "/";
	    data.data += trans_id;
	    data.data += "/";
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

void status_control::update(){
    //ROS_INFO("update ...");
    if (clear_task_flag) //清除当前任务
    {
        ROS_INFO("clear task");
        road_tasks.clear();
        clear_task_flag = false;
    }
    yidamsg::ControlMode msg;
    msg.robot_id = robot_id;
	msg.mode = ctr_mode;
	control_mode_pub.publish(msg);
    int state = 0;
    int cpoint = 0;
    bool is_finish = false;
    ros::Time last;
    //ROS_INFO("task size %i",road_tasks.size());
    for (size_t i = 0; i < road_tasks.size(); i++){
        if (clear_task_flag) break;
        task_running = true;
        ROAD_PLAN ctask = road_tasks[i];
        ROS_INFO("run %i task",i);
        state = 0;
        cpoint = 0;
        watch_flag = false;
        is_finish = false;
        if(ctask.nType == (int)TASK_TYPE::TASK_TYPE_TRANSFER){
            // "CameraPose" : "120000:5/21.4207859039,6.13516521454,-131.956115723/10010012/2/4869",
            std::vector<std::string> lists = split_str(ctask.cameraPose[cpoint], "/");
			std::vector<std::string> task_camera = split_str(lists[0], ":");
			int point_type = stoi(task_camera[1]);
            int watch_point = ctask.cameraPose.size();
            while (!is_finish)
            {
                if(cpoint<watch_point){
                    if(state==0){
                        /* 如果是着色点 */
                        if(point_type==5){}
                        sleep(1);
					    transfer(lists, task_camera, ctask.transfer_id, 0);
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
                        if(now-last>ros::Duration(30)){
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
    if(task_running){
        taskStatusPub(task_id,0);
        task_running = false;
        //finish ok
        if(road_tasks.size()>0)
            road_tasks.clear();
    }
}