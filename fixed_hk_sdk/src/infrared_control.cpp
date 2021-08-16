/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-06 13:37:38
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 15:18:57
 */
#include "fixed_hk_sdk/infrared_control.hpp"

infrared_control::infrared_control(const ros::NodeHandle &nh):nh_(nh){
    std::string camera_focus_value_str,camera_focus_mm_str;
    std::string camera_file = nh_.param<std::string>("camera_file", "");
    nh_.param<int>("camera_id", camera_id, 1);
    nh_.param<int>("cali_offset_h", cali_offset_h, 0);
    nh_.param<int>("cali_offset_v", cali_offset_v, 0);
    
    meterflag_pub = nh_.advertise<std_msgs::String>("/meter_flag", 1);
    infrared_result_pub = nh_.advertise<fixed_msg::inspected_result>("/infrared_survey_parm", 1);
    transfer_sub = nh_.subscribe("/fixed/platform/transfer", 1, &infrared_control::transfer_callback, this);
    isreach_sub = nh_.subscribe("/fixed/platform/isreach", 1, &infrared_control::isreach_callback, this);
    ptz_sub = nh_.subscribe("/fixed/platform/position", 1, &infrared_control::ptz_callback, this);
    camera_pose_pub = nh_.advertise<geometry_msgs::PoseStamped>("/camera_pose", 1);
	ptz_client = nh_.serviceClient<fixed_msg::cp_control>("/fixed/platform/cmd");
	goal_sub = nh_.subscribe("/goal", 1, &infrared_control::target_callback,this);
	init(camera_file);
}

infrared_control::~infrared_control(){}

void infrared_control::update(){
}

void infrared_control::init(std::string camera_file){
	cv::Mat color_lidar_exRT;
    read_calibration(camera_file, color_lidar_exRT);

    Eigen::Matrix4d transform;
    cv::cv2eigen(color_lidar_exRT, transform);
    std::cout << "Matrix4d:"<< transform.matrix() << std::endl;
    Eigen::Transform<double, 3, Eigen::Affine> a3d_transform(transform);

    std::cout <<"translation:" <<a3d_transform.translation() << std::endl;
    std::cout <<"linear:" <<a3d_transform.linear() << std::endl;
    Eigen::Quaterniond qua;
    Eigen::Matrix3d rotation;
    rotation << a3d_transform.linear();
    qua = rotation;

	//calc offset
	Eigen::AngleAxisd v_off( M_PI / 180 * cali_offset_v, Vector3d(0, 1, 0));
	Quaterniond v_q(v_off);
	qua = v_q * qua;
	Eigen::AngleAxisd h_off( M_PI / 180 * cali_offset_h, Vector3d(0, 0, 1));
	Quaterniond h_q(h_off);
	qua = h_q * qua;

    std::cout << "qua x:" << qua.x()<< " y:" << qua.y()<< " z:" << qua.z()<< " w:" << qua.w() << std::endl;
    Eigen::Vector3d eulerAngle=rotation.eulerAngles(2,1,0);
    std::cout << "roll:" << eulerAngle(0) << std::endl;
    std::cout << "pitch:" << eulerAngle(1) << std::endl;
    std::cout << "yaw:" << eulerAngle(2) << std::endl;

    geometry_msgs::PoseStamped goal;
    goal.header.stamp = ros::Time::now();
    goal.header.frame_id = "map";
    goal.pose.position.x = a3d_transform.translation()[0];
    goal.pose.position.y = a3d_transform.translation()[1];
    goal.pose.position.z = a3d_transform.translation()[2];
    goal.pose.orientation.x = qua.x();
    goal.pose.orientation.y = qua.y();
    goal.pose.orientation.z = qua.z();
    goal.pose.orientation.w = qua.w();
	c_pos.pose.position = goal.pose.position;
    c_pos.pose.orientation = goal.pose.orientation;
}

void infrared_control::read_calibration(std::string _choose_file,cv::Mat &out_RT)
{
    std::cout << "_choose_file: " << _choose_file << std::endl;
    cv::FileStorage fs(_choose_file, cv::FileStorage::READ);
    if (!fs.isOpened())
    {
        std::cout << "Cannot open file calibration file" << _choose_file << std::endl;
    }
    else
    {
      fs["CameraExtrinsicMat"] >> out_RT;
    }
}

void infrared_control::transfer_callback(const fixed_msg::platform_transfer& msg){
    std::cout << "transfer_callback" << std::endl;
    if(msg.flag == 0){
        std::string str_devicepoint = msg.data;
        msg_list.clear();
    	SplitString(str_devicepoint, msg_list, "/");
        if(msg_list.size()<5) return;
        //set value
        if(msg_list[3] == "2" || msg_list[3] == "5"){
            vector<std::string> device_point;
			SplitString(msg_list[4], device_point, ",");
			float x_device = atof(device_point[0].c_str());
			float y_device = atof(device_point[1].c_str());
			float z_device = atof(device_point[2].c_str());
			std::cout << "=====x_device, y_device, z_device value is: " << x_device << " " << y_device << " " << z_device << " .=====" << std::endl;
            t_pos.pose.position.x = x_device;
            t_pos.pose.position.y = y_device;
            t_pos.pose.position.z = z_device;
            Eigen::Quaterniond q(c_pos.pose.orientation.w, c_pos.pose.orientation.x, c_pos.pose.orientation.y, c_pos.pose.orientation.z);
            //forward
            Eigen::Vector3d v(1, 0, 0);
            Eigen::Vector3d f = q * v;
            Eigen::Vector3d c(t_pos.pose.position.x-c_pos.pose.position.x, t_pos.pose.position.y-c_pos.pose.position.y, t_pos.pose.position.z-c_pos.pose.position.z);

            //calc angle/get yaw
            Eigen::AngleAxisd QX90(M_PI / 2, Eigen::Vector3d(0, 0, 1));
            Eigen::Quaterniond t_Q(QX90);
            Eigen::Vector3d r = q * t_Q * v;
	        float dotf = c(0) * f[0] + c(1) * f[1];
	        float dotr = c(0) * r[0] + c(1) * r[1];
            float roll = acos(dotf/sqrt(c(0) * c(0) + c(1) * c(1)));
            float roll_angle = 180 * roll / M_PI;
	        if(dotr>0){
		        std::cout <<" 右 " << roll_angle;
	        }else{
		        std::cout <<" 左 " << roll_angle;
                roll_angle = 360 - roll_angle;
	        }
            //get pitch
            float magnitude = sqrt(c(0) * c(0) + c(1) * c(1) + c(2) * c(2));
            float pitch = asin(abs(c(2)) / magnitude);
            float pitch_angle = 180 * pitch / M_PI;
            if(c(2)<0){
                std::cout <<" 下 " << " angle:" << pitch_angle << std::endl;
            }else{
                pitch_angle =  360 - pitch_angle;
                std::cout <<" 上 " << " angle:" << pitch_angle << std::endl;
            }
	        //ptz_client.call();
	        fixed_msg::cp_control ptz_cmd;
	        ptz_cmd.request.id = camera_id;
	        ptz_cmd.request.action = 1;
	        ptz_cmd.request.type = 3;
	        std::vector<int> quavalue;
	        quavalue.push_back(roll_angle * 100);
	        quavalue.push_back(pitch_angle * 100);
	        ptz_cmd.request.allvalue = quavalue;
	        if(ptz_client.call(ptz_cmd))
		        std::cout << "set xy degree success!" << std::endl;
	        else
		        std::cout << "set xy degree failed!" << std::endl;
            //read callback
            do_task = true;
        }
    }
}

void infrared_control::ptz_callback(const nav_msgs::Odometry& msg){
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

void infrared_control::isreach_callback(const std_msgs::Int32& msg){
    std::cout << "isreach_callback" << std::endl;
    // for (int i = 0; i < msg_list.size(); i++)
    // {
    //     std::cout << " " << i << ":" << msg_list[i];
    // }
    // std::cout << std::endl;
    if(msg.data == 1 && do_task && (msg_list[3] == "2" || msg_list[3] == "5")){
        sleep(5);
        do_task = false;
        fixed_msg::inspected_result inspected_msg;
		inspected_msg.camid = camera_id;
		std::stringstream ss;
		ss.str("");
		ss << msg_list[2] << ":" << msg_list[3] << "/" << msg_list[4] << "/" << msg_list[5] << "/" << msg_list[6] << "/" << msg_list[7];
		inspected_msg.equipid = ss.str();
        inspected_msg.success = true;
		infrared_result_pub.publish(inspected_msg);
    }
}

void infrared_control::reset(){
    fixed_msg::cp_control ptz_cmd;
	ptz_cmd.request.id = camera_id;
	ptz_cmd.request.action = 1;
	ptz_cmd.request.type = 4;
	ptz_cmd.request.value = 0;
	vector<int> value;
	value.push_back(0);
	value.push_back(0);
	value.push_back(0);
	ptz_cmd.request.allvalue = value;
	if(ptz_client.call(ptz_cmd))
		std::cout << "set xyz_zoom degree success!" << std::endl;
	else
		std::cout << "set xyz_zoom degree failed!" << std::endl;
}

void infrared_control::target_callback(const geometry_msgs::PoseStampedConstPtr &msg){
    t_pos.pose.position.x = msg->pose.position.x;
    t_pos.pose.position.y = msg->pose.position.y;
    t_pos.pose.position.z = msg->pose.position.z;
    Eigen::Quaterniond q(c_pos.pose.orientation.w, c_pos.pose.orientation.x, c_pos.pose.orientation.y, c_pos.pose.orientation.z);
    //forward
    Eigen::Vector3d v(1, 0, 0);
    Eigen::Vector3d f = q * v;
    Eigen::Vector3d c(t_pos.pose.position.x-c_pos.pose.position.x, t_pos.pose.position.y-c_pos.pose.position.y, t_pos.pose.position.z-c_pos.pose.position.z);

    //calc angle/get yaw
    Eigen::AngleAxisd QX90(M_PI / 2, Eigen::Vector3d(0, 0, 1));
    Eigen::Quaterniond t_Q(QX90);
    Eigen::Vector3d r = q * t_Q * v;
	float dotf = c(0) * f[0] + c(1) * f[1];
	float dotr = c(0) * r[0] + c(1) * r[1];
    float roll = acos(dotf/sqrt(c(0) * c(0) + c(1) * c(1)));
    float roll_angle = 180 * roll / M_PI;
	if(dotr>0){
		std::cout <<" 右" <<std::endl;
	}else{
		std::cout <<" 左" <<std::endl;
        roll_angle = 360 - roll_angle;
	}
    //get pitch
    float magnitude = sqrt(c(0) * c(0) + c(1) * c(1) + c(2) * c(2));
    float pitch = asin(abs(c(2)) / magnitude);
    float pitch_angle = 180 * pitch / M_PI;
    //std::cout <<"pitch_angle:" << pitch_angle <<std::endl;
    if(c(2)<0){
        std::cout <<"下" << " angle:"<<pitch_angle<< std::endl;
    }else{
        pitch_angle =  360 - pitch_angle;
        std::cout <<"上" << " angle:"<<pitch_angle << std::endl;
    }
	//ptz_client.call();
	fixed_msg::cp_control ptz_cmd;
	ptz_cmd.request.id = camera_id;
	ptz_cmd.request.action = 1;
	ptz_cmd.request.type = 3;
	std::vector<int> quavalue;
	quavalue.push_back(roll_angle * 100);
	quavalue.push_back(pitch_angle * 100);
	ptz_cmd.request.allvalue = quavalue;
	if(ptz_client.call(ptz_cmd))
		std::cout << "set xy degree success!" << std::endl;
	else
		std::cout << "set xy degree failed!" << std::endl;
	sleep(0.2);    
}