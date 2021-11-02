#pragma once
#ifndef __STRING_EXTENSION_H__
#define __STRING_EXTENSION_H__

#include <iostream>
#include <vector>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/encodedstream.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

typedef struct _task_data
{
	std::string taskData;
	int type;
	std::string rule;
}TaskData;

typedef struct _map_json
{
	std::string InspectId;
	int InspectTypeId;
	int RobotId;
    int TaskId;
    int TaskHistoryId;
    int TaskType;
    int stationMapId;
    std::string stationMapVersion;
    std::string density;
}MapJson;

typedef struct _task_plan
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
}TaskPlan;

typedef struct _color_json
{
	std::string InspectId;
	int InspectTypeId;
	int RobotId;
    int TaskId;
    int TaskHistoryId;
    int TaskType;
    int stationMapId;
    std::string stationMapVersion;
    std::vector<std::shared_ptr<TaskPlan>> plans;
}ColorJson;

typedef struct _task_command
{
    int robotId;
    int taskHistoryId;
    int flag;
}TaskCommand;

static std::vector<std::string> split_str(std::string str, std::string pattern){
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

static std::string json2string(const rapidjson::Value& valObj){
    rapidjson::StringBuffer sbBuf;
    rapidjson::Writer<rapidjson::StringBuffer> jWriter(sbBuf);
    valObj.Accept(jWriter);
    return std::string(sbBuf.GetString());
}

static std::shared_ptr<MapJson> resolve_map_json(const char* in){
        LOG(INFO) << "resolve_map_json in:" << in;
        rapidjson::StringStream ss(in);
        rapidjson::Document doc;
        if (doc.ParseStream(ss).HasParseError())
        {
            LOG(ERROR) << "Failed to parse json";
            return nullptr;
        }
        if(!doc.IsObject()){
            LOG(WARNING) <<"json is not object";
            return nullptr;
        }
        std::shared_ptr<MapJson> json(new MapJson());
        const rapidjson::Value &iid = doc["InspectId"];
        if(iid.IsString()){LOG(WARNING) << "InspectId:" << iid.GetString();json->InspectId = iid.GetString();}
        const rapidjson::Value &iti = doc["InspectTypeId"];
        if(iti.IsNumber()){LOG(WARNING) << "InspectTypeId:" << iti.GetInt();json->InspectTypeId = iti.GetInt();}
        const rapidjson::Value &rid = doc["RobotId"];
        if(rid.IsNumber()){LOG(WARNING) << "RobotId:" << rid.GetInt();json->RobotId = rid.GetInt();}
        const rapidjson::Value &thid = doc["TaskHistoryId"];
        if(thid.IsNumber()){LOG(WARNING) << "TaskHistoryId:" << thid.GetInt();json->TaskHistoryId = thid.GetInt();}
        const rapidjson::Value &tid = doc["TaskId"];
        if(tid.IsNumber()){LOG(WARNING) << "TaskId:" << tid.GetInt();json->TaskId = tid.GetInt();}
        const rapidjson::Value &tt = doc["TaskType"];
        if(tt.IsNumber()){LOG(WARNING) << "TaskType:" << tt.GetInt();json->TaskType = tt.GetInt();}
        const rapidjson::Value &smi = doc["stationMapId"];
        if(smi.IsNumber()){LOG(WARNING) << "stationMapId:" << smi.GetInt();json->stationMapId = smi.GetInt();}
        const rapidjson::Value &smv = doc["stationMapVersion"];
        if(smv.IsString()){LOG(WARNING) << "stationMapVersion:" << smv.GetString();json->stationMapVersion = smv.GetString();}
        const rapidjson::Value &density = doc["density"];
        if(density.IsString()){LOG(WARNING) << "density:" << density.GetString();json->density = density.GetString();}
        return json;
}

static std::shared_ptr<ColorJson> resolve_color_json(const char* in){
        rapidjson::StringStream ss(in);
        rapidjson::Document doc;
        if (doc.ParseStream(ss).HasParseError())
        {
            LOG(ERROR) << "Failed to parse json";
            return nullptr;
        }
        if(!doc.IsObject()){
            LOG(WARNING) <<"json is not object";
            return nullptr;
        }
        if (!doc.HasMember("Tasks"))
        {
            LOG(WARNING) <<"No 'Tasks' field";
            return nullptr;
        }

        const rapidjson::Value &tasks = doc["Tasks"];
        if (!tasks.IsArray())
        {
            LOG(WARNING) <<"Invalid 'Tasks' field";
            return nullptr;
        }
        
        std::shared_ptr<ColorJson> json(new ColorJson());
        const rapidjson::Value &iid = doc["InspectId"];
        if(iid.IsString()){LOG(WARNING) << "InspectId:" << iid.GetString();json->InspectId = iid.GetString();}
        const rapidjson::Value &iti = doc["InspectTypeId"];
        if(iti.IsNumber()){LOG(WARNING) << "InspectTypeId:" << iti.GetInt();json->InspectTypeId = iti.GetInt();}
        const rapidjson::Value &rid = doc["RobotId"];
        if(rid.IsNumber()){LOG(WARNING) << "RobotId:" << rid.GetInt();json->RobotId = rid.GetInt();}
        const rapidjson::Value &thid = doc["TaskHistoryId"];
        if(thid.IsNumber()){LOG(WARNING) << "TaskHistoryId:" << thid.GetInt();json->TaskHistoryId = thid.GetInt();}
        const rapidjson::Value &tid = doc["TaskId"];
        if(tid.IsNumber()){LOG(WARNING) << "TaskId:" << tid.GetInt();json->TaskId = tid.GetInt();}
        const rapidjson::Value &tt = doc["TaskType"];
        if(tt.IsNumber()){LOG(WARNING) << "TaskType:" << tt.GetInt();json->TaskType = tt.GetInt();}
        const rapidjson::Value &smi = doc["stationMapId"];
        if(smi.IsNumber()){LOG(WARNING) << "stationMapId:" << smi.GetInt();json->stationMapId = smi.GetInt();}
        const rapidjson::Value &smv = doc["stationMapVersion"];
        if(smv.IsString()){LOG(WARNING) << "stationMapVersion:" << smv.GetString();json->stationMapVersion = smv.GetString();}
        static const char* kTypeNames[] = { "Null", "False", "True", "Object", "Array", "String", "Number" };
        for (rapidjson::SizeType i = 0; i < tasks.Size(); i++) // 使用 SizeType 而不是 size_t
        {
            std::shared_ptr<TaskPlan> task(new TaskPlan());
            const rapidjson::Value &obj = tasks[i]; 
            for (rapidjson::Value::ConstMemberIterator itr = obj.MemberBegin();itr != obj.MemberEnd(); ++itr){
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
                    /*
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
                        */
                }
            }
            json->plans.push_back(task);
        }
        return json;
}

static std::shared_ptr<TaskData> resolve_task_data(const char* in){
        rapidjson::StringStream ss(in);
        rapidjson::Document doc;
        if (doc.ParseStream(ss).HasParseError())
        {
            LOG(ERROR) << "Failed to parse json";
            return nullptr;
        }
        if(!doc.IsObject()){
            LOG(WARNING) <<"json is not object";
            return nullptr;
        }
        if (!doc.HasMember("data"))
        {
            LOG(WARNING) <<"No 'data' field";
            return nullptr;
        }
        const rapidjson::Value &data = doc["data"];
        if (!data.IsObject())
        {
            LOG(WARNING) <<"Invalid 'data' field";
            return nullptr;
        }
        std::shared_ptr<TaskData> json(new TaskData());
        const rapidjson::Value &td = data["taskData"];
        if(td.IsObject()){json->taskData = json2string(td); }
        const rapidjson::Value &type = data["type"];
        if(type.IsNumber()){LOG(WARNING) << "type:" << type.GetInt();json->type = type.GetInt();}
        const rapidjson::Value &rule = data["rule"];
        if(rule.IsObject()){json->rule = json2string(rule); }
        return json;
}

static std::shared_ptr<TaskCommand> resolve_task_command(const char* in){
        rapidjson::StringStream ss(in);
        rapidjson::Document doc;
        if (doc.ParseStream(ss).HasParseError())
        {
            LOG(ERROR) << "Failed to parse json";
            return nullptr;
        }
        if(!doc.IsObject()){
            LOG(WARNING) <<"json is not object";
            return nullptr;
        }
        if (!doc.HasMember("data"))
        {
            LOG(WARNING) <<"No 'data' field";
            return nullptr;
        }
        const rapidjson::Value &data = doc["data"];
        if (!data.IsObject())
        {
            LOG(WARNING) <<"Invalid 'data' field";
            return nullptr;
        }
        std::shared_ptr<TaskCommand> json(new TaskCommand());
        const rapidjson::Value &td = data["taskData"];
        const rapidjson::Value &robotId = data["robotId"];
        if(robotId.IsNumber()){LOG(WARNING) << "robotId:" << robotId.GetInt();json->robotId = robotId.GetInt();}
        const rapidjson::Value &taskHistoryId = data["taskHistoryId"];
        if(taskHistoryId.IsNumber()){LOG(WARNING) << "taskHistoryId:" << taskHistoryId.GetInt();json->taskHistoryId = taskHistoryId.GetInt();}
        const rapidjson::Value &flag = data["flag"];
        if(flag.IsNumber()){LOG(WARNING) << "flag:" << flag.GetInt();json->flag = flag.GetInt();}
        return json;
}

#endif