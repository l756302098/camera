#include <ros/ros.h>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "tio_control/fsm.hpp"
#include "tio_control/action_upload.hpp"

namespace fsm
{
    void action_upload::init(){
        LOG(INFO) << "action_upload init " << name ;
        if(!is_init){
            is_init = true;
            LOG(INFO) << "action_upload init ros client ";
            ftp_client = nh_.serviceClient<tio_ftp::upload>("/tio/ftp/upload");
            status_sub = nh_.subscribe("/tio/ftp/status", 1, &fsm::action_upload::status_cb,this);
        }
        last = ros::Time::now();
    }
    void action_upload::run(){
        if(is_pause){
            if(ros::Time::now()-last>ros::Duration(1)){
                LOG(WARNING) << "action_upload " << name << " pause" ;
            }
            return;
        }
        switch ((int)current_status)
        {
            case (int)fsm::upload_enum::WAITFOR_UPLOAD:
                time_tick(task_control::option_.upload_timeout);
            default:
                break;
        }
    }
    void action_upload::exit(){
        LOG(INFO) << "action_upload exit " << name;
        current_status = fsm::upload_enum::NONE;
    }
    bool action_upload::start_upload(std::vector<std::string> &data){
        LOG(INFO) << "action_upload::start_upload ";
        reset();
        //resolve data
        if(data.size()<=0) return false;
        task_id = atoi(data[0].c_str());
        map_id = atoi(data[1].c_str());
        std::string target_name = data[2];
        std::string src_name = data[3];
        tio_ftp::upload cmd;
        LOG(INFO) << task_id << " " << map_id << " " << src_name << " " << target_name;
        cmd.request.flag = 1;
        cmd.request.device_id = 0;
        cmd.request.task_id = task_id;
        cmd.request.map_id = map_id;
        cmd.request.target_name = target_name;
        cmd.request.src_name = src_name;
        if(ftp_client.call(cmd)){
            start_wait_time = ros::Time::now();
            current_status = fsm::upload_enum::WAITFOR_UPLOAD;
            std::string error_msg = "ftp_client call success.";
            response(fsm::upload_error_enum::NORMAL,error_msg);
            return true;
        }else{
            LOG(ERROR) << "ftp_client call failed.";
        }
        return false;
    }
    bool action_upload::stop_upload(){
        tio_ftp::upload cmd;
        cmd.request.flag = 2;
        cmd.request.device_id = 0;
        if(ftp_client.call(cmd)){
            std::string error_msg = "ftp_client call success.";
            response(fsm::upload_error_enum::QUIT,error_msg);
            current_status = fsm::upload_enum::START;
            return true;
        }else{
            LOG(ERROR) << "ftp_client call failed.";
        }
        return false;
    }
    bool action_upload::time_tick(int seconds){
        ros::Time now = ros::Time::now();
        if(now - start_wait_time > ros::Duration(seconds)){
            if(current_status == fsm::upload_enum::WAITFOR_UPLOAD){
                current_status = fsm::upload_enum::ERROR_FINISH;
                std::string error_msg = "upload timeout ";
                response(fsm::upload_error_enum::UPLOAD_TIMEOUT,error_msg);
            }
            return true;
        }
        return false;
    }
    void action_upload::response(fsm::upload_error_enum code,std::string &error_msg){
        if((int)code <=  (int)fsm::upload_error_enum::UPLOAD_TIMEOUT){
            LOG(INFO) << "response status:" << status << " code:" << (int)code << " ok";
        }else{
            current_status = fsm::upload_enum::ERROR_FINISH;
            LOG(ERROR) << "response status:" << status << " code:" << (int)code << " "<< error_msg;
            percent = 0;
        }
        if (callback){
            callback(status,(int)code,error_msg, percent);
        }
    }
    void action_upload::reset(){
        task_id = 0;
        percent = 0;
    }
    bool action_upload::play(uint8_t flag,std::vector<std::string> &data){
        LOG(INFO) << "action_upload play " << std::to_string(flag);
        switch (flag)
        {
            case (int)fsm::upload_enum::START:
                return start_upload(data);
            case (int)fsm::upload_enum::QUIT:
                return stop_upload();
            case (int)fsm::upload_enum::PAUSE:
                is_pause = true;
                return true;
            case (int)fsm::upload_enum::GOON:
                is_pause = false;
                return true;;
            default:
                break;
        }
        return false;
    }
    void action_upload::status_cb(const tio_ftp::status::ConstPtr &msg){
        if(current_status == fsm::upload_enum::WAITFOR_UPLOAD ){
            if(msg->status == 0){
                percent = msg->progress;
                std::string error_msg = "ok";
                response(fsm::upload_error_enum::NORMAL,error_msg);
            }
            if(msg->status == 1){
                current_status = fsm::upload_enum::NOR_FINISH;
                reset();
                std::string error_msg = "upload finish! ";
                response(fsm::upload_error_enum::FINISH,error_msg);
            }else if(msg->status == 2){
                current_status = fsm::upload_enum::ERROR_FINISH;
                reset();
                std::string error_msg = "upload failed! ";
                response(fsm::upload_error_enum::FTP_FAILED,error_msg);
            }
        }
    }
}