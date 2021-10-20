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

namespace fsm
{
	enum state_result{
		INIT = 1, RUNNING, FINISH, TIMEOUT, UNKNOWN
	};

    class state_base
    {
    public:
        /* data */
        std::string name;
        state_result status;
        std::vector<std::string> args;
    public:
        state_base(/* args */){status = state_result::INIT;}
        ~state_base(){}
        virtual void init(){
            std::cout << "state base " << name << " init" << std::endl;
            status = state_result::RUNNING;
        }
        virtual void run(){
            std::cout << "state base " << name << " run" << std::endl;
        }
        virtual void exit(){
            std::cout << "state base " << name << " exit" << std::endl;
        }
    };

	class manager {
    public:
        manager(){
        }

        ~manager() {
            
        }

		void loop(){
			if(current_state!=NULL)
				current_state->run();
		}

        // number of children (manager)
        size_t size() const {
            return states.size();
        }

		void add_state(std::shared_ptr<fsm::state_base> state){
			states[state->name] = state;
		}

		void set_state(const std::string &state_name ) {
			current_state->exit();
            current_state = states[state_name];
			current_state->init();
        }

		fsm::state_result wait_for_result(){
			ros::Rate loop_rate(5);
			while(ros::ok() && current_state->status != fsm::state_result::RUNNING)
			{
				/* code */
				current_state->run();
				ros::spinOnce();
  				loop_rate.sleep();
			}
			
			return current_state->status;
		}

    protected:
		std::map< std::string, std::shared_ptr<fsm::state_base>> states;
        std::shared_ptr<fsm::state_base> current_state;
    };
}