#include <ros/ros.h>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "fc_control/fsm.hpp"
#include "fc_control/action_coloring.hpp"

namespace fsm
{
    void action_coloring::init(){
        std::cout << "action_coloring " << name << " init" << std::endl;
        status = fsm::state_enum::RUNNING;
    }
    void action_coloring::run(){
        std::cout << "action_coloring " << name << " run" << std::endl;
    }
    void action_coloring::exit(){
        std::cout << "action_coloring " << name << " exit" << std::endl;
    }
}