/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-21 10:14:18
 * @LastEditors: li
 * @LastEditTime: 2021-04-23 12:55:39
 */
#ifndef __RAW_TEMPERATURE__
#define __RAW_TEMPERATURE__
#include "ros/ros.h"
#include <iostream>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "xml/pugixml.hpp" 
#include "httplib.h"
#include "base64.hpp"
#include <vector>
#include <map>
#include "fixed_hk_onvif/common.hpp"
#include "fixed_hk_onvif/base64.hpp"
#include <openssl/md5.h>
#define HTTP_DEGIST_SRC_LEN 128
#define HTTP_HA_LEN 64

using namespace httplib;

class raw_temp
{
private:
    ros::NodeHandle nh_;
public:
    raw_temp(const ros::NodeHandle &nh = ros::NodeHandle("~"));
    ~raw_temp();
    void update();
    int get_url(std::string& url);
    bool auth(std::string name,std::string pwd);
    string MD5(const string& src);
};
#endif