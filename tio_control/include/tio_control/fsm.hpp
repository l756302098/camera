/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-22 13:21:17
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 14:43:09
 */
#pragma once

#include <ros/ros.h>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "glog/logging.h"
#include "gflags/gflags.h"

namespace fsm
{
    enum fsm_enum{
        IDLE = 1,COLORRING, MAPPING, UPLOAD
    };
	enum action_enum{
		INIT = 1, RUNNING, FINISH, TIMEOUT, UNKNOWN,
	};
    typedef std::function<void(int,int,std::string,float)> deal_func;

    class state_base
    {
    public:
        /* data */
        std::string name;
        fsm_enum status;
        std::vector<std::string> args;
        deal_func callback;
    public:
        state_base(/* args */){status = fsm::fsm_enum::IDLE;}
        ~state_base(){}
        void set_callback(deal_func cb){
           callback = cb; 
        }
        virtual void init(){
            std::cout << "state base " << name << " init" << std::endl;
        }
        virtual void run(){
            std::cout << "state base " << name << " run" << std::endl;
        }
        virtual void exit(){
            std::cout << "state base " << name << " exit" << std::endl;
        }
        virtual bool play(uint8_t flag,std::vector<std::string> &data){
            std::cout << "state base " << name << " play " << flag << std::endl;
        }
    };

	class manager {
    public:
        manager(){}
        ~manager() {}

		void loop(){
            //std::cout << "fsm::manager loop" << std::endl;
			if(current_state!=NULL)
				current_state->run();
		}

        // number of children (manager)
        size_t size() const {
            return states.size();
        }

		void add_state(std::shared_ptr<fsm::state_base> state){
			states[state->status] = state;
		}

		void set_state(fsm::fsm_enum ftype) {
            if(current_state!=NULL)
            {
                if(current_state->status != ftype){
                    current_state->exit();
                }else{
                    LOG(ERROR) << " set_state failed because current status is target status";
                    return;
                }
            }
            current_state = states[ftype];
			current_state->init();
        }

        std::shared_ptr<fsm::state_base> get_current(){
            return current_state;
        }

        fsm::fsm_enum get_status(){
            if(current_state==NULL) return fsm::fsm_enum::IDLE;
            return current_state->status;
        }

		// fsm::action_enum wait_for_result(){
		// 	ros::Rate loop_rate(5);
		// 	while(ros::ok() && current_state->status != fsm::action_enum::RUNNING)
		// 	{
		// 		/* code */
		// 		current_state->run();
		// 		ros::spinOnce();
  		// 		loop_rate.sleep();
		// 	}
			
		// 	return current_state->status;
		// }

    protected:
		std::map<fsm::fsm_enum, std::shared_ptr<fsm::state_base>> states;
        std::shared_ptr<fsm::state_base> current_state;
    };
}