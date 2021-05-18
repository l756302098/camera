#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <string>
#include <sstream>
#include <vector>

#include "ros/ros.h"
#include "image_transport/image_transport.h"
#include "cv_bridge/cv_bridge.h"
#include "sensor_msgs/image_encodings.h"
#include "sensor_msgs/Image.h"
#include <opencv2/opencv.hpp>

#include <fixed_decoder/decoder_h264_node.hpp>
#include <fixed_decoder/H264Decoder.h>
#include <thread>

using namespace std;


cv::Mat g_result_pic;
int g_get_image = 0;
H264Decoder h264_decoder;

void decoder_h264_node::stream_callback(const sensor_msgs::Image& msg)
{
    try
    {
        vector<unsigned char> vc;
        vc = msg.data;
        unsigned char* pBuffer = &vc.at(0);
        int dwBufSize = vc.size();
        // if(pBuffer[4] == 0x67)
        // {
            h264_decoder.decode(pBuffer, dwBufSize);
            g_result_pic = h264_decoder.getMat();
            g_get_image = 1;
        //}
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

decoder_h264_node::decoder_h264_node()
{
    ros::NodeHandle nh_("~");
    std::string raw_topic,decode_topic;
    nh_.param<std::string>("raw_topic", raw_topic, "/yida/heartbeat");
    nh_.param<std::string>("decode_topic", decode_topic, "/yida/heartbeat");
    stream_sub_ = nh_.subscribe(raw_topic, 1, &decoder_h264_node::stream_callback, this, ros::TransportHints().tcpNoDelay(true));
    image_transport::ImageTransport it(nh_);
    image_pub = it.advertise(decode_topic, 1);
}

decoder_h264_node::~decoder_h264_node()
{
}

void decoder_h264_node::update()
{
    if(g_get_image == 1)
    {
        try
        {
            if(!g_result_pic.empty())
            {
                sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", g_result_pic).toImageMsg();
                msg->header.stamp = ros::Time::now();
                msg->header.frame_id = "visible_picture";
                image_pub.publish(msg);
                g_get_image = 0;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        } 
    }
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, __app_name__);
    decoder_h264_node hsdkcap;
    ros::Rate rate(40);
    while (ros::ok())
    {
        hsdkcap.update();
        ros::spinOnce();
        rate.sleep();
    }
    return 0;
}
