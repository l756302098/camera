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
#include "fixed_control/status_control.hpp"

#define __app_name__ "fixed_control_node"

bool is_running;
void signalHandler(int signum)
{ 
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    is_running = false;
    // 清理并关闭  
    exit(signum);  
}
/*
void test(){
    mfloat_t position[VEC3_SIZE];
    mfloat_t target[VEC3_SIZE];
    mfloat_t up[VEC3_SIZE];
    mfloat_t view[MAT4_SIZE];

    position[0] = 0;
    position[1] = 0;
    position[2] = 10;

    target[0] = 0;
    target[1] = 0;
    target[2] = 0;

    up[0] = 0;
    up[1] = 0;
    up[2] = 1;
    
    mat4_look_at(view,position,target,up);
}
*/
int main(int argc, char **argv)
{
    ros::init(argc, argv, __app_name__);
    ros::NodeHandle nh_("~");

    signal(SIGINT, signalHandler);
    is_running = true;
    status_control control;
    std::string finish_topic;
    nh_.param<std::string>("finish_topic",finish_topic,"/meter_flag");
    std::cout << "subscribe topic " << finish_topic << std::endl;
    ros::Subscriber meter_sub = nh_.subscribe(finish_topic, 1, &status_control::meter_cb,&control);
    ros::Subscriber mode_sub = nh_.subscribe("/fixed/control/mode_control", 1, &status_control::mode_cb,&control);
    ros::ServiceServer task_server = nh_.advertiseService("/fixed/control/task", &status_control::task_srv, &control);
    ros::ServiceServer clear_server = nh_.advertiseService("/fixed/control/task_control", &status_control::task_clear_srv, &control);
    ros::Timer timer = nh_.createTimer(ros::Duration(0.1), &status_control::tick, &control, false);
    ros::MultiThreadedSpinner s(4);
    ros::spin(s);
    ROS_INFO("exit...");
    return 0;
}