/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-01 13:11:04
 * @LastEditors: li
 * @LastEditTime: 2021-05-13 13:36:13
 */
#ifndef __INFRARED_SLIDE_CONTROL__
#define __INFRARED_SLIDE_CONTROL__

#include "ros/ros.h"
#include <iostream>
#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <mutex>
#include <queue>
#include "fixed_infrared/common.hpp"
#include <std_msgs/Int32.h>
#include <actionlib/client/simple_action_client.h>
#include <nav_msgs/Odometry.h>
#include <diagnostic_msgs/DiagnosticArray.h>
#include "sensor_msgs/Image.h"
#include "fixed_msg/cp_control.h"
#include "fixed_msg/platform_transfer.h"
#include <std_msgs/String.h>
#include <fixed_msg/inspected_result.h>
#include <fixed_msg/detect_result.h>
#include <geometry_msgs/PoseStamped.h>
#include <slide_control_msgs/PositionAction.h>
// Eigen
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <geometry_msgs/PoseStamped.h>
#include <opencv2/opencv.hpp>
#include "eigen3/Eigen/Dense"
#include <opencv2/core.hpp>
#include "opencv2/core/eigen.hpp"
using namespace cv;
using namespace Eigen;
using namespace std;
class infrared_slide_control
{
typedef actionlib::SimpleActionClient<slide_control_msgs::PositionAction> Client;
private:
    ros::NodeHandle nh_;
    Client *client;
	ros::Subscriber transfer_sub,isreach_sub,goal_sub,ptz_sub;
    ros::Publisher camera_pose_pub,infrared_result_pub,meterflag_pub;
    geometry_msgs::PoseStamped c_pos,t_pos;
public:
    std::vector<float> camera_focus_dis;
    std::vector<int> camera_focus_set;
    vector<std::string> msg_list;
    string device_id;
    int camera_id,device_type,visible_survey_parm_id;
    int cali_offset_h,cali_offset_v;
    int camera_image_width,camera_image_height;
    double camera_cmos_width,camera_cmos_height;
    static bool do_task;
    double camera_pixel_size,focus_dis;
    
public:
    infrared_slide_control(const ros::NodeHandle &nh = ros::NodeHandle("~"));
    ~infrared_slide_control();
    void transfer_callback(const fixed_msg::platform_transfer& msg);
    void update();
    void reset();
    void ptz_callback(const nav_msgs::Odometry& msg);
    void connect_server();
    static void activeCb()
    {
        ROS_INFO("Goal just went active");
    }
    static void feedbackCb(const slide_control_msgs::PositionFeedbackConstPtr &feedback)
    {
        ROS_INFO_STREAM_THROTTLE(1,"feedbackCb:"<< feedback->feedback.position);
    }
    static void doneCb(const actionlib::SimpleClientGoalState &state,
                       const slide_control_msgs::PositionResultConstPtr &result)
    {
        if (state == state.SUCCEEDED)
        {
            ROS_INFO("success avoid obs");
            infrared_slide_control::do_task = true;
        }
        else if (state == state.PREEMPTED)
        {
            ROS_INFO("client cancel job.");
        }
        else
        {
            ROS_INFO("failed avoid obs");
            infrared_slide_control::do_task = false;
        }
    }
};

#endif