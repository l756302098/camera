/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-20 10:48:32
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 18:14:36
 */
#include <ros/ros.h>
#include <boost/filesystem.hpp>
#include "glog/logging.h"
#include "gflags/gflags.h"
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

#include <dirent.h>
int MAX_SAVE_DAYS = 5;
std::string checkDir(const std::string& LOGPATH)
{
    if (!boost::filesystem::exists(LOGPATH))
    {
        boost::filesystem::create_directory(LOGPATH);
    }
    time_t raw_time;
    struct tm* tm_info;

    time(&raw_time);
    tm_info = localtime(&raw_time);

    //printf("time_now:%d%d%d%d%d%d.%ld(us)\n", 1900+tm_info->tm_year, 1+tm_info->tm_mon, tm_info->tm_mday, tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    char day[128] = {0};
    sprintf(day, "%s%04d%02d%02d", LOGPATH.c_str(), 1900+tm_info->tm_year, 1+tm_info->tm_mon, tm_info->tm_mday);
    
    std::string log_path = day;
    if (!boost::filesystem::exists(log_path))
    {
        boost::filesystem::create_directory(log_path);
    }
    return log_path;
}
int readFilesInDir(const char *basePath)
{
    DIR *dir;
    struct dirent *ptr;

    if ((dir=opendir(basePath)) == NULL){
        perror("Open dir error...");
        return -1;
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;
        else{

            printf("rm file:%s\n", ptr->d_name);
            char file[128] = {0};
            sprintf(file, "%s/%s", basePath, ptr->d_name);
            int ret = unlink(file);
            if(ret != 0){
                printf("unlink %s failed!\n", file);
                closedir(dir);
                return -1;
            }
        }
    }
    closedir(dir);
    return 0;
}

int rmdir2(const char *path)
{
    int ret = readFilesInDir(path);
    if(ret != 0)
        return -1;

    ret = rmdir(path);
    if(ret != 0){
        printf("rm %s failed!\n", path);
        return -1;
    }

    return 0;
}

bool rmOldDay(const std::string& LOGPATH)
{
    struct dirent **namelist;
    int n = 0, i = 0;

    n = scandir(LOGPATH.c_str(), &namelist, 0, alphasort);
    printf("scandir %i \n",n);
    for (size_t i = 0; i < n; i++)
    {
        std::cout << " namelist " << namelist[i]->d_name << std::endl;
    }
    
    if(n > 0){
        int dir_index = 0;
        int max_index = n-2-MAX_SAVE_DAYS;
        while(dir_index < max_index){
            printf("dir index:%i max index:%i",dir_index,max_index);
            char first[128] = {0};
            memcpy(first, namelist[2+dir_index]->d_name, sizeof(first));
            sprintf(first, "%s%s", LOGPATH.c_str(), namelist[2+dir_index]->d_name);
            printf("rm old:%s\n", first);
            int ret = rmdir2(first);
            if(ret != 0){
                printf("rm %s failed!\n", first);
            }
            dir_index += 1;
        }

        for (i = 0; i < n; i++)
        {
            free(namelist[i]);
        }
        free(namelist);
    }

    return true;
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, __app_name__);
    ros::NodeHandle nh_("~");
    if(argc < 3){
        printf("input params error");
        return -1;
    }
    //set glog
    google::InitGoogleLogging(argv[0]);
    //所有级别的日志同时输出到标准输出设备
    google::LogToStderr();
    //缓冲日志输出，默认为30秒，此处改为立即输出
    FLAGS_logbufsecs = 0;
    // 设置记录到标准输出的颜色消息（如果终端支持）
    FLAGS_colorlogtostderr = true;
    std::string home = argv[1];
    std::cout << "home path:" << home << std::endl;
    int save_days = atoi(argv[2]);
    MAX_SAVE_DAYS = save_days;
    std::cout << "save_days:" << MAX_SAVE_DAYS << std::endl;
    rmOldDay(home);
    home = checkDir(home);
    std::cout << "log path:" << home << std::endl;
    std::string info_log = home + "/info_";
    google::SetLogDestination(google::GLOG_INFO, info_log.c_str());
    std::string warning_log = home + "/warning_";
    google::SetLogDestination(google::GLOG_WARNING, warning_log.c_str());
    std::string error_log = home + "/error_";
    google::SetLogDestination(google::GLOG_ERROR, error_log.c_str());
    std::string fatal_log = home + "/fatal_";
    google::SetLogDestination(google::GLOG_FATAL, fatal_log.c_str());
    google::ParseCommandLineFlags(&argc, &argv, true);
    //ROS_INFO("node started...");
    LOG(INFO) << "fixed control node started...";

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