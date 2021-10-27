/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-22 13:21:17
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 14:43:09
 */
#ifndef __ACTION_IDLE_H__
#define __ACTION_IDLE_H__
#include <ros/ros.h>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "tio_control/fsm.hpp"

namespace fsm
{
    class action_idle: public state_base
    {
    public:
        action_idle(){status = fsm::fsm_enum::IDLE;}
        void init();
        void run();
        void exit();
        bool play(uint8_t flag,std::vector<std::string> &data);
    };
}
#endif