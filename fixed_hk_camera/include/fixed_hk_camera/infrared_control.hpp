/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-01 13:11:04
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 15:18:13
 */
#ifndef __INFRARED_CONTROL__
#define __INFRARED_CONTROL__

#include "ros/ros.h"
#include <iostream>
#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <mutex>
#include <queue>
#include "fixed_hk_camera/common.hpp"
#include <std_msgs/Int32.h>
#include <nav_msgs/Odometry.h>
#include <diagnostic_msgs/DiagnosticArray.h>
#include "sensor_msgs/Image.h"
#include "fixed_msg/cp_control.h"
#include "fixed_msg/platform_transfer.h"
#include <std_msgs/String.h>
#include <fixed_msg/inspected_result.h>
#include <fixed_msg/detect_result.h>
#include <geometry_msgs/PoseStamped.h>
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
 
class infrared_control
{
private:
    ros::NodeHandle nh_;
    ros::ServiceClient ptz_client;
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
    bool do_task;
    double camera_pixel_size,focus_dis;

private:
    void init(std::string camera_file);
    
public:
    infrared_control(const ros::NodeHandle &nh = ros::NodeHandle("~"));
    ~infrared_control();
    void transfer_callback(const fixed_msg::platform_transfer& msg);
    void isreach_callback(const std_msgs::Int32& msg);
    void detect_rect_callback(const fixed_msg::detect_result& msg);
    void update();
    void reset();
    void read_calibration(std::string _choose_file,cv::Mat &out_RT);
    void target_callback(const geometry_msgs::PoseStampedConstPtr &msg);
    void ptz_callback(const nav_msgs::Odometry& msg);
};

#endif