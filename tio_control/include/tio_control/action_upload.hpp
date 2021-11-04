/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-22 13:21:17
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 14:43:09
 */
#ifndef __ACTION_UPLOAD_H__
#define __ACTION_UPLOAD_H__

#include <ros/ros.h>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "tio_control/fsm.hpp"
#include "tio_control/task_control.hpp"
#include "mapping_msg/start_mapping.h"
#include "mapping_msg/stop_mapping.h"
#include "mapping_msg/quit_mapping.h"
#include "mapping_msg/collect_data.h"
#include "mapping_msg/mapping_progress.h"
#include "mapping_msg/mapping_status.h"
#include "tio_ftp/upload.h"
#include "tio_ftp/status.h"
#include <std_msgs/Int32.h>

namespace fsm
{
    enum class upload_enum{NONE = 0, START, STOP, WAITFOR_UPLOAD, NOR_FINISH ,ERROR_FINISH, QUIT,PAUSE,GOON};
    enum class upload_error_enum{NORMAL = 0 ,FINISH ,UPLOAD_TIMEOUT, FTP_NORESPNSE, FTP_FAILED,QUIT };

    class action_upload: public state_base
    {
    private:
        ros::NodeHandle nh_;
        ros::ServiceClient ftp_client;
        ros::Subscriber status_sub;
        bool is_init,is_pause;
        ros::Time last,start_wait_time;
        int device_id,task_id,map_id, flag = 0;
        float percent;
    private:
        bool start_upload(std::vector<std::string> &data);
        bool stop_upload();
        bool time_tick(int seconds);
        void response(fsm::upload_error_enum code,std::string &error_msg);
        void reset();
        void status_cb(const tio_ftp::status::ConstPtr &msg);
    public:
        upload_enum current_status;
    public:
        action_upload():is_pause(false),is_init(false){status = fsm::fsm_enum::UPLOAD;}
        void init();
        void run();
        void exit();
        bool play(uint8_t flag,std::vector<std::string> &data);
    };
}
#endif