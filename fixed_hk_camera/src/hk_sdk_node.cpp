/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-01 13:11:04
 * @LastEditors: li
 * @LastEditTime: 2021-04-02 14:01:45
 */
#include <ros/ros.h>
#include <thread>
#include <iostream>
#include <csignal>
#include "fixed_hk_camera/hk_sdk_control.hpp"

#define __app_name__ "hk_sdk_control_node"

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
    hk_sdk_control *hsc = new hk_sdk_control();
    std::thread *write_thread = new std::thread(std::bind(&hk_sdk_control::write_hk, hsc));
    std::thread *read_thread = new std::thread(std::bind(&hk_sdk_control::read_hk, hsc));
    std::thread *rawtemp_thread = new std::thread(std::bind(&hk_sdk_control::read_raw_temp, hsc));
    //ros
    ros::Rate rate(30);
    while (ros::ok() && is_running)
    {
        //ROS_INFO("update...");
        hsc->update();
        ros::spinOnce();
        rate.sleep();
    }
    ROS_INFO("exit...");
    if(write_thread)
        delete write_thread;
    if(read_thread)
        delete read_thread;
    if(rawtemp_thread)
        delete rawtemp_thread;
    return 0;
}