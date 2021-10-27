/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-22 13:21:17
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 14:43:09
 */
#ifndef __ACTION_MAPPING_H__
#define __ACTION_MAPPING_H__

#include <ros/ros.h>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "tio_control/fsm.hpp"
#include "mapping_msg/start_mapping.h"
#include "mapping_msg/stop_mapping.h"
#include "mapping_msg/quit_mapping.h"
#include "mapping_msg/collect_data.h"
#include "mapping_msg/mapping_progress.h"
#include "mapping_msg/mapping_status.h"
#include "fixed_msg/cp_control.h"
#include <std_msgs/Int32.h>

namespace fsm
{
    enum  mapping_enum{NONE = 0, START, MOTOR , WAITFOR_MOTOR ,COLLECT, WAITFOR_COLLECT, STOP, WAITFOR_COLOR, NOR_FINISH ,ERROR_FINISH, QUIT,PAUSE,GOON};
    enum  mapping_error_enum{NORMAL = 0,MAP_FINISH,MAP_QUIT, MOTOR_TIMEOUT,MOTOR_NORESPNSE ,COLLECT_TIMEOUT, COLOR_TIMEOUT,MAPPING_NORESPNSE,NO_DATA };

    class action_mapping: public state_base
    {
    private:
        ros::NodeHandle nh_;
        ros::ServiceClient ptz_client,mapping_start_client,mapping_stop_client,mapping_quit_client,collect_data_client;
        ros::Subscriber status_sub,process_sub,arrive_sub,collected_sub;
        bool is_init,is_pause;
        ros::Time last,start_wait_time;
        int device_id,task_id,map_id, flag = 0;
        int collect_count, mottor_timeout_count = 0;
        float motor_angle_x,motor_angle_z;
        float target_angle_x,target_angle_z;
    private:
        bool start_mapping(std::vector<std::string> &data);
        bool stop_mapping();
        bool quit_mapping(std::vector<std::string> &data);
        bool collect_data();
        void motor();
        void status_cb(const mapping_msg::mapping_status::ConstPtr &msg);
        void progress_cb(const mapping_msg::mapping_progress::ConstPtr &msg);
        void arrive_cb(const std_msgs::Int32::ConstPtr &msg);
        void collect_cb(const std_msgs::Int32::ConstPtr &msg);
        bool time_tick(int seconds);
        void response(fsm::mapping_error_enum code,std::string &error_msg);
        void reset();
    public:
        mapping_enum current_status;
    public:
        action_mapping():is_pause(false),is_init(false){status = fsm::fsm_enum::MAPPING;}
        void init();
        void run();
        void exit();
        bool play(uint8_t flag,std::vector<std::string> &data);
    };
}
#endif