/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-22 13:21:17
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 14:43:09
 */
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

typedef struct _watch_node
{
	int id;
	float angle_x;
	float angle_z;
}WatchNode;

typedef struct _node_option
{
	std::string log_path;
	std::string mapping_file;
	int log_max_days;
}NodeOption;