/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-20 10:48:32
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 18:14:36
 */
#include <ros/ros.h>
#include <thread>
#include <iostream>
#include <csignal>
#include "fixed_control/fsm.hpp"
// #include "fixed_control/state.hpp"

#define __app_name__ "fsm_node"

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
    ros::NodeHandle nh_("~");

    signal(SIGINT, signalHandler);
    is_running = true;
    ros::Rate loop_rate(10);
    int count = 0;
    while (ros::ok())
    {

        ros::spinOnce();

        loop_rate.sleep();
        ++count;
    }
    ROS_INFO("exit...");
    return 0;
}