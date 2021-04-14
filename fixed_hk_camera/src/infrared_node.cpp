/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-01 13:11:04
 * @LastEditors: li
 * @LastEditTime: 2021-04-12 15:37:33
 */
#include <ros/ros.h>
#include <thread>
#include <iostream>
#include <csignal>
#include "fixed_hk_camera/infrared_control.hpp"

#define __app_name__ "infrared_control_node"

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
    infrared_control *rc = new infrared_control();
    //ros
    ros::Rate rate(30);
    while (ros::ok() && is_running)
    {
        //ROS_INFO("update...");
        rc->update();
        ros::spinOnce();
        rate.sleep();
    }
    ROS_INFO("exit...");
    if(rc)
        delete rc;
    return 0;
}