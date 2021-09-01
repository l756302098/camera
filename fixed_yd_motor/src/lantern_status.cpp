/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-06 13:37:38
 * @LastEditors: li
 * @LastEditTime: 2021-05-08 14:52:25
 */
#include "fixed_yd_motor/lantern_status.hpp"

lantern_status::lantern_status(const ros::NodeHandle &nh):nh_(nh){
    nh_.param<std::string>("device_ip", device_ip, "192.168.1.4");
    nh_.param<int>("device_port", device_port, 1001);
    std::cout << "ip:" << device_ip << " port:" << device_port << std::endl;
    nh_.param<std::string>("ptz_topic", ptz_topic, "/fixed/platform/position");
    ptz_pub_ = nh_.advertise<nav_msgs::Odometry>(ptz_topic, 1);
    tcp_ptr = std::make_shared<EpollTcpClient>(device_ip, device_port);
    if (!tcp_ptr) {
        std::cout << "tcp_client create faield!" << std::endl;
        ros::shutdown();
    }
    auto recv_call = [&](const PacketPtr& data) -> void {
        std::vector<unsigned char> rv;
        char *p=(char*)data->msg.c_str();
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
    std::cout << "lantern_status network init finish" << std::endl;
}

lantern_status::~lantern_status(){
    if(tcp_ptr)
        tcp_ptr->Stop();
}

void lantern_status::update(){
    motor_temp_id++;
    unsigned char query_id = motor_temp_id % 2 == 0 ? 0x01 : 0x02;
    motor_status(query_id);
    nav_msgs::Odometry pose;
	pose.header.stamp = ros::Time::now();
	pose.pose.pose.position.x = g_now_xyposition;
	pose.pose.pose.position.z = g_now_zposition;
    pose.pose.pose.position.y = g_now_zoom;
	ptz_pub_.publish(pose);
}

void lantern_status::tick(const ros::TimerEvent &event){
    vector<unsigned char> cmd = queue_pop();
    if(cmd.empty() || cmd.size()<20) return;
    if(cmd[3]!=0x0B) return;
    if(cmd[2]==0x01){
        //printf("%x号电机 ",cmd[2]);
        uint16_t single_value = cmd[5] + (cmd[6] << 8);
        uint32_t multi_value = cmd[7] + (cmd[8] << 8) + (cmd[9] << 16)+ (cmd[10] << 24);
        g_now_zposition = single_value * 360 / MOTOR_ROTATE * 100 ;
        // printf(" 运行状态:%x \n",cmd[17]);
        //printf(" single_value=%i single_angle=%i multi_value=%i \n",single_value,g_now_xyposition,multi_value);
    }else{
        uint16_t single_value = cmd[5] + (cmd[6] << 8);
        uint32_t multi_value = cmd[7] + (cmd[8] << 8) + (cmd[9] << 16)+ (cmd[10] << 24);
        g_now_xyposition = single_value * 360 / MOTOR_ROTATE * 100;
        // printf(" 运行状态:%x \n",cmd[17]);
        //printf(" single_value=%i single_angle=%i multi_value=%i \n",single_value,g_now_zposition,multi_value);
    }
}

void lantern_status::motor_relat_angle(char motor_id,int angle){
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

void lantern_status::motor_absolute_angle(char motor_id,int angle){
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
    send_messages(cmd);
}
//电机按照最短的距离回到设定的原点
void lantern_status::motor_back(char motor_id){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x51);
    cmd.push_back(0x00);
    crc_check(cmd);
    send_messages(cmd);
}
//关闭电机,电机进入关闭模式
void lantern_status::motor_close(char motor_id){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x50);
    cmd.push_back(0x00);
    crc_check(cmd);
    send_messages(cmd);
}
//清除系统当前故障
void lantern_status::motor_clear_mal(char motor_id){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x41);
    cmd.push_back(0x00);
    crc_check(cmd);
    send_messages(cmd);
}
//设置电机当前位置为原点
void lantern_status::motor_set_ori(char motor_id){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x21);
    cmd.push_back(0x00);
    crc_check(cmd);
    send_messages(cmd);
}

void lantern_status::crc_check(std::vector<unsigned char> &data){
    unsigned short value = N_CRC16(&(data[0]),data.size());
    char byte1 = (value >> 8) & 0x00FF;
	char byte2 = value & 0x00FF;
    // printf("byte1 %x \n",byte1);
    // printf("byte2 %x \n",byte2);
    data.push_back(byte1);
    data.push_back(byte2);
}

void lantern_status::motor_status(char motor_id){
    std::vector<unsigned char> cmd;
    cmd.push_back(0x3E);
    cmd.push_back(0x00);
    cmd.push_back(motor_id);
    cmd.push_back(0x0B);
    cmd.push_back(0x00);
    crc_check(cmd);
    // for(int i=0;i<cmd.size();i++){
    //     printf("%x ",cmd[i]);
    // }
    // printf("\n");
    send_messages(cmd);
    //std::cout << "end tcp send bytes" << std::endl;
}