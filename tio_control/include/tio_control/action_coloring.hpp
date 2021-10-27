/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-22 13:21:17
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 14:43:09
 */
#ifndef __ACTION_COLORING_H__
#define __ACTION_COLORING_H__
#include <ros/ros.h>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "tio_control/fsm.hpp"
#include "tf_ircampose_msg/start_pose.h"
#include "tf_ircampose_msg/stop_pose.h"
#include "undistort_service_msgs/start_undistort.h"
#include "undistort_service_msgs/stop_undistort.h"
#include "fixed_msg/cp_control.h"
#include <std_msgs/Int32.h>
#include <fixed_msg/inspected_result.h>
#include "tio_control/task_control.hpp"

namespace fsm
{
    enum coloring_enum{COR_NONE = 0, COR_START, COR_NOR_FINISH ,COR_ERROR_FINISH, 
                    COR_MOTOR,COR_WAITFOR_MOTOR,
                    COR_START_POSE,COR_STOP_POSE,COR_START_UNDISTORT,COR_STOP_UNDISTORT,COR_WAIT_COLOR,
                    COR_QUIT,COR_PAUSE,COR_GOON};
    enum  coloring_error_enum{COR_NORMAL = 0,COR_FINISH,COR_EXIT, COR_MOTOR_TIMEOUT,COR_MOTOR_NORESPNSE,COR_POSE_NORESPNSE ,COR_UNDISTORT_NORESPNSE,COR_DATA_EXP };

    class action_coloring: public state_base
    {
    private:
        ros::NodeHandle nh_;
        ros::ServiceClient ptz_client,tf_pose_start_client,tf_pose_stop_client,undistort_start_client,undistort_stop_client;
        ros::Subscriber arrive_sub,coloring_finish_sub;
        ros::Publisher coloring_start_pub;
        bool is_init,is_pause;
        ros::Time last,start_wait_time;
        int device_id,task_id,map_id=0;
        std::string coloring_pub_info;
        int move_level1,move_level2,mottor_timeout_count = 0;
        float motor_angle_x,motor_angle_z;
        float target_angle_x,target_angle_z;
    private:
        void arrive_cb(const std_msgs::Int32::ConstPtr &msg);
        bool start_coloring(std::vector<std::string> &data);
        bool quit_coloring();
        void reset();
        void motor();
        void start_pose();
        bool stop_pose();
        void start_undistort();
        bool stop_undistort();
        void go_next();
        void response(fsm::coloring_error_enum code,std::string &error_msg);
        bool time_tick(int seconds);
        void task_finish_cb(const std_msgs::String msg);
    public:
        coloring_enum current_status;
    public:
        action_coloring():is_pause(false),is_init(false){status = fsm::fsm_enum::COLORRING;}
        void init();
        void run();
        void exit();
        bool play(uint8_t flag,std::vector<std::string> &data);
    };
}
#endif