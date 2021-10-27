#include <ros/ros.h>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "tio_control/fsm.hpp"
#include "tio_control/action_coloring.hpp"

namespace fsm
{
    void action_coloring::init(){
        LOG(INFO) << "action_coloring init " << name ;
        if(!is_init){
            is_init = true;
            LOG(INFO) << "action_coloring init ros client ";
            tf_pose_start_client = nh_.serviceClient<tf_ircampose_msg::start_pose>("/tio/inspect/ircam_pose_start");
            tf_pose_stop_client = nh_.serviceClient<tf_ircampose_msg::stop_pose>("/tio/inspect/ircam_pose_stop");
            undistort_start_client = nh_.serviceClient<undistort_service_msgs::start_undistort>("/undistort_service/start_undistort");
            undistort_stop_client = nh_.serviceClient<undistort_service_msgs::stop_undistort>("/undistort_service/stop_undistort");
            ptz_client = nh_.serviceClient<fixed_msg::cp_control>("/fixed/platform/cmd");
            arrive_sub = nh_.subscribe("/fixed/platform/isreach", 1, &fsm::action_coloring::arrive_cb,this);
            status_sub = nh_.subscribe("/tio/irpose/pose_status", 1, &fsm::action_coloring::status_cb,this);
            //coloring_finish_sub = nh_.subscribe(task_control::option_.coloring_finish_topic, 1, &fsm::action_coloring::task_finish_cb,this);
            coloring_undistort_sub = nh_.subscribe("/infrared/undistorted", 1, &fsm::action_coloring::undistort_cb,this);
            coloring_start_pub = nh_.advertise<fixed_msg::inspected_result>(task_control::option_.coloring_start_topic, 1);
        }
        last = ros::Time::now();
    }
    void action_coloring::run(){
       if(is_pause){
            if(ros::Time::now()-last>ros::Duration(1)){
                LOG(WARNING) << "action_coloring " << name << " pause" ;
            }
            return;
        }
        switch (current_status)
        {
            case fsm::coloring_enum::COR_NONE:
                /* code */
                break;
            case fsm::coloring_enum::COR_ERROR_FINISH:
                /* code */
                break;
            case fsm::coloring_enum::COR_QUIT:
                /* code */
                break;
            case fsm::coloring_enum::COR_MOTOR:
                motor();
                break;
            case fsm::coloring_enum::COR_START_POSE:
                start_pose();
                break;
            case fsm::coloring_enum::COR_START_UNDISTORT:
                start_undistort();
                break;
            case fsm::coloring_enum::COR_WAITFOR_MOTOR:
                time_tick(task_control::option_.motor_timeout);
            case fsm::coloring_enum::COR_WAIT_COLOR:
                time_tick(task_control::option_.coloring_timeout);
                break;
            default:
                break;
        }
    }
    void action_coloring::exit(){
        std::cout << "action_coloring " << name << " exit" << std::endl;
    }
    bool action_coloring::play(uint8_t flag,std::vector<std::string> &data){
        LOG(INFO) << "action_coloring play " << std::to_string(flag);
        switch (flag)
        {
            case fsm::coloring_enum::COR_START:
                return start_coloring(data);
            case fsm::coloring_enum::COR_QUIT:
                return quit_coloring();
            case fsm::coloring_enum::COR_PAUSE:
                is_pause = true;
                return true;
            case fsm::coloring_enum::COR_GOON:
                is_pause = false;
                return true;;
            default:
                break;
        }
        return false;
    }
    bool action_coloring::time_tick(int seconds){
        ros::Time now = ros::Time::now();
        if(now - start_wait_time > ros::Duration(seconds)){
            if(current_status == fsm::coloring_enum::COR_WAITFOR_MOTOR){
                mottor_timeout_count++;
                if(mottor_timeout_count>3){
                    std::string error_msg = "motor no response for three consecutive times,go next watch points ";
                    LOG(ERROR) << error_msg;
                    response(fsm::coloring_error_enum::COR_MOTOR_NORESPNSE,error_msg);
                }else{
                    LOG(INFO) << "motor timeout, try again.";
                    motor();
                }
            }
            else if(current_status == fsm::coloring_enum::COR_WAIT_COLOR){
                LOG(ERROR) << "color timeout,go next watch points";
                go_next();
            }
            return true;
        }
        return false;
    }
    void action_coloring::arrive_cb(const std_msgs::Int32::ConstPtr &msg){
        LOG(INFO) << "action_coloring::arrive_cb" << msg->data ;
        mottor_timeout_count = 0;
        if(current_status == fsm::coloring_enum::COR_WAITFOR_MOTOR){
            move_level2++;
            motor_angle_x = target_angle_x;
            motor_angle_z = target_angle_z;
            current_status = fsm::coloring_enum::COR_START_POSE;
        }
    }

    bool action_coloring::start_coloring(std::vector<std::string> &data){
        LOG(INFO) << "action_coloring::start_coloring" ;
        reset();
        current_status = fsm::coloring_enum::COR_MOTOR;
        return true;
    }
    bool action_coloring::quit_coloring(){
        LOG(INFO) << "action_coloring::quit_coloring" ;
        bool is_stop_undistort = stop_undistort();
        if(!is_stop_undistort){
            LOG(ERROR) << "go_next:stop undistort failed";
            return false;
        }
        bool is_stop_pose = stop_pose();
        if(!is_stop_pose){
            LOG(ERROR) << "go_next:stop pose failed";
            return false;
        }
        std::string error_msg = "mapping quit success.";
        response(fsm::coloring_error_enum::COR_EXIT,error_msg);
        reset();
        return true;
    }
    void action_coloring::reset(){
        coloring_pub_info.clear();
        undistort_count = 0;
        device_id = 0;
        task_id = 0;
        map_id = 0;
        move_level1 = 0;
        move_level2 = 0;
        callback = NULL;
    }
    void action_coloring::motor(){
        if(task_control::option_.coloring_tasks.size() <= 0){
            LOG(ERROR) << "coloring_tasks size equal 0";
            return;
        }
        if(move_level1 < task_control::option_.coloring_tasks.size()){
            LOG(INFO) << "move_level1:" << move_level1 << " move_level2:" << move_level2;
            std::shared_ptr<ROAD_PLAN> rp = task_control::option_.coloring_tasks[move_level1];
            int watch_point_count = rp->cameraPose.size();
            if(move_level2 >= watch_point_count){
                move_level1 ++;
                move_level2 = 0;
                current_status = fsm::coloring_enum::COR_MOTOR;
                LOG(INFO) << "next move_level1:" << move_level1 << " move_level2:" << move_level2;
                return;
            }
            if(rp){
                try
                {        
                    std::vector<std::string> watch_point_info = split_str(rp->cameraPose[move_level2], "/");
			        std::vector<std::string> task_info = split_str(watch_point_info[0], ":");
                    int task_id = atoi(task_info[0].c_str());
			        int watch_point_type = atoi(task_info[1].c_str());
                    std::vector<std::string> camera_pos_info = split_str(watch_point_info[1], ",");
                    float camera_pose_x = atof(camera_pos_info[0].c_str());
                    float camera_pose_y = atof(camera_pos_info[1].c_str());
                    float camera_pose_z = atof(camera_pos_info[2].c_str());
                    LOG(INFO) <<"task id:" << task_id << "watch_point_type" << watch_point_type  << " transfer_id:" << rp->transfer_id
                            << " x:" << camera_pose_x << " y:" << camera_pose_y << " z:" << camera_pose_z
                            << watch_point_info[2] << watch_point_info[3] << watch_point_info[4];
                    coloring_pub_info = watch_point_info[0] + "/" + camera_pos_info[0] +"/"+ camera_pos_info[1] +"/"+ camera_pos_info[2] +"/";
                    coloring_pub_info = coloring_pub_info + watch_point_info[2] +"/" + watch_point_info[3] +"/" + watch_point_info[4];
                    if(watch_point_type == 7){
                        fixed_msg::cp_control ptz_cmd;
                        ptz_cmd.request.id = 1;
                        ptz_cmd.request.action = 1;
                        ptz_cmd.request.type = 3;
                        std::vector<int> quavalue;
                        quavalue.push_back( camera_pose_x * 100);
                        quavalue.push_back( camera_pose_y * 100);
                        ptz_cmd.request.allvalue = quavalue;
                        if(ptz_client.call(ptz_cmd)){
                            LOG(INFO) << "ptz_client call success";
                            target_angle_x = camera_pose_x;
                            target_angle_z = camera_pose_y;
                            current_status = fsm::coloring_enum::COR_WAITFOR_MOTOR;
                            start_wait_time = ros::Time::now();
                        }
                        else{
                            std::string error_msg = "ptz_client call failed ";
                            response(fsm::coloring_error_enum::COR_MOTOR_NORESPNSE,error_msg);
                        }
                    }else{
                        std::string error_msg = "tio node cannot deal watch point type " +  std::to_string(watch_point_type);
                        response(fsm::coloring_error_enum::COR_DATA_EXP,error_msg);
                    }
                }
                catch(const std::exception& e)
                {
                    LOG(ERROR) << e.what() ;
                    std::string error_msg = "data resolve exception ";
                    response(fsm::coloring_error_enum::COR_DATA_EXP,error_msg);
                }
            }
        }else{
            current_status = fsm::coloring_enum::COR_NOR_FINISH;
            std::string error_msg = "coloring motor finish ";
            response(fsm::coloring_error_enum::COR_FINISH,error_msg);
        }
    }

    void action_coloring::response(fsm::coloring_error_enum code,std::string &error_msg){
        //LOG(INFO) << "action_coloring::response code:" << code << " msg:" << error_msg; 
        if((int)code <=  (int)fsm::coloring_error_enum::COR_EXIT){
            LOG(INFO) << "response status:" << status << " code:" << code << " ok";
        }else{
            current_status = fsm::coloring_enum::COR_ERROR_FINISH;
            LOG(ERROR) << "response status:" << status << " code:" << code << " "<< error_msg;
        }
        if (callback){
            float percent = 1.0 * (move_level1 + 1) / task_control::option_.coloring_tasks.size();
            callback(status,code,error_msg, percent);
        }
    }

    void action_coloring::start_pose(){
        //start pose
        LOG(INFO) << "action_coloring::start_pose" ;
        tf_ircampose_msg::start_pose cmd;
        cmd.request.device_id = device_id;
        cmd.request.task_id = task_id;
        cmd.request.map_id = map_id;
        cmd.request.pitch_angle = motor_angle_z;
        cmd.request.roll_angle = motor_angle_x;
        if(tf_pose_start_client.call(cmd))
		{
            LOG(INFO) << "tf_pose_start_client call success.";
            current_status = fsm::coloring_enum::COR_START_UNDISTORT;
        }
	    else{
            std::string error_msg = "tf_pose_start_client call failed ";
            response(fsm::coloring_error_enum::COR_POSE_NORESPNSE,error_msg);
        }
    }
    bool action_coloring::stop_pose(){
        //stop pose
        LOG(INFO) << "action_coloring::stop_pose" ;
        tf_ircampose_msg::stop_pose cmd;
        cmd.request.device_id = device_id;
        cmd.request.task_id = task_id;
        cmd.request.map_id = map_id;
        if(tf_pose_stop_client.call(cmd))
		{
            LOG(INFO) << "tf_pose_stop_client call success.";
            return true;
        }
	    else{
            std::string error_msg = "tf_pose_stop_client call failed ";
            response(fsm::coloring_error_enum::COR_POSE_NORESPNSE,error_msg);
        }
        return false;
    }
    void action_coloring::start_undistort(){
        //start undistort
        LOG(INFO) << "action_coloring::start_undistort" ;
        undistort_service_msgs::start_undistort cmd;
        if(undistort_start_client.call(cmd))
		{
            if(cmd.response.status.code == 0){
                LOG(INFO) << "undistort_start_client call success. coloring_start_pub " << coloring_pub_info;
                undistort_count = 0;
                fixed_msg::inspected_result inspected_msg;
                inspected_msg.camid = device_id;
                inspected_msg.equipid = coloring_pub_info;
                inspected_msg.success = true;
                coloring_start_pub.publish(inspected_msg);
                current_status = fsm::coloring_enum::COR_WAIT_COLOR;
                start_wait_time = ros::Time::now();
            }else{
                response(fsm::coloring_error_enum::COR_UNDISTORT_NORESPNSE,cmd.response.status.message);
            }
        }
	    else{
            std::string error_msg = "undistort_start_client call failed ";
            response(fsm::coloring_error_enum::COR_UNDISTORT_NORESPNSE,error_msg);
        }
    }
    bool action_coloring::stop_undistort(){
        //stop_undistort
        LOG(INFO) << "action_coloring::stop_undistort" ;
        undistort_service_msgs::stop_undistort cmd;
        if(undistort_stop_client.call(cmd))
		{
            if(cmd.response.status.code == 0){
                LOG(INFO) << "undistort_stop_client call success.";
                return true;
            }else{
                response(fsm::coloring_error_enum::COR_UNDISTORT_NORESPNSE,cmd.response.status.message);
            }
        }
	    else{
            std::string error_msg = "undistort_stop_client call failed ";
            response(fsm::coloring_error_enum::COR_UNDISTORT_NORESPNSE,error_msg);
        }
        return false;
    }
    void action_coloring::task_finish_cb(const std_msgs::String msg){
        LOG(INFO) << "action_coloring::task_finish_cb";
        if(current_status == fsm::coloring_enum::COR_WAIT_COLOR){
            std::string error_msg = "ok";
            response(fsm::coloring_error_enum::COR_NORMAL,error_msg);
            go_next();
        }
    }

    void action_coloring::undistort_cb(const undistort_service_msgs::PosedImage msg){
        LOG(INFO) << "action_coloring::undistort_cb " << undistort_count;
        undistort_count++;
        if(undistort_count>5){
            if(current_status == fsm::coloring_enum::COR_WAIT_COLOR){
                std::string error_msg = "ok";
                response(fsm::coloring_error_enum::COR_NORMAL,error_msg);
                go_next();
            }
        }
    }

    void action_coloring::status_cb(const tf_ircampose_msg::tf_pose_status::ConstPtr &msg){
        LOG(INFO) << "action_coloring status_cb task_status:" << msg->task_status << " error_message:" << msg->error_message
        << " device_id:" << msg->device_id<< " task_id" <<msg->task_id << " map_id:" << msg->map_id;
        if(current_status == fsm::coloring_enum::COR_WAIT_COLOR){
            if(msg->task_status == -1){
                LOG(ERROR) << "action_coloring status_cb tf_pose error:"<<msg->error_message;
                current_status = fsm::coloring_enum::COR_ERROR_FINISH;
                //notify control
                std::string error_msg = msg->error_message;
                response(fsm::coloring_error_enum::COR_DATA_EXP,error_msg);
            }
        }
    }

    void action_coloring::go_next(){
        LOG(INFO) << "action_coloring::go_next" ;
        bool is_stop_undistort = stop_undistort();
        if(!is_stop_undistort){
            LOG(ERROR) << "go_next:stop undistort failed";
            return;
        }
        bool is_stop_pose = stop_pose();
        if(!is_stop_pose){
            LOG(ERROR) << "go_next:stop pose failed";
            return;
        }
        //calc next point
        current_status = fsm::coloring_enum::COR_MOTOR;
    }
}