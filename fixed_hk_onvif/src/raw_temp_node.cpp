/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-01 13:11:04
 * @LastEditors: li
 * @LastEditTime: 2021-04-22 17:28:08
 */
#include <ros/ros.h>
#include <thread>
#include <iostream>
#include <csignal>
#include "fixed_hk_onvif/raw_temp.hpp"
#include "fixed_hk_onvif/base64.hpp"

#define __app_name__ "raw_temp_node"

bool is_running;

void signalHandler(int signum)
{ 
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    is_running = false;
    // 清理并关闭  
    exit(signum);  
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, __app_name__);
    ros::NodeHandle nh_;
    ROS_INFO_STREAM(__app_name__<<" node started...");
    signal(SIGINT, signalHandler);
    is_running = true;
    std::shared_ptr<raw_temp> ptr(new raw_temp);
    //ros
    ros::Rate rate(30);
    //check auth
    ptr->auth("admin","abcd1234");
    // std::string userpwd = "admin:abcd1234";
    // std::string auth = rtsptool::base64_encode(userpwd);
    // std::cout << "auth:" << auth << std::endl;
    // std::string auth_url = "/ISAPI/Security/userCheck";
    // ptr->get_url(auth_url);
    // std::string cap_url = "/ISAPI/Thermal/capabilities";
    // ptr->get_url(cap_url);
    // std::string url = "/ISAPI/Thermal/channels/2/thermometry/jpegPicWithAppendData?format=json";
    // ptr->get_url(url);
    while (ros::ok() && is_running)
    {
        //ROS_INFO("update...");
        ptr->update();
        ros::spinOnce();
        rate.sleep();
    }
    ROS_INFO("exit...");
    return 0;
}