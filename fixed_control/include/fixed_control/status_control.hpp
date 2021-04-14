/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-22 13:21:17
 * @LastEditors: li
 * @LastEditTime: 2021-04-13 16:20:17
 */
#include <ros/ros.h>
#include "std_msgs/String.h"
#include "yidamsg/transfer.h"
#include "yidamsg/ControlMode.h"
#include "yidamsg/TaskList.h"
#include "yidamsg/TaskExecuteStatus.h"
#include <thread>
#include <fstream>
#include <iostream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/encodedstream.h>
#include <rapidjson/document.h>
using namespace rapidjson;
using namespace std;
#define TASK_CONTROL 0
#define HANDLE_CONTROL 1
#define PC_CONTROL 2
#define URGENCY_CONTROL 3
#define OBSTACLE_CONTROL 4

typedef struct _road_plan
{
	std::string road;
	float LocX;
	float LocY;
	float LocZ;
	int fLocWidth;
	int nType;
	float fTurnAngle;
    std::string transfer_id;
	bool ongoing;
	bool end;
	bool finish;
	int align;
	std::vector<std::string> cameraPose;
}ROAD_PLAN;

typedef enum __task_type__
{
	TASK_TYPE_START = 1,
	TASK_TYPE_TURN = 2,
	TASK_TYPE_END = 3,
	TASK_TYPE_MONITOR = 4,
	TASK_TYPE_TRANSFER = 5,
	TASK_TYPE_BACK=6,
	TASK_TYPE_UNKNOWN=0
}TASK_TYPE;

class status_control
{
private:
    /* data */
    int ctr_mode = 0;
    int robot_id,task_id;
    std::vector<ROAD_PLAN> road_tasks;
    bool clear_task_flag,watch_flag,task_running;
    ros::NodeHandle nh_;
    //ros::Subscriber meter_sub;
    ros::ServiceServer task_server;
    ros::Publisher transfer_pub,control_mode_pub,task_status_pub;
    std::vector<std::string> split_str(std::string str, std::string pattern){
	    std::string::size_type pos;
	    std::vector<std::string> result;
	    str += pattern;
	    int size = str.size();

	    for (int i = 0; i < size; i++)
	    {
		    pos = str.find(pattern, i);
		    if (pos < size)
		    {
			    std::string s = str.substr(i, pos - i);
			    result.push_back(s);
			    i = pos + pattern.size() - 1;
		    }
	    }
	    return result;
    }
public:
    //void load_from_stream(std::ifstream &in){
    void load_from_stream(std::string &in){
    road_tasks.clear();
    std::cout << "out:" << std::endl;
    std::cout << in << std::endl;
    const char* json = in.c_str();
    StringStream ss(json);
    // rapidjson::IStreamWrapper isw(in);
    // rapidjson::EncodedInputStream<rapidjson::UTF8<>, rapidjson::IStreamWrapper> eis(isw);
    rapidjson::Document doc;
    if (doc.ParseStream(ss).HasParseError())
    {
        ROS_ERROR("Failed to parse json ");
        return;
    }
    // if (doc.ParseStream(eis).HasParseError())
    // {
    //     ROS_ERROR("Failed to parse json ");
    //     return;
    // }
    if (!doc.HasMember("Tasks"))
    {
        ROS_WARN("No 'Tasks' field");
        return;
    }

    const rapidjson::Value &tasks = doc["Tasks"];
    if (!tasks.IsArray())
    {
        ROS_WARN("Invalid 'Tasks' field");
        return;
    }
    const rapidjson::Value &iid = doc["InspectId"];
    if(iid.IsString()){
        std::cout << "InspectId:" << iid.GetString() << std::endl;
        task_id = atoi(iid.GetString());
    }
    const rapidjson::Value &rid = doc["RobotId"];
    if(rid.IsNumber()){std::cout << "RobotId:" << rid.GetInt() << std::endl;}
    const rapidjson::Value &thid = doc["TaskHistoryId"];
    if(thid.IsNumber()){std::cout << "TaskHistoryId:" << thid.GetInt() << std::endl;}
    const rapidjson::Value &tid = doc["TaskId"];
    if(tid.IsNumber()){std::cout << "TaskId:" << tid.GetInt() << std::endl;}
    static const char* kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };
    for (SizeType i = 0; i < tasks.Size(); i++) // 使用 SizeType 而不是 size_t
    {
        ROAD_PLAN task;
        const rapidjson::Value &obj = tasks[i]; 
        for (Value::ConstMemberIterator itr = obj.MemberBegin();itr != obj.MemberEnd(); ++itr){
            std::string type = kTypeNames[itr->value.GetType()];
            std::string key = itr->name.GetString();
            //std::cout << "" << itr->name.GetString() << " " << type  << std::endl;
            if (key == "Align")
            {
               std::cout << "Align:" << itr->value.GetString() << std::endl;
               task.road = itr->value.GetString();
            }
            else if (key == "CameraPose")
            {
                std::cout << "CameraPose:" << itr->value.GetString() << std::endl;
                std::string camera_pose = itr->value.GetString();
                std::vector<std::string> cam_pose = split_str(camera_pose, ";");
		        task.cameraPose.assign(cam_pose.begin(), cam_pose.end());
            }
            else if (key == "Id")
            {
                std::cout << "Id:" << itr->value.GetString() << std::endl;
                task.transfer_id = itr->value.GetString();
            }
            else if (key == "TurnAngle")
            {
                std::cout << "TurnAngle:" << itr->value.GetString() << std::endl;
                std::string turn_angle = itr->value.GetString();
                task.fTurnAngle = atof(turn_angle.c_str());
            }
            else if (key == "TLine")
            {
                std::cout << "TLine:" << itr->value.GetString() << std::endl;
                task.road = itr->value.GetString();
            }
            else if (key == "TLoc")
            {
                std::cout << "TLoc:" << itr->value.GetString() << std::endl;
                std::string sloc = itr->value.GetString();
                std::vector<std::string> veLoc = split_str(sloc, ";");
		        task.LocX = atof(veLoc[0].c_str());
		        task.LocY = atof(veLoc[1].c_str());
		        task.LocZ = atof(veLoc[2].c_str());
            }else if(key == "TLocType"){
                std::string sType = itr->value.GetString();
                if (!strcmp(sType.c_str(), "start")) //与transfer相同
			        task.nType = TASK_TYPE_START;
		        else if (!strcmp(sType.c_str(), "turn"))
			        task.nType = TASK_TYPE_TURN;
		        else if (!strcmp(sType.c_str(), "monitor"))
			        task.nType = TASK_TYPE_MONITOR;
		        else if (!strcmp(sType.c_str(), "transfer"))
			        task.nType = TASK_TYPE_TRANSFER;
		        else if (!strcmp(sType.c_str(), "back"))
			        task.nType = TASK_TYPE_BACK;
		        else if (!strcmp(sType.c_str(), "end"))
			        task.nType = TASK_TYPE_END;
		        else
			        task.nType = TASK_TYPE_UNKNOWN;
            }
        }
        road_tasks.push_back(task);
    }
    taskStatusPub(task_id,3);
    }

    void load(std::string &path){
        std::ifstream in(path.c_str());
        if (!in)
        {
            ROS_WARN("Failed to open %s", path.c_str());
            return;
        }
        //load_from_stream(in);
    }
    void mode_cb(const yidamsg::ControlMode msg)
    {
	    if (msg.mode == HANDLE_CONTROL)
	    {
		    ROS_INFO("to handle control");
            ctr_mode = HANDLE_CONTROL;
	    }
	    else if (msg.mode == PC_CONTROL)
	    {
		    ROS_INFO("to pc control");
            ctr_mode = PC_CONTROL;
	    }
	    else if (msg.mode == TASK_CONTROL)
	    {
		    ROS_INFO("to task control");
            ctr_mode = TASK_CONTROL;
	    }
	    else if (msg.mode == URGENCY_CONTROL)
	    {
		    ROS_INFO("to urgency control");
            ctr_mode = TASK_CONTROL;
	    }
        if(msg.mode != TASK_CONTROL)
            clear_task_flag = true;
    }
    void transfer(vector<string> lists, vector<string> task_camera, string trans_id, char flg);
    status_control(const ros::NodeHandle &nh = ros::NodeHandle("~"));
    ~status_control();
    void meter_cb(const std_msgs::String msg);
    bool task_srv(yidamsg::TaskList::Request &req, yidamsg::TaskList::Response &res);
    void tick(const ros::TimerEvent &event);
    void update();
    void taskStatusPub(int task_id, int task_status);

};