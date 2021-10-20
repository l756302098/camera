#include <ros/ros.h>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "fc_control/fsm.hpp"
#include "fc_control/action_mapping.hpp"

namespace fsm
{
    void action_mapping::init(){
        std::cout << "action_mapping " << name << " init" << std::endl;
        status = fsm::state_enum::RUNNING;
    }
    void action_mapping::run(){
        std::cout << "action_mapping " << name << " run" << std::endl;
    }
    void action_mapping::exit(){
        std::cout << "action_mapping " << name << " exit" << std::endl;
    }
}