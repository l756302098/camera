/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-06 13:37:38
 * @LastEditors: li
 * @LastEditTime: 2021-05-08 14:52:25
 */
#include "fixed_yd_motor/lantern_control.hpp"

lantern_control::lantern_control(const ros::NodeHandle &nh):nh_(nh),g_xy_goal(-1),g_z_goal(-1){
    nh_.param<std::string>("device_ip", device_ip, "192.168.1.4");
    nh_.param<int>("device_port", device_port, 1001);
    std::cout << "ip:" << device_ip << " port:" << device_port << std::endl;
    nh_.param<std::string>("ptz_topic", ptz_topic, "/fixed/platform/position");
    nh_.param<std::string>("ptz_server_name", ptz_server_name, "/fixed/platform/cmd");
    motor_sub = nh_.subscribe("/fixed/motor/cmd", 1, &lantern_control::motor_callback, this);
    isreach_pub_ = nh_.advertise<std_msgs::Int32>("/fixed/platform/isreach", 1);
    ptz_sub = nh_.subscribe(ptz_topic, 1, &lantern_control::pose_cb, this);
    ptz_server = nh_.advertiseService(ptz_server_name, &lantern_control::handle_cloudplatform, this);
    tcp_ptr = std::make_shared<EpollTcpClient>(device_ip, device_port);
    if (!tcp_ptr) {
        std::cout << "tcp_client create faield!" << std::endl;
        ros::shutdown();
    }
    auto recv_call = [&](const PacketPtr& data) -> void {
        // just print recv data to stdout
        //std::cout << "recv: " << data->msg.size() << std::endl;
        std::vector<unsigned char> rv;
        char *p=(char*)data->msg.c_str();
        // for(int i = 0; i < sizeof(p); i++){
        //     std::cout << i << ": " << p[i] << std::endl;
        // }
        rv.insert(rv.end(),p,p+data->msg.size());
        que_mtx.lock();
        receive_msg.push_back(rv);
        que_mtx.unlock();
        return;
    };
    tcp_ptr->RegisterOnRecvCallback(recv_call);
    if (!tcp_ptr->Start()) {
        std::cout << "tcp_client start failed!" << std::endl;
        ros::shutdown();
    }
    std::cout << "############tcp_client client started!################" << std::endl;
    std::cout << "lantern_control network init finish" << std::endl;
}

lantern_control::~lantern_control(){
    if(tcp_ptr)
        tcp_ptr->Stop();
}

void lantern_control::update(){
    if(g_action == 1 && g_control_type != 2 )
    {
        if(g_xy_goal != -1){
            xy_diff_val = g_xy_goal - g_now_xyposition;
            std::cout << "g_xy_goal: " << g_xy_goal << std::endl;
            std::cout << "g_now_xyposition: " << g_now_xyposition << std::endl;
            std::cout << "xy_val: " << xy_diff_val << std::endl;
            if(abs(xy_diff_val) > 35700)
                xy_diff_val = 0;
            if(abs(xy_diff_val)<300){
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
            std::cout << "g_z_goal: " << g_z_goal << std::endl;
            std::cout << "g_now_zposition: " << g_now_zposition << std::endl;
            std::cout << "z_val: " << z_diff_val << std::endl;
            if(abs(z_diff_val)>35700)
                z_diff_val = 0;
            if(abs(z_diff_val)<300){
                g_z_reach_flag = 1;
                g_z_goal = -1;
            }
        }
    }
    //motor_status(0x02);
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

bool lantern_control::handle_cloudplatform(fixed_msg::cp_control::Request &req, fixed_msg::cp_control::Response &res)
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

void lantern_control::write_hk(){
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
            //action:0 ??????  action:1 ??????
            //type:0 ??????  type:1 ??????  type:2 ??????
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
            //???????????????????????????????????????????????????????????????485??????pelco-d?????????????????????
            if(action == 1){
                this->set_action(id, type, value, xy_value, z_value, 0);
            }
        }
    }
}

bool lantern_control::set_action(int id, int type, int value, int xy_value, int z_value, int zoom_value){
    //wPanPos   1?????????
    //wTiltPos  2?????????
    if(type == 0){
        motor_id = 0x01;
        motor_absolute_angle(0x02,value / 100);
    }else if(type == 1){
        motor_id = 0x02;
        motor_absolute_angle(0x01,value / 100);
    }
    else if(type == 3){
        //????????????
        motor_id = 0x01;
        motor_absolute_angle(0x01,xy_value / 100);
        std::unique_lock <std::mutex> lck(mtx);
        std::cout << "wait for xydegree." << std::endl;
        if(cv.wait_for(lck,std::chrono::seconds(10)) == std::cv_status::timeout){
            std::cout << "wait timeout." << std::endl;
        }
        std::cout << "write thread wake." << std::endl;
        ready = false;
        motor_id = 0x02;
        motor_absolute_angle(0x02,z_value / 100);
    }
}

void lantern_control::motor_callback(const std_msgs::String::ConstPtr& msg){
    std::string cmd =  msg->data.c_str();
    std::cout << "cmd:" << cmd;
    if(cmd.compare(0,CMD_GOBACK.length(),CMD_GOBACK)==0){
        if(cmd==CMD_GOBACK+":1"){
            motor_back(0x01);
            std::cout << " goback 0x01"<< std::endl;
        }else{
            motor_back(0x02);
            std::cout << " goback 0x02"<< std::endl;
        }
    }else if(cmd.compare(0,CMD_SETHOME.length(),CMD_SETHOME)==0)
    {
        if(cmd==CMD_SETHOME+":1"){
            motor_set_ori(0x01);
            std::cout << " sethome 0x01"<< std::endl;
        }else{
            motor_set_ori(0x02);
            std::cout << " sethome 0x02"<< std::endl;
        }
    }else if(cmd.compare(0,CMD_CLEAR.length(),CMD_CLEAR)==0)
    {
        if(cmd==CMD_CLEAR+":1"){
            motor_clear_mal(0x01);
        }else{
            motor_clear_mal(0x02);
        }
    }else if(cmd.compare(0,CMD_CLOSE.length(),CMD_CLOSE)==0)
    {
        if(cmd==CMD_CLOSE+":1"){
            motor_close(0x01);
        }else{
            motor_close(0x02);
        }
    }
}

void lantern_control::motor_relat_angle(char motor_id,int angle){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x56);
    cmd.push_back(0x02);
    int rotate = MOTOR_ROTATE / 360 * angle;
    char byte1 =  rotate & 0x00FF;
    char byte2 =  (rotate >> 8) & 0x00FF;
    std::cout << "rotate:" << rotate << " byte1:" << byte1<< " byte2:" << byte2 << std::endl;
    cmd.push_back(byte1);
    cmd.push_back(byte2);
    crc_check(cmd);
    send_messages(cmd);
}

void lantern_control::motor_absolute_angle(char motor_id,int angle){
    printf("motor_absolute_angle");
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x55);
    cmd.push_back(0x04);
    unsigned int rotate = MOTOR_ROTATE / 360 * angle;
    unsigned char byte1 =  rotate & 0x00FF;
    unsigned char byte2 =  (rotate >> 8) & 0x00FF;
    unsigned char byte3 =  0;
    unsigned char byte4 =  0;
    std::cout << "angle:" << angle << " rotate:" << rotate << std::endl; 
    //printf("10--> %i %i \n",byte1,byte2); 
    //printf("16--> %x %x \n",byte1,byte2); 
    cmd.push_back(byte1);
    cmd.push_back(byte2);
    cmd.push_back(byte3);
    cmd.push_back(byte4);
    crc_check(cmd);
    for (size_t i = 0; i < cmd.size(); i++)
    {
        printf(" %x ",cmd[i]);
    }
    printf("\n");
    send_messages(cmd);
}
//????????????????????????????????????????????????
void lantern_control::motor_back(char motor_id){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x51);
    cmd.push_back(0x00);
    crc_check(cmd);
    send_messages(cmd);
}
//????????????,????????????????????????
void lantern_control::motor_close(char motor_id){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x50);
    cmd.push_back(0x00);
    crc_check(cmd);
    send_messages(cmd);
}
//????????????????????????
void lantern_control::motor_clear_mal(char motor_id){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x41);
    cmd.push_back(0x00);
    crc_check(cmd);
    send_messages(cmd);
}
//?????????????????????????????????
void lantern_control::motor_set_ori(char motor_id){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x21);
    cmd.push_back(0x00);
    crc_check(cmd);
    send_messages(cmd);
}

void lantern_control::crc_check(std::vector<unsigned char> &data){
    unsigned short value = N_CRC16(&(data[0]),data.size());
    char byte1 = (value >> 8) & 0x00FF;
	char byte2 = value & 0x00FF;
    // printf("byte1 %x \n",byte1);
    // printf("byte2 %x \n",byte2);
    data.push_back(byte1);
    data.push_back(byte2);
}

void lantern_control::motor_status(char motor_id){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x0B);
    cmd.push_back(0x00);
    crc_check(cmd);
    //printf("3E 00 %x 0B 00 %x %x \n",motor_id,cmd[5],cmd[6]);
    //std::cout << "start tcp send bytes" << std::endl;
    send_messages(cmd);
    //std::cout << "end tcp send bytes" << std::endl;
}