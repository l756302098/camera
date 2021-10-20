/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-22 13:21:17
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 14:43:09
 */
#include <ros/ros.h>
#include "std_msgs/String.h"
#include "geometry_msgs/Point32.h"
#include <thread>
#include <fstream>
#include <iostream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/encodedstream.h>
#include <rapidjson/document.h>
#include "fc_control/fsm.hpp"
#include "fc_control/action_idle.hpp"
#include "fc_control/action_coloring.hpp"
#include "fc_control/action_mapping.hpp"
#include "fc_control/glod_extension.hpp"
#include "fc_control/common.hpp"
#include "glog/logging.h"
#include "gflags/gflags.h"

using namespace rapidjson;
using namespace std;

class fc_control
{
private:
	/* data */
	ros::NodeHandle nh_;
	ros::ServiceServer task_srv,task_control_srv;
	NodeOption option_;
	std::unique_ptr<fsm::manager> brain_;
	std::map<fsm::action_enum,std::string> actions_;
public:
	fc_control(const ros::NodeHandle &nh = ros::NodeHandle("~"));
	~fc_control();
	void tick(const ros::TimerEvent &event);
    void update();
	void load_params();
	void launch_client();
	void launch_server();
	void launch_subscribers();
	void launch_publishs();
};
