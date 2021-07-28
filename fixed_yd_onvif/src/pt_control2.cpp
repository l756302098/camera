/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-06 13:37:38
 * @LastEditors: li
 * @LastEditTime: 2021-05-08 14:52:25
 */
#include "fixed_yd_onvif/pt_control2.hpp"

pt_control2::pt_control2(const ros::NodeHandle &nh):nh_(nh),g_xy_goal(-1),g_z_goal(-1){
    nh_.param<std::string>("device_ip", device_ip, "192.168.1.4");
    nh_.param<int>("device_port", device_port, 1001);
    std::cout << "ip:" << device_ip << " port:" << device_port << std::endl;
    nh_.param<std::string>("ptz_topic", ptz_topic, "/fixed/platform/position");
    nh_.param<std::string>("ptz_server_name", ptz_server_name, "/fixed/platform/cmd");
    motor_sub = nh_.subscribe("/fixed/motor/cmd", 1, &pt_control2::motor_callback, this);
    isreach_pub_ = nh_.advertise<std_msgs::Int32>("/fixed/platform/isreach", 1);
    zoom_pub_ = nh_.advertise<std_msgs::Float32>("/fixed/visible/zoom", 1);
    ptz_pub_ = nh_.advertise<nav_msgs::Odometry>(ptz_topic, 1);
    ptz_server = nh_.advertiseService(ptz_server_name, &pt_control2::handle_cloudplatform, this);
    std::unique_ptr<client> ptr (new client(device_ip,device_port));
    tcp_ptr = std::move(ptr);
    sock_thread = new std::thread(std::bind(&client::run,tcp_ptr.get()));
    std::cout << "pt_control2 network init finish" << std::endl;
}

pt_control2::~pt_control2(){
    if(sock_thread)
        delete sock_thread;
}

void pt_control2::update(){
    motor_temp_id++;
    unsigned char query_id = motor_temp_id % 2 == 0 ? 0x47 : 0x49;
    if(!tcp_ptr->is_open()){
        std::cout << "network error reconnect ..." << std::endl;
        sleep(1);
        tcp_ptr->start();
    }
    motor_status(query_id);
    //motor_status(0x46);
    nav_msgs::Odometry pose;
	pose.header.stamp = ros::Time::now();
	pose.pose.pose.position.x = g_now_xyposition;
	pose.pose.pose.position.z = g_now_zposition;
    pose.pose.pose.position.y = g_now_zoom;
	ptz_pub_.publish(pose);
    if((g_xy_reach_flag == 1) && (g_z_reach_flag == 1))
    {
        g_xy_reach_flag = 0;
        g_z_reach_flag = 0;
        g_xy_goal = -1;
        g_z_goal = -1;
        std_msgs::Int32 res_msg;
		res_msg.data = 1;
		isreach_pub_.publish(res_msg);
        std::cout << "isreach publish" << std::endl;
    }
}

bool pt_control2::handle_cloudplatform(fixed_msg::cp_control::Request &req, fixed_msg::cp_control::Response &res)
{
    this->write_mtx.lock();
    std::stringstream ss;
	ss.str("");
	ss << req.id << "/" << req.action << "/" << req.type << "/" << req.value;
    if(req.type == 3){
        ss << "/" << req.allvalue[0] << "/" << req.allvalue[1];
    }
    if(req.type == 4){
        std::cout << "1111111111111" << std::endl;
        ss << "/" << req.allvalue[0] << "/" << req.allvalue[1] << "/" << req.allvalue[2];
    }
    std::string s_cmd = ss.str();
    std::cout << "handle message:" <<  s_cmd << std::endl;
    _cmd_control_queue.push_back(s_cmd);
    this->write_mtx.unlock();
	return true;
}

void pt_control2::write_hk(){
    while(true)
    {
        sleep(0.1);
        std::string work_cmd = "";
        //std::cout << "_cmd_control_queue size:" << this->_cmd_control_queue.size() << std::endl;
        if(this->_cmd_control_queue.size() > 0)
        {
            this->write_mtx.lock();
            std::cout << "before execute, _cmd_control_queue size is : " << this->_cmd_control_queue.size() << std::endl;
            work_cmd = this->_cmd_control_queue[0];
            this->_cmd_control_queue.pop_front();
            std::cout << "after execute, _cmd_control_queue size is : " << this->_cmd_control_queue.size() << std::endl;
            this->write_mtx.unlock();

            if(work_cmd != "")
            {
                vector<std::string> cmd_value_strv;
                SplitString(work_cmd, cmd_value_strv, "/");

                int id = atoi(cmd_value_strv[0].c_str());
                int action = atoi(cmd_value_strv[1].c_str());
                int type = atoi(cmd_value_strv[2].c_str());
                int value = atoi(cmd_value_strv[3].c_str());
                std::cout << "id: " << id << ", action: " << action << ", type: " << type << ", value: " << value << std::endl;
                g_action = action;
                if(type == 0 || type == 1)
                {
                    g_control_type = type;
                    if(g_control_type == 0){
                        g_xy_goal = value;
                    }
                    else if(g_control_type == 1)
                        g_z_goal = value;
                }
                else if(type == 3){
                    g_control_type = type;
                    int xy_value = atoi(cmd_value_strv[4].c_str());
                    int z_value = atoi(cmd_value_strv[5].c_str());
                    g_xy_goal = xy_value;
                    g_z_goal = z_value;
                    std::cout << "now  xy_value:" << g_now_xyposition << " z_value:" << g_now_zposition << std::endl;
                    std::cout << "goal xy_value:" << g_xy_goal << " z_value:" << g_z_goal << std::endl;
                    //ready for wait
                    ready = true;
                }
            }
        }
        if(work_cmd != "")
        {
            unsigned int result_value = 0;
            vector<std::string> cmd_value_strv;
            SplitString(work_cmd, cmd_value_strv, "/");
            //action:0 获取  action:1 设置
            //type:0 水平  type:1 垂直  type:2 变倍
            int id = atoi(cmd_value_strv[0].c_str());
            int action = atoi(cmd_value_strv[1].c_str());
            int type = atoi(cmd_value_strv[2].c_str());
            int value,xy_value,z_value;
            value = xy_value = z_value = 0;
            if(type==0){
                value = atoi(cmd_value_strv[3].c_str());
                g_xy_reach_flag = 0;
                g_z_reach_flag = 1;
            }else if(type==1){
                value = atoi(cmd_value_strv[3].c_str());
                g_xy_reach_flag = 1;
                g_z_reach_flag = 0;
            }else if(type==2){
                value = atoi(cmd_value_strv[3].c_str());
            }
            else if(type==3){
                xy_value = atoi(cmd_value_strv[4].c_str());
                z_value = atoi(cmd_value_strv[5].c_str());
                g_xy_reach_flag = 0;
                g_z_reach_flag = 0;
            }
            if(action == 1){
                this->set_action(id, type, value, xy_value, z_value, 0);
            }
        }
    }
}

void pt_control2::tick(const ros::TimerEvent &event){
    vector<unsigned char> cmd = tcp_ptr->queue_pop();
    if(cmd.empty()) return;
    if(cmd.size()<7) return;
    if(cmd[2]==0x48){
        unsigned char data4 = cmd[4];
        unsigned char data5 = cmd[5];
        uint hdata = (data4 << 8) + data5;
        g_now_xyposition = hdata;
        //printf("%x %x h:%i \n",data4,data5,g_now_xyposition);
    }else if(cmd[2]==0x50){
        unsigned char data4 = cmd[4];
        unsigned char data5 = cmd[5];
        std::vector<unsigned char> temp;
        temp.push_back(data5);
        temp.push_back(data4);
        int16_t* tempi = (int16_t*)&temp[0];
        g_now_zposition = *tempi;
        //uint vdata = (data4 << 8) + data5;
        //g_now_zposition = vdata;
        //printf("%x %x v:%i \n",data4,data5,g_now_zposition);
    }else if(cmd[2]==0x46){
        unsigned char data4 = cmd[4];
        unsigned char data5 = cmd[5];
        //printf("%x %x \n",data4,data5);
        uint data = (data4 << 8) + data5;
        // if(data==0){
        //     printf("current motor:move \n");
        // }else{
        //     printf("current motor:stop \n");
        // }
        //printf("%x %x v:%i \n",data4,data5,vdata);
    }
    // std::cout << "g_xy_reach_flag:" << g_xy_reach_flag << " g_z_reach_flag:" << g_z_reach_flag << std::endl;
    // std::cout << "g_action:" << g_action << " g_control_type:" << g_control_type << std::endl;
    // std::cout << "g_xy_goal:" << g_xy_goal << " g_z_goal:" << g_z_goal << std::endl;
    if(g_action == 1 &&   g_control_type != 2 )
    {
        if(g_xy_goal != -1){
            xy_diff_val = g_xy_goal - g_now_xyposition;
            xy_diff_val = abs(xy_diff_val) % 36000;
            std::cout << "g_xy_goal: " << g_xy_goal << std::endl;
            std::cout << "g_now_xyposition: " << g_now_xyposition << std::endl;
            std::cout << "xy_val: " << xy_diff_val << std::endl;
            if(xy_diff_val > 35800)
                xy_diff_val = 0;
            if(abs(xy_diff_val)<200){
                g_xy_goal = -1;
                g_xy_reach_flag = 1;
                if(ready && g_control_type==3){
                    std::unique_lock <std::mutex> lck(mtx);
                    ready = false;
                    cv.notify_one();
                    std::cout << "notify_one" << std::endl;
                }
            }
        }
        if (g_z_goal != -1){
            z_diff_val = g_z_goal - g_now_zposition;
            z_diff_val = abs(z_diff_val) % 36000;
            std::cout << "g_z_goal: " << g_z_goal << std::endl;
            std::cout << "g_now_zposition: " << g_now_zposition << std::endl;
            std::cout << "z_val: " << z_diff_val << std::endl;
            if(z_diff_val > 35800)
                z_diff_val = 0;
            if(abs(z_diff_val)<200){
                g_z_reach_flag = 1;
                g_z_goal = -1;
            }
        }
    }
}

bool pt_control2::set_action(int id, int type, int value, int xy_value, int z_value, int zoom_value){
    //wPanPos   1号电机
    //wTiltPos  2号电机
    if(type == 0){
        motor_absolute_angle(0x41,value);
    }else if(type == 1){
        motor_absolute_angle(0x42,value);
    }else if(type == 2){
        //set zoom
        std_msgs::Float32 data;
        data.data = value / 100;
        zoom_pub_.publish(data);
    }
    else if(type == 3){
        motor_absolute_angle(0x41,xy_value);
        std::unique_lock <std::mutex> lck(mtx);
        std::cout << "wait for xydegree." << std::endl;
        if(cv.wait_for(lck,std::chrono::seconds(10)) == std::cv_status::timeout){
            std::cout << "wait timeout." << std::endl;
        }
        sleep(1);
        std::cout << "write thread wake." << std::endl;
        ready = false;
        motor_absolute_angle(0x42,z_value);
    }
}

void pt_control2::motor_callback(const std_msgs::String::ConstPtr& msg){
    std::string cmd =  msg->data.c_str();
    std::cout << "cmd:" << cmd;
    motor_set_ori();
}

void pt_control2::motor_relat_angle(char cmd1,int angle){
    /*
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x56);
    cmd.push_back(0x02);
    int rotate =  angle;
    char byte1 =  rotate & 0x00FF;
    char byte2 =  (rotate >> 8) & 0x00FF;
    std::cout << "rotate:" << rotate << " byte1:" << byte1<< " byte2:" << byte2 << std::endl;
    cmd.push_back(byte1);
    cmd.push_back(byte2);
    crc_check(cmd);
    tcp_ptr->send_bytes(cmd);
    */
}

void pt_control2::motor_absolute_angle(char cmd1,int angle){
    std::cout << "motor_absolute_angle." << std::endl;
    std::vector<unsigned char> cmd;
    cmd.push_back(0xFF);
    cmd.push_back(0x01);
    cmd.push_back(cmd1);
    cmd.push_back(0x00);
    unsigned int rotate = angle;
    unsigned char byte1 =  (rotate >> 8) & 0x00FF;
    unsigned char byte2 =  rotate & 0x00FF;
    cmd.push_back(byte1);
    cmd.push_back(byte2);
    crc_check(cmd);
    for(int i=0;i<cmd.size();i++){
        printf("%x ",cmd[i]);
    }
    printf("\n");
    tcp_ptr->send_bytes(cmd);
}
//设置电机当前位置为原点
void pt_control2::motor_set_ori(){
    std::vector<unsigned char> cmd;
    cmd.push_back(0xFF);
    cmd.push_back(0x01);
    cmd.push_back(0x00);
    cmd.push_back(0x9B);
    cmd.push_back(0x00);
    cmd.push_back(0x00);
    crc_check(cmd);
    tcp_ptr->send_bytes(cmd);
}

void pt_control2::crc_check(std::vector<unsigned char> &data){
    uint sum = 0;
    for(int i=1;i<data.size();i++){
        sum += data[i];
    }
    unsigned char crc = sum % 0x100;
    data.push_back(crc);
}

void pt_control2::motor_status(char cmd1){
    std::vector<unsigned char> cmd;
    cmd.push_back(0xFF);
    cmd.push_back(0x01);
    cmd.push_back(cmd1);
    cmd.push_back(0x00);
    cmd.push_back(0x00);
    cmd.push_back(0x00);
    crc_check(cmd);
    //printf("cmd: %x \n",cmd1);
    tcp_ptr->send_bytes(cmd);
}