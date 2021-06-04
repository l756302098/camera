/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-19 14:32:48
 * @LastEditors: li
 * @LastEditTime: 2021-03-04 16:13:19
 */
#include "ros/ros.h"
#include <image_transport/image_transport.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CompressedImage.h>
#include <sensor_msgs/image_encodings.h>
#include <vector>
#include "fixed_decoder/base64.hpp"

class rtsp_capture{
public:
    pthread_t tid = 0;
    std::string topic_str, rtsp_str;
    bool extra_data;
    ros::Publisher image_pub;
    rtsp_capture();
    ~rtsp_capture();
    void update();
};