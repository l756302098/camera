/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-01 13:11:04
 * @LastEditors: li
 * @LastEditTime: 2021-05-13 13:39:59
 */
#ifndef __YD_PT_CONTROL__
#define __YD_PT_CONTROL__

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
#include "fixed_yd_motor/asio_client.hpp"
#include <thread>
#include <mutex>                // std::mutex, std::unique_lock
#include <condition_variable>    // std::condition_variable
#include <stdlib.h>

using namespace std;

class pt_control2
{
private:
    std::string CMD_GOBACK = "goback";
    std::string CMD_SETHOME = "sethome";
    std::string CMD_CLEAR = "clear";
    std::string CMD_CLOSE = "close";
    unsigned char motor_id = 0x01;
    unsigned char motor_temp_id = 0x01;
    ros::NodeHandle nh_;
    ros::ServiceServer ptz_server;
	ros::Subscriber detect_sub,motor_sub;
    ros::Publisher isreach_pub_,ptz_pub_,zoom_pub_;
    geometry_msgs::PoseStamped c_pos,t_pos;

    unsigned int g_get_info_flag = 1;
    int g_now_xyposition = 0;
    int g_now_zposition = 0;
    unsigned int g_now_zoom = 0;
    unsigned int g_control_type = 0;
    int g_action = 0;
    int g_xy_goal = 0;
    int g_z_goal = 0;
    unsigned int g_reach_flag = 0;
    unsigned int g_xy_reach_flag = 0;
    unsigned int g_z_reach_flag = 0;
    float g_temperature_c = 0.0;
    int pan_max,pan_min;
    int tilt_max,tilt_min;
    int z_diff_val = 0, xy_diff_val = 0;

    std::thread *sock_thread;
    std::unique_ptr<client> tcp_ptr;
    
public:
    std::string device_ip;
    int device_port;
    std::string device_id,ptz_topic,ptz_server_name;
    std::deque<string> _cmd_control_queue;
    std::mutex write_mtx;

    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
    
public:
    pt_control2(const ros::NodeHandle &nh = ros::NodeHandle("~"));
    ~pt_control2();
    bool handle_cloudplatform(fixed_msg::cp_control::Request &req, fixed_msg::cp_control::Response &res);
    void update();
    void tick(const ros::TimerEvent &event);
    void write_hk();
    bool set_action(int id, int type, int value, int xy_value, int z_value, int zoom_value);
    void crc_check(std::vector<unsigned char> &data);
    void motor_callback(const std_msgs::String::ConstPtr& msg);
    //绝对角度
    void motor_absolute_angle(char cmd1,int16_t angle);
    //相对角度
    void motor_relat_angle(char cmd1,int angle);
    // 停止运动
    void stop_move();
    // 连续运动
    void continuous_left(uint8_t speed);
    void continuous_right(uint8_t speed);
    //设置电机当前位置为原点
    void motor_set_ori();
    //读取电机系统实时数据
    void motor_status(char cmd1);
};

#endif