#include <ros/ros.h>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "tio_control/fsm.hpp"
#include "tio_control/action_idle.hpp"

namespace fsm
{
    void action_idle::init(){
        std::cout << "action_idle " << name << " init" << std::endl;
    }
    void action_idle::run(){
        //std::cout << "action_idle " << name << " run" << std::endl;
    }
    void action_idle::exit(){
        std::cout << "action_idle " << name << " exit" << std::endl;
    }
    bool action_idle::play(uint8_t flag,std::vector<std::string> &data){
        return false;
    }
}