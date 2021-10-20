/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-22 13:21:17
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 14:43:09
 */
#include <ros/ros.h>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "fc_control/fsm.hpp"

namespace fsm
{
    class action_coloring: public state_base
    {
    public:
        action_coloring(std::string name): state_base(name){status = fsm::state_enum::INIT;}
        void init();
        void run();
        void exit();
    };
}