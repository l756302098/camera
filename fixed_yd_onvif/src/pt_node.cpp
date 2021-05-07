/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-01 13:11:04
 * @LastEditors: li
 * @LastEditTime: 2021-05-07 17:10:47
 */
#include <ros/ros.h>
#include <thread>
#include <iostream>
#include <csignal>
#include "fixed_yd_onvif/pt_control.hpp"

#define __app_name__ "yd_pt_control_node"

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
    std::shared_ptr<pt_control> ptr(new pt_control);
    std::thread *write_thread = new std::thread(std::bind(&pt_control::write_hk, ptr));
    std::thread *read_thread = new std::thread(std::bind(&pt_control::read_hk, ptr));
    //ros
    ros::Rate rate(10);
    while (ros::ok() && is_running)
    {
        //ROS_INFO("update...");
        ptr->update();
        ros::spinOnce();
        rate.sleep();
    }
    ROS_INFO("exit...");
    if(write_thread)
        delete write_thread;
    if(read_thread)
        delete read_thread;
    return 0;
}