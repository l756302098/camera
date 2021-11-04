/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-22 13:21:17
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 14:43:09
 */
#ifndef __TASK_CONTROL_H__
#define __TASK_CONTROL_H__

#include <ros/ros.h>
#include "std_msgs/String.h"
#include "geometry_msgs/Point32.h"
#include <thread>
#include <fstream>
#include <iostream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/encodedstream.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include "tio_control/fsm.hpp"
#include "tio_control/action_idle.hpp"
#include "tio_control/action_coloring.hpp"
#include "tio_control/action_mapping.hpp"
#include "tio_control/action_upload.hpp"
#include "tio_control/common.hpp"
#include "tio_control/string_extension.hpp"
#include "tio_control/glod_extension.hpp"
#include "tio_control/mapping_task.h"
#include "tio_control/coloring_task.h"
#include "tio_control/control_task.h"
#include "tio_control/task_status.h"
#include "glog/logging.h"
#include "gflags/gflags.h"

using namespace rapidjson;
using namespace std;

class task_control
{
private:
	/* data */
	ros::NodeHandle nh_;
	ros::Publisher task_status_pub,web_status_pub,web_progress_pub;
	ros::ServiceServer mapping_srv,control_mapping_srv,coloring_srv,control_coloring_srv;
    ros::Subscriber web_data_sub,web_command_sub;
	std::unique_ptr<fsm::manager> brain_;
	TaskContext task_context;
public:
	static NodeOption option_;
private:
	void load_params();
	std::string task_name(){
		fsm::fsm_enum  ffe = brain_->get_status();
		switch (ffe)
		{
		case fsm::fsm_enum::IDLE:
			return "idle";
		case fsm::fsm_enum::MAPPING:
			return "mapping";
		case fsm::fsm_enum::COLORRING:
			return "coloring";
        case fsm::fsm_enum::UPLOAD:
			return "upload";
		default:
			break;
		}
		return "unknown";
	}
	void reset(){
		task_context.task_id = 0;
		task_context.map_id = 0;
		task_context.resolution_flag = 0;
		task_context.task_name = "idle";
		task_context.task_code = 0;
		task_context.task_progress = 0;
        task_context.task_status = 4;
		task_context.message = "ok";
        brain_->set_state(fsm::fsm_enum::IDLE);
	}
private:
	void launch_client();
	void launch_server();
	void launch_subscribers();
	void launch_publishs();
	bool mapping_task_cb(tio_control::mapping_task::Request &req,tio_control::mapping_task::Response &res);
	bool control_mapping_cb(tio_control::control_task::Request &req,tio_control::control_task::Response &res);
	void deal_callback(int module,int code,std::string msg,float percent);
	bool coloring_task_cb(tio_control::coloring_task::Request &req,tio_control::coloring_task::Response &res);
    bool control_coloring_cb(tio_control::control_task::Request &req,tio_control::control_task::Response &res);
    void web_data_cb(const std_msgs::String::ConstPtr& msg);
    void web_command_cb(const std_msgs::String::ConstPtr& msg);
    bool control_mapping(int flag);
    bool control_coloring(int flag);
    bool control_upload(int flag);
    void pub_status_web();
    void pub_progress_web();
public:
	task_control(const ros::NodeHandle &nh = ros::NodeHandle("~"));
	~task_control(){}
	void tick(const ros::TimerEvent &event);
    void loop(const ros::TimerEvent &event);
    void update();
	int load_from_stream(std::string &in){
        LOG(WARNING) << "out:";
        LOG(WARNING) << in;
        const char* json = in.c_str();
        StringStream ss(json);
        rapidjson::Document doc;
        if (doc.ParseStream(ss).HasParseError())
        {
            LOG(ERROR) << "Failed to parse json";
            return -1;
        }
        return resolve_coloring_json(doc);
    }
    int load_task_file(const std::string &file){
		std::ifstream in(file.c_str());
        if (!in)
        {
			LOG(ERROR) << "Failed to open" << file.c_str();
            return -1;
        }
        rapidjson::IStreamWrapper isw(in);
        rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::IStreamWrapper> eis(isw);
        rapidjson::Document doc;
        if (doc.ParseStream(eis).HasParseError())
        {
			LOG(ERROR) << "Failed to parse json";
            return -1;
        }
        return resolve_coloring_json(doc);
	}
	int load_from_file(const std::string &file){
		std::ifstream in(file.c_str());
        if (!in)
        {
			LOG(ERROR) << "Failed to open" << file.c_str();
            return -1;
        }
        rapidjson::IStreamWrapper isw(in);
        rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::IStreamWrapper> eis(isw);
        rapidjson::Document doc;
        if (doc.ParseStream(eis).HasParseError())
        {
			LOG(ERROR) << "Failed to parse json";
            return -1;
        }
        return resolve_motor_json(doc);
	}
	int resolve_motor_json(rapidjson::Document &doc){
		if(!doc.IsArray()){
			LOG(ERROR) << "json is not array";
            return -1;
        }
		static const char* kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };
		for (SizeType i = 0; i < doc.Size(); i++){
			std::shared_ptr<WatchNode> point(new WatchNode());
			const rapidjson::Value &obj = doc[i];
			for (Value::ConstMemberIterator itr = obj.MemberBegin();itr != obj.MemberEnd(); ++itr){
				std::string type = kTypeNames[itr->value.GetType()];
                std::string key = itr->name.GetString();
				//LOG(WARNING) << "key:" << key.c_str() << " type:" << type.c_str();
				if (key == "watch_id")
                {
					LOG(WARNING) << "watch_id:" <<  itr->value.GetInt();
					point->id = itr->value.GetInt();
                }
                else if (key == "watch_posx")
                {
					LOG(WARNING) << "watch_posx:" << itr->value.GetFloat();
					point->angle_x = itr->value.GetFloat();
                }
				else if (key == "watch_posz")
                {
					LOG(WARNING) << "watch_posz:" << itr->value.GetFloat();
					point->angle_z = itr->value.GetFloat();
                }
			}
			task_control::option_.motor_trajectory.push_back(point);
		}
		return 1;
	}
	int resolve_coloring_json(rapidjson::Document &doc){
        if(!doc.IsObject()){
            ROS_WARN("json is not object");
            return -1;
        }
        if (!doc.HasMember("Tasks"))
        {
            ROS_WARN("No 'Tasks' field");
            return -1;
        }

        const rapidjson::Value &tasks = doc["Tasks"];
        if (!tasks.IsArray())
        {
            ROS_WARN("Invalid 'Tasks' field");
            return -1;
        }
        const rapidjson::Value &iid = doc["InspectId"];
        if(iid.IsString()){
            LOG(WARNING) << "InspectId:" << iid.GetString();
        }
        const rapidjson::Value &rid = doc["RobotId"];
        if(rid.IsNumber()){LOG(WARNING) << "RobotId:" << rid.GetInt();}
        const rapidjson::Value &thid = doc["TaskHistoryId"];
        if(thid.IsNumber()){LOG(WARNING) << "TaskHistoryId:" << thid.GetInt();}
        const rapidjson::Value &tid = doc["TaskId"];
        if(tid.IsNumber()){LOG(WARNING) << "TaskId:" << tid.GetInt();}
        static const char* kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };
        for (SizeType i = 0; i < tasks.Size(); i++) // 使用 SizeType 而不是 size_t
        {
            std::shared_ptr<ROAD_PLAN> task(new ROAD_PLAN());
            //ROAD_PLAN task;
            const rapidjson::Value &obj = tasks[i]; 
            for (Value::ConstMemberIterator itr = obj.MemberBegin();itr != obj.MemberEnd(); ++itr){
                std::string type = kTypeNames[itr->value.GetType()];
                std::string key = itr->name.GetString();
                //LOG(WARNING) << "" << itr->name.GetString() << " " << type ;
                if (key == "Align")
                {
                    LOG(WARNING) << "Align:" << itr->value.GetString();
                    task->road = itr->value.GetString();
                }
                else if (key == "CameraPose")
                {
                    LOG(WARNING) << "CameraPose:" << itr->value.GetString();
                    std::string camera_pose = itr->value.GetString();
                    std::vector<std::string> cam_pose = split_str(camera_pose, ";");
		            task->cameraPose.assign(cam_pose.begin(), cam_pose.end());
                }
                else if (key == "Id")
                {
                    LOG(WARNING) << "Id:" << itr->value.GetString();
                    task->transfer_id = itr->value.GetString();
                }
                else if (key == "TurnAngle")
                {
                    LOG(WARNING) << "TurnAngle:" << itr->value.GetString();
                    std::string turn_angle = itr->value.GetString();
                    task->fTurnAngle = atof(turn_angle.c_str());
                }
                else if (key == "TLine")
                {
                    LOG(WARNING) << "TLine:" << itr->value.GetString();
                    task->road = itr->value.GetString();
                }
                else if (key == "TLoc")
                {
                    LOG(WARNING) << "TLoc:" << itr->value.GetString();
                    std::string sloc = itr->value.GetString();
                    std::vector<std::string> veLoc = split_str(sloc, ";");
		            task->LocX = atof(veLoc[0].c_str());
		            task->LocY = atof(veLoc[1].c_str());
		            task->LocZ = atof(veLoc[2].c_str());
                }
                else if(key == "TLocType"){
                    std::string sType = itr->value.GetString();
                    if (!strcmp(sType.c_str(), "start")) //与transfer相同
			            task->nType = TASK_TYPE_START;
		            else if (!strcmp(sType.c_str(), "turn"))
			            task->nType = TASK_TYPE_TURN;
		            else if (!strcmp(sType.c_str(), "monitor"))
			            task->nType = TASK_TYPE_MONITOR;
		            else if (!strcmp(sType.c_str(), "transfer"))
			            task->nType = TASK_TYPE_TRANSFER;
		            else if (!strcmp(sType.c_str(), "back"))
			            task->nType = TASK_TYPE_BACK;
		            else if (!strcmp(sType.c_str(), "end"))
			            task->nType = TASK_TYPE_END;
		            else
			            task->nType = TASK_TYPE_UNKNOWN;
                }
            }
            //task_control::option_.coloring_tasks.push_back(task);
        }
        return tid.GetInt();
    }
};
#endif