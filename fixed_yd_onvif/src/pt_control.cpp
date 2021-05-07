/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-06 13:37:38
 * @LastEditors: li
 * @LastEditTime: 2021-05-07 17:29:51
 */
#include "fixed_yd_onvif/pt_control.hpp"

pt_control::pt_control(const ros::NodeHandle &nh):nh_(nh){
    nh_.param<std::string>("device_ip", device_ip, "192.168.1.4");
    nh_.param<int>("device_port", device_port, 1001);
    std::cout << "ip:" << device_ip << " port:" << device_port << std::endl;
    nh_.param<std::string>("ptz_topic", ptz_topic, "/fixed/yuntai/position");
    nh_.param<std::string>("ptz_server_name", ptz_server_name, "/fixed/internal/platform_cmd");
    isreach_pub_ = nh_.advertise<std_msgs::Int32>("/fixed/platform_isreach", 1);
    zoom_pub_ = nh_.advertise<std_msgs::Float32>("/fixed/visible/zoom", 1);
    ptz_pub_ = nh_.advertise<nav_msgs::Odometry>(ptz_topic, 1);
    ptz_server = nh_.advertiseService(ptz_server_name, &pt_control::handle_cloudplatform, this);
    std::unique_ptr<client> ptr (new client(device_ip,device_port));
    tcp_ptr = std::move(ptr);
    sock_thread = new std::thread(std::bind(&client::run,tcp_ptr.get()));
    std::cout << "pt_control init finish" << std::endl;
}

pt_control::~pt_control(){
    if(sock_thread)
        delete sock_thread;
}

void pt_control::update(){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(0x01);
    cmd.push_back(0x0B);
    cmd.push_back(0x00);
    crc_check(cmd);
    //std::cout << "crc size:" << cmd.size() << std::endl;
    tcp_ptr->send_bytes(cmd);
}

bool pt_control::handle_cloudplatform(fixed_msg::cp_control::Request &req, fixed_msg::cp_control::Response &res)
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
    std::cout << s_cmd << std::endl;
    _cmd_control_queue.push_back(s_cmd);
    this->write_mtx.unlock();
	return true;
}

void pt_control::write_hk(){
    while(true)
    {
        sleep(0.1);
        std::string work_cmd = "";
        //std::cout << "this:" << this << std::endl;
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

                int xy_value = 0;
                int z_value = 0;
                int zoom_value = 0;
                if(type == 3)
                {
                    xy_value = atoi(cmd_value_strv[4].c_str());
                    z_value = atoi(cmd_value_strv[5].c_str());
                }
                if(type == 4)
                {
                    std::cout << "in type 4!" << std::endl;
                    xy_value = atoi(cmd_value_strv[4].c_str());
                    z_value = atoi(cmd_value_strv[5].c_str());
                    zoom_value = atoi(cmd_value_strv[6].c_str());
                }

                g_action = action;
                if((type == 0) || (type == 1))
                {
                    g_control_type = type;
                    if(g_control_type == 0)
                        g_xy_goal = value;
                    else if(g_control_type == 1)
                        g_z_goal = value;
                }
                else if((type == 3) || (type == 4))
                {
                    g_control_type = type;
                    g_xy_goal = xy_value;
                    g_z_goal = z_value;
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
            int value = atoi(cmd_value_strv[3].c_str());
            int xy_value = 0;
            int z_value = 0;
            int zoom_value = 0;
            if((action==1) && (type == 3))
            {
                xy_value = atoi(cmd_value_strv[4].c_str());
                z_value = atoi(cmd_value_strv[5].c_str());
            }
            if((action==1) && (type == 4))
            {
                xy_value = atoi(cmd_value_strv[4].c_str());
                z_value = atoi(cmd_value_strv[5].c_str());
                zoom_value = atoi(cmd_value_strv[6].c_str());
            }
            //此处分开设置水平、垂直及变倍值是为兼容通过485遵循pelco-d协议设置的方式
            if(action == 1){
                this->set_action(id, type, value, xy_value, z_value, zoom_value);
            }
            else if(action == 0){
                
            }
            else if(action == 2){
                
            }
            else if(action == 3){
                
            }
            if((action == 0) && (type == 0))
            {
                g_now_xyposition = result_value;
            }
            else if((action == 0) && (type == 1))
            {
                g_now_zposition = result_value;
            }
            else if((action == 0) && (type == 2))
            {
                g_now_zoom = result_value;
            }
        }
    }
}

void pt_control::read_hk(){
    int z_diff_val = 0, xy_diff_val = 0;
    while(true)
    {
        sleep(0.1);
        vector<unsigned char> cmd = tcp_ptr->queue_pop();
        if(cmd.empty() || cmd.size()<5) continue;
        if(cmd[3]!=0x0B) continue;
        for (int i = 0; i < cmd.size(); i++)
        {
            printf(" %i=%x ",i,cmd[i]);
        }
        printf("\n");
        int single_value = cmd[5] + cmd[6] << 8;
        int multi_value = cmd[7] + cmd[8] << 8+ cmd[9] << 16+ cmd[10] << 24;
        float single_angle = single_value * 360 / MOTOR_ROTATE ;
        printf(" single_value=%i single_angle=%f multi_value=%i \n",single_value,single_angle,multi_value);

        if((g_action == 1) &&   ((g_control_type == 0) || (g_control_type == 1) || (g_control_type == 3) || (g_control_type == 4)))
        {
            if((g_xy_goal != 0) || (g_z_goal != 0)){
                xy_diff_val = g_xy_goal - g_now_xyposition;
                // std::cout << "g_xy_goal: " << g_xy_goal << std::endl;
                // std::cout << "g_now_xyposition: " << g_now_xyposition << std::endl;
                // std::cout << "xy_val: " << xy_diff_val << std::endl;
                if((xy_diff_val > 35900) || (xy_diff_val < -35900))
                    xy_diff_val = 0;
                z_diff_val = g_z_goal - g_now_zposition;
                // std::cout << "g_z_goal: " << g_z_goal << std::endl;
                // std::cout << "g_now_zposition: " << g_now_zposition << std::endl;
                // std::cout << "z_val: " << z_diff_val << std::endl;
                if((z_diff_val > 35900) || (z_diff_val < -35900))
                    z_diff_val = 0;
                if(((z_diff_val < 100) && (z_diff_val > -100)) && ((xy_diff_val < 100) && (xy_diff_val > -100)))
                {
                    g_action = -1;
                    g_xy_reach_flag = 1;
                    g_z_reach_flag = 1;
                }
            }
        }
    }
}

bool pt_control::set_action(int id, int type, int value, int xy_value, int z_value, int zoom_value){
    //设置水平、垂直、变倍
    float z_degree = 0.0;
    float xy_degree = 0.0;
    float focus_adjust = 0.0;
    if(type == 0){
        xy_degree = value;
    }
    else if(type == 1){
        z_degree = value;
    }
    else if(type == 2){
        focus_adjust = value;   
    }
    else if(type == 3){
        xy_degree = xy_value;
        z_degree = z_value;
    }
    else if(type == 4){
        xy_degree = xy_value;
        z_degree = z_value;
        focus_adjust = zoom_value;
    }
    int xy_idegree = xy_degree/100;
    int z_idegree = z_degree/100;
    int focus_iadjust = focus_adjust / 100;
    std::cout << "wPanPos:" << xy_idegree << std::endl;
    std::cout << "wTiltPos:" << z_idegree << std::endl;
    std::cout << "wZoomPos:" << focus_iadjust << std::endl;
    //wPanPos   1号电机
    //wTiltPos  2号电机
    if(type == 0){
        motor_ctr(0x01,xy_idegree);
    }else if(type == 1){
        motor_ctr(0x02,z_idegree);
    }else if(type == 2){
        //set zoom
        std_msgs::Float32 data;
        data.data = focus_iadjust;
        zoom_pub_.publish(data);
    }
}

// void pt_control::motor_ctr(char motor_id,int angle){
//     std::vector<unsigned char> cmd;
//     cmd.push_back(0x3E);
//     cmd.push_back(0x00);
//     cmd.push_back(motor_id);
//     cmd.push_back(0x56);
//     cmd.push_back(0x02);
//     int rotate = MOTOR_ROTATE / 360 * angle;
//     char byte1 =  rotate & 0x00FF;
//     char byte2 =  rotate & 0xFF00;
//     cmd.push_back(0xE8);
//     cmd.push_back(0x03);
//     std::cout << "cmd size:" << cmd.size() << std::endl;
//     crc_check(cmd);
//     /*
//     std::cout << "crc size:" << cmd.size() << std::endl;
//     for (int i = 0; i < cmd.size(); i++)
//     {
//         printf("10 %i:%i ",i,cmd[i]);
//         printf("16 %i:%x ",i,cmd[i]);
//     }
//     printf("\n");
//     */
//     tcp_ptr->send_bytes(cmd);
// }

void pt_control::motor_ctr(char motor_id,int angle){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x55);
    cmd.push_back(0x04);
    unsigned int rotate = MOTOR_ROTATE / 360 * angle;
    unsigned char byte1 =  rotate & 0x00FF;
    unsigned char byte2 =  (rotate >> 8) & 0x00FF;
    unsigned char byte3 =  (rotate >> 16) & 0x00FF;
    unsigned char byte4 =  (rotate >> 24) & 0x00FF;
    std::cout << "angle:" << angle << " rotate:" << rotate << std::endl; 
    //printf("10--> %i %i \n",byte1,byte2); 
    //printf("16--> %x %x \n",byte1,byte2); 
    cmd.push_back(byte1);
    cmd.push_back(byte2);
    cmd.push_back(byte3);
    cmd.push_back(byte4);
    crc_check(cmd);
    tcp_ptr->send_bytes(cmd);
}
//电机按照最短的距离回到设定的原点
void pt_control::motor_back(char motor_id){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x52);
    cmd.push_back(0x00);
    crc_check(cmd);
    tcp_ptr->send_bytes(cmd);
}
//关闭电机,电机进入关闭模式
void pt_control::motor_close(char motor_id){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x50);
    cmd.push_back(0x00);
    crc_check(cmd);
    tcp_ptr->send_bytes(cmd);
}
//清除系统当前故障
void pt_control::motor_clear_mal(char motor_id){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x41);
    cmd.push_back(0x00);
    crc_check(cmd);
    tcp_ptr->send_bytes(cmd);
}
//设置电机当前位置为原点
void pt_control::motor_set_ori(char motor_id){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x21);
    cmd.push_back(0x00);
    crc_check(cmd);
    tcp_ptr->send_bytes(cmd);
}

void pt_control::crc_check(std::vector<unsigned char> &data){
    unsigned short value = N_CRC16(&(data[0]),data.size());
    char byte1 = (value >> 8) & 0x00FF;
	char byte2 = value & 0x00FF;
    // printf("byte1 %x \n",byte1);
    // printf("byte2 %x \n",byte2);
    data.push_back(byte1);
    data.push_back(byte2);
}
