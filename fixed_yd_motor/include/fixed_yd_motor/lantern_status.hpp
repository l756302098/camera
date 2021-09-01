/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-01 13:11:04
 * @LastEditors: li
 * @LastEditTime: 2021-05-13 13:39:59
 */
#ifndef __YD_LANTERN_STATUS__
#define __YD_LANTERN_STATUS__

#include "ros/ros.h"
#include <iostream>
#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <mutex>
#include <queue>
#include <std_msgs/Int32.h>
#include <std_msgs/Float32.h>
#include <nav_msgs/Odometry.h>
#include "std_msgs/String.h"
#include <diagnostic_msgs/DiagnosticArray.h>
#include "sensor_msgs/Image.h"
#include "fixed_msg/cp_control.h"
#include "fixed_msg/platform_transfer.h"
#include <fixed_msg/inspected_result.h>
#include <fixed_msg/detect_result.h>
#include <geometry_msgs/PoseStamped.h>
#include "fixed_yd_motor/common.hpp"
#include "fixed_yd_motor/epoll_client.hpp"
#include <thread>
#include <mutex>                // std::mutex, std::unique_lock
#include <condition_variable>    // std::condition_variable

using namespace std;
using namespace transport;
#define MOTOR_ROTATE 16384.0

class lantern_status
{
private:
    unsigned char motor_id = 0x01;
    unsigned char motor_temp_id = 0x01;
    ros::NodeHandle nh_;
	ros::Subscriber detect_sub,motor_sub,ptz_sub;
    ros::Publisher isreach_pub_,ptz_pub_,zoom_pub_;
    geometry_msgs::PoseStamped c_pos,t_pos;

    std::mutex que_mtx;
    std::shared_ptr<EpollTcpClient> tcp_ptr;
    std::deque<std::vector<unsigned char>> receive_msg;
    vector<unsigned char> queue_pop(){
        if(!receive_msg.empty() && que_mtx.try_lock()){
            auto msg = receive_msg.front();
            receive_msg.pop_front();
            que_mtx.unlock();
            return msg;
        }
        return {};
    }
    
public:
    unsigned int g_now_xyposition = 0;
    unsigned int g_now_zposition = 0;
    unsigned int g_now_zoom = 0;
    std::string device_ip;
    int device_port;
    std::string device_id,ptz_topic,ptz_server_name;
    
public:
    lantern_status(const ros::NodeHandle &nh = ros::NodeHandle("~"));
    ~lantern_status();
    bool handle_cloudplatform(fixed_msg::cp_control::Request &req, fixed_msg::cp_control::Response &res);
    void update();
    void tick(const ros::TimerEvent &event);
    void crc_check(std::vector<unsigned char> &data);
    //绝对角度
    void motor_absolute_angle(char motor_id,int angle);
    //相对角度
    void motor_relat_angle(char motor_id,int angle);
    //电机按照最短的距离回到设定的原点
    void motor_back(char motor_id);
    //关闭电机,电机进入关闭模式
    void motor_close(char motor_id);
    //清除系统当前故障
    void motor_clear_mal(char motor_id);
    //设置电机当前位置为原点
    void motor_set_ori(char motor_id);
    //读取电机系统实时数据
    void motor_status(char motor_id);
    void send_messages(std::vector<unsigned char> data){
        if(tcp_ptr!=nullptr){
            std::string message(data.begin(), data.end());
            auto packet = std::make_shared<Packet>(message);
            //std::cout << "send_messages packet size:" << packet->msg.size() << std::endl;
            tcp_ptr->SendData(packet);
        }
    }

};

#endif