/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-01 13:11:04
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 15:07:15
 */
#ifndef __HK_SDK_CONTROL__
#define __HK_SDK_CONTROL__

#include "ros/ros.h"
#include <iostream>
#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <mutex>
#include <queue>
#include "fixed_hk_camera/common.hpp"
#include <std_msgs/Int32.h>
#include <nav_msgs/Odometry.h>
#include <diagnostic_msgs/DiagnosticArray.h>
#include "sensor_msgs/Image.h"
#include "fixed_msg/cp_control.h"
#include "fixed_msg/detect_result.h"

#include "hk/HCNetSDK.h"
#include "hk/LinuxPlayM4.h"
#include "hk/convert.h"

using namespace std;
 
class hk_sdk_control
{
private:
    ros::NodeHandle nh_;

    std::string app_name;
    std::string ptz_topic,ptz_server_name,raw_temp_topic;
    std::string visible_topic_str, heartbeat_topic_str, sdkcom_dir_str;
    bool m_set_infrared_focus,pub_raw_temp;

    ros::Publisher heartbeat_pub_,ptz_pub_,isreach_pub_,raw_temp_pub_;
    ros::Subscriber detect_sub;
    ros::ServiceServer ptz_server,raw_temp_server;

    //sdk
    std::string m_device_ip, m_device_port, m_device_username, m_device_password;
    std::string m_image_width, m_image_height;
    long lUserID;
    long lRealPlayHandle;

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
    bool auto_zoom;

public:
    string device_id;
    std::deque<string> _cmd_control_queue, _cmd_read_queue;
    int _set_infrared_focus_waittime;
    std::mutex write_mtx,read_mtx;

private:
    bool set_action(int id, int type, int value, int xy_value, int z_value, int zoom_value);
    int get_action(int id, int type);
    void pub_heartbeat(int level, string message);
    void init_sdk();
    
public:
    hk_sdk_control(const ros::NodeHandle &nh = ros::NodeHandle("~"));
    ~hk_sdk_control();
    void update();
    void write_hk();
    void read_hk();
    void read_raw_temp();
    void set_focusmode();
    bool handle_cloudplatform(fixed_msg::cp_control::Request &req, fixed_msg::cp_control::Response &res);
    void detect_rect_callback(const fixed_msg::detect_result &msg);
};

#endif