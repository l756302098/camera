/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-21 10:14:18
 * @LastEditors: li
 * @LastEditTime: 2021-04-21 15:39:49
 */
#ifndef __RAW_TEMPERATURE__
#define __RAW_TEMPERATURE__
#include "ros/ros.h"
#include <iostream>
#include <stdio.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xml/pugixml.hpp" 
#include "httplib.h"
#include "base64.hpp"

class raw_temp
{
private:
    ros::NodeHandle nh_;
public:
    raw_temp(const ros::NodeHandle &nh = ros::NodeHandle("~"));
    ~raw_temp();
    void update();
    int get_url(std::string& url);
};
#endif