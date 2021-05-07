/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-01 13:11:04
 * @LastEditors: li
 * @LastEditTime: 2021-05-07 17:30:52
 */
#ifndef __YD_PT_CONTROL__
#define __YD_PT_CONTROL__

#include "ros/ros.h"
#include <iostream>
#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <mutex>
#include <queue>
#include <std_msgs/Int32.h>
#include <std_msgs/Float32.h>
#include <nav_msgs/Odometry.h>
#include <diagnostic_msgs/DiagnosticArray.h>
#include "sensor_msgs/Image.h"
#include "fixed_msg/cp_control.h"
#include "yidamsg/transfer.h"
#include <yidamsg/InspectedResult.h>
#include <yidamsg/Detect_Result.h>
#include <geometry_msgs/PoseStamped.h>
#include "fixed_yd_onvif/common.hpp"
#include "fixed_yd_onvif/asio_client.hpp"
#include <thread>

using namespace std;
#define MOTOR_ROTATE 16384

class pt_control
{
private:
    ros::NodeHandle nh_;
    ros::ServiceServer ptz_server;
	ros::Subscriber detect_sub;
    ros::Publisher isreach_pub_,ptz_pub_,zoom_pub_;
    geometry_msgs::PoseStamped c_pos,t_pos;

    unsigned int g_get_info_flag = 1;
    unsigned int g_now_xyposition = 0;
    unsigned int g_now_zposition = 0;
    unsigned int g_now_zoom = 0;
    unsigned int g_action = 0;
    unsigned int g_control_type = 0;
    unsigned int g_xy_goal = 0;
    unsigned int g_z_goal = 0;
    unsigned int g_reach_flag = 0;
    unsigned int g_xy_reach_flag = 0;
    unsigned int g_z_reach_flag = 0;
    float g_temperature_c = 0.0;
    int pan_max,pan_min;
    int tilt_max,tilt_min;

    std::thread *sock_thread;
    std::shared_ptr<client> tcp_ptr;
    
public:
    std::string device_ip;
    int device_port;
    std::string device_id,ptz_topic,ptz_server_name;
    std::deque<string> _cmd_control_queue;
    std::mutex write_mtx;
    
public:
    pt_control(const ros::NodeHandle &nh = ros::NodeHandle("~"));
    ~pt_control();
    bool handle_cloudplatform(fixed_msg::cp_control::Request &req, fixed_msg::cp_control::Response &res);
    void update();
    void write_hk();
    void read_hk();
    bool set_action(int id, int type, int value, int xy_value, int z_value, int zoom_value);
    void crc_check(std::vector<unsigned char> &data);
    void motor_ctr(char motor_id,int angle);
    //电机按照最短的距离回到设定的原点
    void motor_back(char motor_id);
    //关闭电机,电机进入关闭模式
    void motor_close(char motor_id);
    //清除系统当前故障
    void motor_clear_mal(char motor_id);
    //设置电机当前位置为原点
    void motor_set_ori(char motor_id);
};

#endif