/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-01 13:11:04
 * @LastEditors: li
 * @LastEditTime: 2021-05-08 10:37:21
 */
#ifndef __VISIBLE_CONTROL__
#define __VISIBLE_CONTROL__

#include "ros/ros.h"
#include <iostream>
#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <mutex>
#include <queue>
#include "fixed_yd_onvif/common.hpp"
#include <std_msgs/Int32.h>
#include <nav_msgs/Odometry.h>
#include <diagnostic_msgs/DiagnosticArray.h>
#include "sensor_msgs/Image.h"
#include "fixed_msg/cp_control.h"
#include "yidamsg/transfer.h"
#include <yidamsg/InspectedResult.h>
#include <yidamsg/Detect_Result.h>
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
 
class visible_control
{
private:
    ros::NodeHandle nh_;
    ros::ServiceClient ptz_client;
	ros::Subscriber transfer_sub,isreach_sub,goal_sub,detectresult_sub,ptz_sub;
    ros::Publisher camera_pose_pub,readyimage_pub;
    geometry_msgs::PoseStamped c_pos,t_pos;
public:
    std::vector<float> camera_focus_dis;
    std::vector<int> camera_focus_set;
    vector<std::string> msg_list;
    string device_id;
    int camera_id,device_type,watch_counter;
    int cali_offset_h,cali_offset_v;
    int camera_image_width,camera_image_height;
    double camera_cmos_width,camera_cmos_height;
    bool do_task,auto_zoom;
    double camera_pixel_size,focus_dis;
    float yaw_first,pitch_first;
    double h_offset,v_offset;

private:
    void init(std::string camera_file);
    
public:
    visible_control(const ros::NodeHandle &nh = ros::NodeHandle("~"));
    ~visible_control();
    void transfer_callback(const yidamsg::transfer& msg);
    void isreach_callback(const std_msgs::Int32& msg);
    void detect_rect_callback(const yidamsg::Detect_Result& msg);
    void ptz_callback(const nav_msgs::Odometry& msg);
    void update();
    void reset();
    void read_calibration(std::string _choose_file,cv::Mat &out_RT);
    void target_callback(const geometry_msgs::PoseStampedConstPtr &msg);
    int get_zoomset(float distance, int device_width, int device_height, int zoom_scale = 5);
};

#endif