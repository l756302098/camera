/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-06 13:37:38
 * @LastEditors: li
 * @LastEditTime: 2021-05-13 13:32:07
 */
#include "fixed_infrared/infrared_slide_control.hpp"

bool infrared_slide_control::do_task = false;

infrared_slide_control::infrared_slide_control(const ros::NodeHandle &nh):nh_(nh),is_reset(false){
    std::string camera_focus_value_str,camera_focus_mm_str;
    std::string camera_file = nh_.param<std::string>("camera_file", "");
    nh_.param<int>("camera_id", camera_id, 1);
    nh_.param<int>("cali_offset_h", cali_offset_h, 0);
    nh_.param<int>("cali_offset_v", cali_offset_v, 0);
    
    meterflag_pub = nh_.advertise<std_msgs::String>("/meter_flag", 1);
    infrared_result_pub = nh_.advertise<fixed_msg::inspected_result>("/infrared_survey_parm", 1);
    transfer_sub = nh_.subscribe("/fixed/platform/transfer", 1, &infrared_slide_control::transfer_callback, this);
    ptz_sub = nh_.subscribe("/fixed/platform/position", 1, &infrared_slide_control::ptz_callback, this);
    camera_pose_pub = nh_.advertise<geometry_msgs::PoseStamped>("/camera_pose", 1);
    connect_server();
}

infrared_slide_control::~infrared_slide_control(){}

void infrared_slide_control::update(){
    if(infrared_slide_control::do_task){
        if(is_reset){
            std_msgs::String msg;
            msg.data = "reset success";
            meterflag_pub.publish(msg);
        }else{
            //publish
            sleep(1);
            fixed_msg::inspected_result inspected_msg;
            inspected_msg.camid = camera_id;
            std::stringstream ss;
            ss.str("");
            ss << msg_list[3] << ":" << msg_list[4] << "/" << msg_list[5] << "/" << msg_list[6] << "/" << msg_list[7] << "/" << msg_list[8];
            inspected_msg.equipid = ss.str();
            inspected_msg.success = true;
            infrared_result_pub.publish(inspected_msg);
            std::cout << "publish infrared_survey_parm" << std::endl;
        }
        infrared_slide_control::do_task = false;
    }
}

void infrared_slide_control::connect_server()
{
    client = new Client("/slide/device/position", true);
    // 等待服务器端
    ROS_INFO("Waiting for action server to start.");
    client->waitForServer();
    ROS_INFO("Action server started, sending goal.");
}

void infrared_slide_control::transfer_callback(const fixed_msg::platform_transfer& msg){
    std::cout << "transfer_callback" << std::endl;
    if(msg.flag == 0){
        is_reset = false;
        std::string str_devicepoint = msg.data;
        msg_list.clear();
    	SplitString(str_devicepoint, msg_list, "/");
        for (size_t i = 0; i < msg_list.size(); i++)
        {
            std::cout << i << " " << msg_list[i] << std::endl;
        }
        
        std::cout << "SplitString size:" << msg_list.size() << std::endl;
        if(msg_list.size()<5) return;
        vector<std::string> local_point;
        SplitString(msg_list[2], local_point, ",");
        std::cout << "x:" << local_point[0]<< " y:" << local_point[1]<< " z:" << local_point[2] << std::endl;
        slide_control_msgs::PositionGoal base_goal;
        base_goal.action = 2;
        base_goal.goal.position.x = atof(local_point[0].c_str());
        base_goal.goal.position.y = atof(local_point[1].c_str());
        base_goal.goal.position.z = atof(local_point[2].c_str());

        client->sendGoal(base_goal, &infrared_slide_control::doneCb, &infrared_slide_control::activeCb, &infrared_slide_control::feedbackCb);
    }else if(msg.flag == 1){
        is_reset = true;
        bool isOk = client->isServerConnected();
        if (isOk)
        {
            client->cancelAllGoals();
        }
        slide_control_msgs::PositionGoal base_goal;
        base_goal.action = 1;
        base_goal.goal.position.x = 0;
        base_goal.goal.position.y = 0;
        base_goal.goal.position.z = 0;
        client->sendGoal(base_goal, &infrared_slide_control::doneCb, &infrared_slide_control::activeCb, &infrared_slide_control::feedbackCb);
    }
}

void infrared_slide_control::ptz_callback(const nav_msgs::Odometry& msg){
    float x = msg.pose.pose.position.x;
    float z = msg.pose.pose.position.z;
    Eigen::AngleAxisd h_off( -1 * M_PI / 180 * x / 100, Vector3d(0, 0, 1));
    Eigen::Quaterniond qua(c_pos.pose.orientation.w, c_pos.pose.orientation.x, c_pos.pose.orientation.y, c_pos.pose.orientation.z);
    Eigen::Quaterniond h_q(h_off);
	qua = h_q * qua;
    Eigen::AngleAxisd v_off( -1 * M_PI / 180 * z / 100, Vector3d(0, 1, 0));
    Eigen::Quaterniond v_q(v_off);
	qua = v_q * qua;
    geometry_msgs::PoseStamped camera_pose;
    camera_pose.header.stamp = ros::Time::now();
    camera_pose.header.frame_id = "map";
	camera_pose.pose.position = c_pos.pose.position;
	camera_pose.pose.orientation.w = qua.w();
    camera_pose.pose.orientation.x = qua.x();
    camera_pose.pose.orientation.y = qua.y();
    camera_pose.pose.orientation.z = qua.z();
    camera_pose_pub.publish(camera_pose);
}

void infrared_slide_control::reset(){
    bool isOk = client->isServerConnected();
    if (isOk)
    {
        client->cancelAllGoals();
    }
    slide_control_msgs::PositionGoal base_goal;
    base_goal.action = 1;
    base_goal.goal.position.x = 0;
    base_goal.goal.position.y = 0;
    base_goal.goal.position.z = 0;
    client->sendGoal(base_goal, &infrared_slide_control::doneCb, &infrared_slide_control::activeCb, &infrared_slide_control::feedbackCb);
}
