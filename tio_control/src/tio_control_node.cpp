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
#include "tio_control/task_control.hpp"
#include "tio_control/glod_extension.hpp"
#include "glog/logging.h"
#include "gflags/gflags.h"
#define __app_name__ "ftp_control_node"

void glod_init(int argc, char **argv){
    std::string log_path = argv[1];
    int save_days = atoi(argv[2]);
    std::cout << "log_path:" << log_path << "save_days:" << save_days << std::endl;

    rmOldDay(log_path,save_days);
    std::string save_path = checkDir(log_path);

    //set glog
    google::InitGoogleLogging(argv[0]);
    //所有级别的日志同时输出到标准输出设备
    google::LogToStderr();
    //缓冲日志输出，默认为30秒，此处改为立即输出
    FLAGS_logbufsecs = 0;
    // 设置记录到标准输出的颜色消息（如果终端支持）
    FLAGS_colorlogtostderr = true;
    std::cout << "log path:" << save_path << std::endl;
    std::string info_log = save_path + "/info_";
    google::SetLogDestination(google::GLOG_INFO, info_log.c_str());
    std::string warning_log = save_path + "/warning_";
    google::SetLogDestination(google::GLOG_WARNING, warning_log.c_str());
    std::string error_log = save_path + "/error_";
    google::SetLogDestination(google::GLOG_ERROR, error_log.c_str());
    std::string fatal_log = save_path + "/fatal_";
    google::SetLogDestination(google::GLOG_FATAL, fatal_log.c_str());
    google::ParseCommandLineFlags(&argc, &argv, true);
    LOG(INFO) << "fc control node glod_init finish";
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, __app_name__);
    if(argc < 3){
        std::cerr << "input params not enough" << std::endl;
        std::cout << "usage: " << std::endl;
        std::cout << "argv [0]:app name" << std::endl;
        std::cout << "argv [1]:app log path" << std::endl;
        std::cout << "argv [2]:app log save days" << std::endl;
        return 0;
    }
    glod_init(argc,argv);
    ros::NodeHandle nh_("~");
    task_control control;
    ros::Timer tick_timer = nh_.createTimer(ros::Duration(1), &task_control::tick, &control, false);
    ros::Timer loop_timer = nh_.createTimer(ros::Duration(1), &task_control::loop, &control, false);
    ros::MultiThreadedSpinner s(4);
    ros::spin(s);
    LOG(INFO) << "tio control node exit ";
    return 0;
}