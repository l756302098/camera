#include <ros/ros.h>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "tio_control/fsm.hpp"
#include "tio_control/action_mapping.hpp"
#include "tio_control/task_control.hpp"

namespace fsm
{
    void action_mapping::init(){
        LOG(INFO) << "action_mapping init " << name ;
        if(!is_init){
            is_init = true;
            LOG(INFO) << "action_mapping init ros client ";
            mapping_start_client = nh_.serviceClient<mapping_msg::start_mapping>("/tio/map/mapping_start");
            mapping_stop_client = nh_.serviceClient<mapping_msg::stop_mapping>("/tio/map/mapping_stop");
            mapping_quit_client = nh_.serviceClient<mapping_msg::quit_mapping>("/tio/map/mapping_quit");
            collect_data_client = nh_.serviceClient<mapping_msg::collect_data>("/tio/map/collect_map_data");
            ptz_client = nh_.serviceClient<fixed_msg::cp_control>("/fixed/platform/cmd");
            status_sub = nh_.subscribe("/tio/map/mapping_status", 1, &fsm::action_mapping::status_cb,this);
            process_sub = nh_.subscribe("/tio/map/mapping_progress", 1, &fsm::action_mapping::progress_cb,this);
            arrive_sub = nh_.subscribe("/fixed/platform/isreach", 1, &fsm::action_mapping::arrive_cb,this);
            collected_sub = nh_.subscribe("/tio/collected_frame_cnt", 1, &fsm::action_mapping::collect_cb,this);
        }
        last = ros::Time::now();
    }
    void action_mapping::run(){
        if(is_pause){
            if(ros::Time::now()-last>ros::Duration(1)){
                LOG(WARNING) << "action_mapping " << name << " pause" ;
            }
            return;
        }
        switch (current_status)
        {
            case fsm::mapping_enum::NONE:
                /* code */
                break;
            case fsm::mapping_enum::ERROR_FINISH:
                break;
            case fsm::mapping_enum::QUIT:
                break;
            case fsm::mapping_enum::START:
                current_status = fsm::mapping_enum::MOTOR;
                break;
            case fsm::mapping_enum::MOTOR:
                motor();
                break;
            case fsm::mapping_enum::COLLECT:
                collect_data();
                break;
            case fsm::mapping_enum::STOP:
                stop_mapping();
                break;
            case fsm::mapping_enum::WAITFOR_MOTOR:
                time_tick(task_control::option_.motor_timeout);
            case fsm::mapping_enum::WAITFOR_COLOR:
                time_tick(task_control::option_.mapping_color_timeout * (flag +1));
            case fsm::mapping_enum::WAITFOR_COLLECT:
                time_tick(task_control::option_.collect_timeout);
            default:
                break;
        }
        
    }
    void action_mapping::exit(){
        current_status = fsm::mapping_enum::NONE;
        LOG(INFO) << "action_mapping exit " << name;
    }

    bool action_mapping::play(uint8_t flag,std::vector<std::string> &data){
        LOG(INFO) << "action_mapping play " << std::to_string(flag);
        switch (flag)
        {
            case fsm::mapping_enum::START:
                return start_mapping(data);
            case fsm::mapping_enum::QUIT:
                return quit_mapping(data);
            case fsm::mapping_enum::PAUSE:
                is_pause = true;
                return true;
            case fsm::mapping_enum::GOON:
                is_pause = false;
                return true;;
            default:
                break;
        }
        return false;
    }

    void action_mapping::reset(){
        LOG(INFO) << "action_mapping reset";
        device_id = 0;
        task_id = 0;
        map_id = 0;
        flag = 0;
        collect_count = 0;
        mottor_timeout_count = 0;
        callback = NULL;
    }

    bool action_mapping::time_tick(int seconds){
        ros::Time now = ros::Time::now();
        if(now - start_wait_time > ros::Duration(seconds)){
            if(current_status == fsm::mapping_enum::WAITFOR_MOTOR){
                mottor_timeout_count++;
                if(mottor_timeout_count>3){
                    LOG(ERROR) << "motor: no response for three consecutive times";
                    std::string error_msg = "motor timeout ";
                    response(fsm::mapping_error_enum::MOTOR_TIMEOUT,error_msg);
                }else{
                    LOG(INFO) << "motor timeout, try again.";
                    motor();
                }
            }else if(current_status == fsm::mapping_enum::WAITFOR_COLOR){
                std::string error_msg = "color timeout ";
                response(fsm::mapping_error_enum::COLOR_TIMEOUT,error_msg);
            }else if(current_status == fsm::mapping_enum::WAITFOR_COLLECT){
                std::string error_msg = "collect timeout ";
                response(fsm::mapping_error_enum::COLLECT_TIMEOUT,error_msg);
            }
            return true;
        }
        return false;
    }

    void action_mapping::response(fsm::mapping_error_enum code,std::string &error_msg){
        //LOG(INFO) << "action_mapping::response code:" << code << " msg:" << error_msg; 
        if((int)code <=  (int)fsm::mapping_error_enum::MAP_QUIT){
            LOG(INFO) << "response status:" << status << " code:" << code << " ok";
        }else{
            current_status = fsm::mapping_enum::ERROR_FINISH;
            LOG(ERROR) << "response status:" << status << " code:" << code << " "<< error_msg;
        }
        if (callback){
            float percent = 1.0 * collect_count / (task_control::option_.motor_trajectory.size() +task_control::option_.color_weight);
            callback(status,code,error_msg, percent);
        }
    }

    void action_mapping::status_cb(const mapping_msg::mapping_status::ConstPtr &msg){
        LOG(INFO) << "action_mapping status_cb task_status:" << msg->task_status << " error_message:" << msg->error_message
        << " device_id:" << msg->device_id<< " task_id" <<msg->task_id << " map_id:" << msg->map_id;
        if(current_status == fsm::mapping_enum::WAITFOR_COLOR){
            if(msg->task_status == 1){
                LOG(INFO) << "action_mapping status_cb mapping finish";
                current_status = fsm::mapping_enum::NOR_FINISH;
                //notify control
                collect_count = task_control::option_.motor_trajectory.size() + task_control::option_.color_weight - 1;
                std::string error_msg = "mapping finish! ";
                response(fsm::mapping_error_enum::MAP_FINISH,error_msg);
                reset();
            }
        }
        if(current_status == fsm::mapping_enum::WAITFOR_COLLECT){
            if(msg->task_status == -1){
                LOG(INFO) << "action_mapping status_cb collect error:"<<msg->error_message;
                current_status = fsm::mapping_enum::NOR_FINISH;
                //notify control
                std::string error_msg = msg->error_message;
                response(fsm::mapping_error_enum::NO_DATA,error_msg);
            }
        }
    }
    void action_mapping::progress_cb(const mapping_msg::mapping_progress::ConstPtr &msg){
        LOG(INFO) << "action_mapping progress_cb "<< msg->progress << " device_id:" << msg->device_id
         << " task_id" <<msg->task_id << " map_id:" << msg->map_id;
    }

    bool action_mapping::start_mapping(std::vector<std::string> &data){
        LOG(INFO) << "start_mapping" ;
        reset();
        //resolve data
        if(data.size()<=0) return false;
        device_id = atoi(data[0].c_str());
        task_id = atoi(data[1].c_str());
        map_id = atoi(data[2].c_str());
        flag = atoi(data[3].c_str());
        mapping_msg::start_mapping cmd;
        cmd.request.device_id = device_id;
        cmd.request.task_id = task_id;
        cmd.request.map_id = map_id;
        cmd.request.resolution_flag = flag;
        if(mapping_start_client.call(cmd))
		{
            std::string error_msg = "mapping_start_client call success.";
            response(fsm::mapping_error_enum::NORMAL,error_msg);
            LOG(INFO) << error_msg;
            current_status = fsm::mapping_enum::START;
            collect_count = 0;
            return true;
        }
	    else{
            std::string error_msg = "mapping_start_client call failed ";
            LOG(ERROR) << error_msg;
            response(fsm::mapping_error_enum::MAPPING_NORESPNSE,error_msg);
            return false;
        }
    }
    bool action_mapping::stop_mapping(){
        LOG(INFO) << "stop_mapping" ;
        //mapping_start_client.call()
        mapping_msg::stop_mapping cmd;
        cmd.request.device_id = device_id;
        cmd.request.task_id = task_id;
        cmd.request.map_id = map_id;
        if(mapping_stop_client.call(cmd))
		{
            LOG(INFO) << "mapping_stop_client call success.";
            current_status = fsm::mapping_enum::WAITFOR_COLOR;
            start_wait_time = ros::Time::now();
            return true;
        }
	    else{
            std::string error_msg = "mapping_stop_client call failed ";
            response(fsm::mapping_error_enum::MAPPING_NORESPNSE,error_msg);
            return false;
        }
    }
    bool action_mapping::quit_mapping(std::vector<std::string> &data){
         LOG(INFO) << "quit_mapping" ;
        //resolve data
        mapping_msg::quit_mapping cmd;
        cmd.request.device_id = device_id;
        cmd.request.task_id = task_id;
        cmd.request.map_id = map_id;
        if(mapping_quit_client.call(cmd))
		{
            LOG(INFO) << "mapping_quit_client call success.";
            std::string error_msg = "mapping quit success.";
            response(fsm::mapping_error_enum::MAP_QUIT,error_msg);
            reset();
            return true;
        }
	    else{
            std::string error_msg = "mapping_quit_client call failed ";
            response(fsm::mapping_error_enum::MAPPING_NORESPNSE,error_msg);
            return false;
        }
    }
    bool action_mapping::collect_data(){
        LOG(INFO) << "collect_data" ;
        //resolve data
        mapping_msg::collect_data cmd;
        cmd.request.device_id = device_id;
        cmd.request.task_id = task_id;
        cmd.request.map_id = map_id;
        cmd.request.pitch_angle = motor_angle_z;
        cmd.request.roll_angle = motor_angle_x;
        if(collect_data_client.call(cmd))
		{
            LOG(INFO) << "collect_data_client call success.";
            current_status = fsm::mapping_enum::WAITFOR_COLLECT;
            start_wait_time = ros::Time::now();
            return true;
        }
	    else{
            std::string error_msg = "collect_data_client call failed ";
            response(fsm::mapping_error_enum::MAPPING_NORESPNSE,error_msg);
            return false;
        }
    }

    void action_mapping::arrive_cb(const std_msgs::Int32::ConstPtr &msg){
        LOG(INFO) << "arrive_cb" << msg->data ;
        mottor_timeout_count = 0;
        if(current_status == fsm::mapping_enum::WAITFOR_MOTOR){
            motor_angle_x = target_angle_x;
            motor_angle_z = target_angle_z;
            current_status = fsm::mapping_enum::COLLECT;
        }
    }

    void action_mapping::collect_cb(const std_msgs::Int32::ConstPtr &msg){
        LOG(INFO) << "collect_cb:" << msg->data ;
        if(current_status == fsm::mapping_enum::WAITFOR_COLLECT){
            if(msg->data == 0){
                LOG(INFO) << "collect failed try agaiin" ;
                collect_data();
            }else{
                if(collect_count < task_control::option_.motor_trajectory.size() - 1){
                    current_status = fsm::mapping_enum::MOTOR;
                    collect_count++;
                    LOG(INFO) << "collect_cb collect_count++ " << collect_count ;
                    std::string error_msg = "ok";
                    response(fsm::mapping_error_enum::NORMAL,error_msg);
                }else{
                    LOG(INFO) << "collect_cb collect finish";
                    current_status = fsm::mapping_enum::STOP;
                }
            }
        }
    }

    void action_mapping::motor(){
        if(task_control::option_.motor_trajectory.size() <= 0){
            LOG(ERROR) << "motor_trajectory size equal 0";
            return;
        }
        if(collect_count < task_control::option_.motor_trajectory.size()){
            std::shared_ptr<WatchNode> wn = task_control::option_.motor_trajectory[collect_count];
            if(wn){
                LOG(INFO) << "collect start control motor to target id:" << wn->id  << " x:" << wn->angle_x << " z:"<< wn->angle_z;
                fixed_msg::cp_control ptz_cmd;
                ptz_cmd.request.id = device_id;
                ptz_cmd.request.action = 1;
                ptz_cmd.request.type = 3;
                std::vector<int> quavalue;
                quavalue.push_back( wn->angle_x * 100);
                quavalue.push_back( wn->angle_z * 100);
                ptz_cmd.request.allvalue = quavalue;
                if(ptz_client.call(ptz_cmd)){
                    LOG(INFO) << "ptz_client call success";
                    target_angle_x = wn->angle_x;
                    target_angle_z = wn->angle_z;
                    current_status = fsm::mapping_enum::WAITFOR_MOTOR;
                    start_wait_time = ros::Time::now();
                }
                else{
                    std::string error_msg = "ptz_client call failed ";
                    response(fsm::mapping_error_enum::MOTOR_NORESPNSE,error_msg);
                }
            }
        }else{
            LOG(INFO) << "motor collect finish";
        }
    }
}