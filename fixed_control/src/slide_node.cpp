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
#include "fixed_control/slide_control.hpp"

#define __app_name__ "slide_control_node"

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
    ROS_INFO("node started...");
    signal(SIGINT, signalHandler);
    is_running = true;
    slide_control control;
    std::string finish_topic;
    nh_.param<std::string>("finish_topic",finish_topic,"/meter_flag");
    std::cout << "subscribe topic " << finish_topic << std::endl;
    ros::Subscriber meter_sub = nh_.subscribe(finish_topic, 1, &slide_control::meter_cb,&control);
    ros::Subscriber mode_sub = nh_.subscribe("/fixed/control/mode_control", 1, &slide_control::mode_cb,&control);
    ros::ServiceServer task_server = nh_.advertiseService("/fixed/control/task", &slide_control::task_srv, &control);
    ros::ServiceServer clear_server = nh_.advertiseService("/fixed/control/task_control", &slide_control::task_clear_srv, &control);
    ros::Timer timer = nh_.createTimer(ros::Duration(0.1), &slide_control::tick, &control, false);
    ros::MultiThreadedSpinner s(4);
    ros::spin(s);
    ROS_INFO("exit...");
    return 0;
}