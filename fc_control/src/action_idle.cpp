#include <ros/ros.h>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "fc_control/fsm.hpp"
#include "fc_control/action_idle.hpp"

namespace fsm
{
    void action_idle::init(){
        std::cout << "action_idle " << name << " init" << std::endl;
        status = fsm::state_enum::RUNNING;
    }
    void action_idle::run(){
        std::cout << "action_idle " << name << " run" << std::endl;
    }
    void action_idle::exit(){
        std::cout << "action_idle " << name << " exit" << std::endl;
    }
}