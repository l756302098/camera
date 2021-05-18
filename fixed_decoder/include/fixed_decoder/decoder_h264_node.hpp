/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2020-06-12 23:50:19
 * @LastEditors: li
 * @LastEditTime: 2021-03-03 11:33:53
 */
#include <iostream>
#include <stdio.h>
//#include <queue>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>

#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CompressedImage.h>
#include <sensor_msgs/image_encodings.h>
#include <diagnostic_msgs/DiagnosticArray.h>
#include <unistd.h>
#include <termios.h>

using namespace cv;
using namespace std;

#define __app_name__ "decoder_h264_node"

typedef  unsigned int       DWORD;
typedef  unsigned short     WORD;
typedef  unsigned short     USHORT;
typedef  short              SHORT;
typedef  int                LONG;
typedef  unsigned char      BYTE;
typedef  unsigned int       UINT;
typedef  void*              LPVOID;
typedef  void*              HANDLE;
typedef  unsigned int*      LPDWORD; 
typedef  unsigned long long UINT64;
typedef  signed long long   INT64;

class decoder_h264_node
{
public:
    bool is_save;
    std::string image_path;
	
private:
    ros::Publisher heartbeat_pub_;
    ros::Subscriber stream_sub_;
    std::string visible_topic_str, heartbeat_topic_str;
    image_transport::Publisher image_pub;
public:
    decoder_h264_node();
    ~decoder_h264_node();
    void update();
    void stream_callback(const sensor_msgs::Image& msg);
};
