/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-06 13:37:38
 * @LastEditors: li
 * @LastEditTime: 2021-05-13 14:11:58
 */
#include "fixed_yd_onvif/visible_control.hpp"

visible_control::visible_control(const ros::NodeHandle &nh):nh_(nh),do_task(false){
    std::string camera_focus_dis_str,camera_focus_zoom_str;
    std::vector<std::string> camera_focus_dis_array,camera_focus_zoom_array;
    std::string camera_file = nh_.param<std::string>("camera_file", "");
    nh_.param<int>("camera_id", camera_id, 1);
    nh_.param<int>("cali_offset_h", cali_offset_h, 0);
    nh_.param<int>("cali_offset_v", cali_offset_v, 0);
    nh_.param<double>("h_offset", h_offset, 0);
    nh_.param<double>("v_offset", v_offset, 0);
    nh_.param<bool>("auto_zoom", auto_zoom, false);
    nh_.param<int>("camera_image_width", camera_image_width, 1920);
    nh_.param<int>("camera_image_height", camera_image_height, 1080);
    nh_.param<double>("camera_cmos_width", camera_cmos_width, 3.75);
    nh_.param<double>("camera_cmos_height", camera_cmos_height, 3.75);
    nh_.param<double>("camera_pixel_size", camera_pixel_size, 3.75);
    nh_.param<std::string>("camera_focus_mm", camera_focus_dis_str, "4.3, 8.6");
    SplitString(camera_focus_dis_str, camera_focus_dis_array, ",");
    for(int i = 0; i < camera_focus_dis_array.size(); i++)
        camera_focus_dis.push_back(atoi(camera_focus_dis_array[i].c_str()));
    
    nh_.param<std::string>("camera_focus_value", camera_focus_zoom_str, "0x0");
    SplitString(camera_focus_zoom_str, camera_focus_zoom_array, ",");
    for(int i = 0; i < camera_focus_zoom_array.size(); i++)
        camera_focus_set.push_back(atoi(camera_focus_zoom_array[i].c_str()));
        
    transfer_sub = nh_.subscribe("/fixed/platform/transfer", 1, &visible_control::transfer_callback, this);
    isreach_sub = nh_.subscribe("/fixed/platform/isreach", 1, &visible_control::isreach_callback, this);
    detectresult_sub = nh_.subscribe("/detect_rect", 1, &visible_control::detect_rect_callback, this);
    ptz_sub = nh_.subscribe("/fixed/platform/position", 1, &visible_control::ptz_callback, this);
	ptz_client = nh_.serviceClient<fixed_msg::cp_control>("/fixed/platform/cmd");
	camera_pose_pub = nh_.advertise<geometry_msgs::PoseStamped>("/camera_pose", 1);
    readyimage_pub = nh_.advertise<fixed_msg::inspected_result>("/visible_survey_parm", 1);
	goal_sub = nh_.subscribe("/goal", 1, &visible_control::target_callback,this);
	init(camera_file);
}

visible_control::~visible_control(){}

void visible_control::update(){
}

void visible_control::init(std::string camera_file){
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

void visible_control::read_calibration(std::string _choose_file,cv::Mat &out_RT)
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

void visible_control::transfer_callback(const fixed_msg::platform_transfer& msg){
    std::cout << "transfer_callback" << std::endl;
    if(msg.flag == 0){
        std::string str_devicepoint = msg.data;
        //std::vector<std::string> msg_list;
        msg_list.clear();
    	SplitString(str_devicepoint, msg_list, "/");
        if(msg_list.size()<5) return;
        //set value
        if(msg_list[3] == "1"){
            vector<std::string> device_point;
			SplitString(msg_list[4], device_point, ",");
			float x_device = atof(device_point[0].c_str());
			float y_device = atof(device_point[1].c_str());
			float z_device = atof(device_point[2].c_str());
			int device_width = atoi(device_point[3].c_str());
			int device_height = atoi(device_point[4].c_str());
			device_type = atoi(device_point[5].c_str());
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
            yaw_first = 180 * roll / M_PI;
	        if(dotr>0){
		        std::cout <<" 右 " << yaw_first;
	        }else{
                std::cout <<" 左 " << yaw_first;
                yaw_first = 360 - yaw_first;
	        }
            //get pitch
            float magnitude = sqrt(c(0) * c(0) + c(1) * c(1) + c(2) * c(2));
            float pitch = asin(abs(c(2)) / magnitude);
            pitch_first = 180 * pitch / M_PI;
            std::cout << " first calc angle:" << pitch_first << std::endl;
            //again calc angle
            if(h_offset>0){
                double xy_distance = sqrt(magnitude * magnitude - c(2) * c(2));
                double z_distance = abs(c(2));
                double offset_z = h_offset * cos(pitch);
                double offset_x = h_offset * sin(pitch);
                double xy = xy_distance + offset_x;
                double z = z_distance + h_offset - offset_z;
                float pitch1 = atan(z/xy);
                pitch_first = 180 * pitch1 / M_PI;
                std::cout << "again calc angle:" << pitch_first << std::endl;
                if(c(2)<0){
                    std::cout <<" 下" << " angle:"<<pitch_first<< std::endl;
                }else{
                    pitch_first = 360 - pitch_first;
                    std::cout <<" 上" << " angle:"<<pitch_first << std::endl;
                }
            }
            //calc zoom
            int zoom_set = get_zoomset(magnitude, device_width, device_height, 5);
            std::cout << "magnitude:" << magnitude <<"h angle:" << yaw_first << " v angle:" << pitch_first << std::endl;
            std::cout << "focus_dis:" << focus_dis << " zoom_set:" << zoom_set << std::endl;
	        fixed_msg::cp_control ptz_cmd;
	        ptz_cmd.request.id = camera_id;
	        ptz_cmd.request.action = 1;
	        ptz_cmd.request.type = 4;
	        std::vector<unsigned int> quavalue;
	        quavalue.push_back(yaw_first * 100);
	        quavalue.push_back(pitch_first * 100);
            quavalue.push_back(focus_dis * 100);
	        ptz_cmd.request.allvalue = quavalue;
	        if(ptz_client.call(ptz_cmd))
		        std::cout << "set xy degree success!" << std::endl;
	        else
		        std::cout << "set xy degree failed!" << std::endl;
			//read callback
            do_task = true;
            watch_counter = 1;
        }
    }else if(msg.flag == 1){
        //复位
        reset();
    }
}

void visible_control::isreach_callback(const std_msgs::Int32& msg){
    std::cout << "isreach_callback " << msg_list.size() << std::endl;
    if(msg_list.size()<7) return;
    if(msg.data == 1 && do_task)
	{
        sleep(5);
		do_task = false;
		fixed_msg::inspected_result imagezoom_msg;
		imagezoom_msg.camid = watch_counter;
		imagezoom_msg.picid = device_type;
		std::stringstream ss;
		ss.str("");
		ss << msg_list[2] << "," << msg_list[4] << "/" << msg_list[5] << "/" << msg_list[6];
		imagezoom_msg.equipid = ss.str();
		readyimage_pub.publish(imagezoom_msg);
	}
}

void visible_control::ptz_callback(const nav_msgs::Odometry& msg){
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

void visible_control::detect_rect_callback(const fixed_msg::detect_result& msg){
    watch_counter = 2;
    do_task = true;
    if(auto_zoom){
        sleep(2);
        std_msgs::Int32 msg;
        msg.data = 1;
		isreach_callback(msg);
        return;
    }
    vector<std::string> device_point;
    SplitString(msg_list[4], device_point, ",");
	float x_device = atof(device_point[0].c_str());
	float y_device = atof(device_point[1].c_str());
	float z_device = atof(device_point[2].c_str());
	int device_width = atoi(device_point[3].c_str());
	int device_height = atoi(device_point[4].c_str());
	device_type = atoi(device_point[5].c_str());
    Eigen::Vector3d c(t_pos.pose.position.x-c_pos.pose.position.x, t_pos.pose.position.y-c_pos.pose.position.y, t_pos.pose.position.z-c_pos.pose.position.z);
	//计算检测框的中心坐标
	int framecentral_x = msg.xmin + ((msg.xmax - msg.xmin) / 2);
	int framecentral_y = msg.ymin + ((msg.ymax - msg.ymin) / 2);
	//（与中心坐标的）偏移量
	int offset_x = framecentral_x - (camera_image_width / 2);
	int offset_y = framecentral_y - (camera_image_height / 2);
    std::cout << "offset_x:" << offset_x << " offset_y:" << offset_y << std::endl;
	//计算焦距对应像素点个数
	//float focus_set = focus_dis / camera_pixel_size * 1000;
    float focus_set = focus_dis * camera_image_width / camera_cmos_width;
	//计算水平、垂直偏移量（换算成云台对应的度制）
	float horizontal_x = ((atan(offset_x / focus_set) / M_PI) * 180);
	float vertical_y = ((atan(offset_y / focus_set) / M_PI) * 180);
    std::cout << "horizontal_x:" << horizontal_x << " vertical_y:" << vertical_y << std::endl;
    if(offset_x>0){
        yaw_first = yaw_first + horizontal_x;
    }else{
        yaw_first = yaw_first - horizontal_x;
    }
    if(offset_y>0){
        pitch_first = pitch_first - vertical_y;
    }else{
        pitch_first = pitch_first + vertical_y;
    }
    //calc zoom
    float magnitude = sqrt(c(0) * c(0) + c(1) * c(1) + c(2) * c(2));
    int zoom_set = get_zoomset(magnitude, device_width, device_height, 2);
    std::cout << "h angle:" << yaw_first << " v angle:" << pitch_first << std::endl;
    std::cout << "focus_dis:" << focus_dis << " zoom_set:" << zoom_set << std::endl;
	fixed_msg::cp_control ptz_cmd;
	ptz_cmd.request.id = camera_id;
	ptz_cmd.request.action = 1;
	ptz_cmd.request.type = 4;
	std::vector<unsigned int> quavalue;
	quavalue.push_back(yaw_first * 100);
	quavalue.push_back(pitch_first * 100);
    quavalue.push_back(focus_dis * 100);
	ptz_cmd.request.allvalue = quavalue;
	if(ptz_client.call(ptz_cmd))
		std::cout << "set xy degree success!" << std::endl;
	else
		std::cout << "set xy degree failed!" << std::endl;
}

void visible_control::reset(){
    fixed_msg::cp_control ptz_cmd;
	ptz_cmd.request.id = camera_id;
	ptz_cmd.request.action = 1;
	ptz_cmd.request.type = 4;
	ptz_cmd.request.value = 0;
	vector<unsigned int> value;
	value.push_back(0);
	value.push_back(0);
	value.push_back(0);
	ptz_cmd.request.allvalue = value;
	if(ptz_client.call(ptz_cmd))
		std::cout << "set xyz_zoom degree success!" << std::endl;
	else
		std::cout << "set xyz_zoom degree failed!" << std::endl;
}

void visible_control::target_callback(const geometry_msgs::PoseStampedConstPtr &msg){
	std::cout << "target_callback" << std::endl;
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
    std::cout <<"magnitude:" << magnitude << " offz:" << abs(c(2))  <<std::endl;
    if(c(2)<0){
        std::cout <<"下" << " angle:"<<pitch_angle<< std::endl;
    }else{
        pitch_angle =  360 - pitch_angle;
        std::cout <<"上" << " angle:"<<pitch_angle << std::endl;
    }
    //calc angle
    

	//ptz_client.call();
	fixed_msg::cp_control ptz_cmd;
	ptz_cmd.request.id = camera_id;
	ptz_cmd.request.action = 1;
	ptz_cmd.request.type = 3;
	std::vector<unsigned int> quavalue;
	quavalue.push_back(roll_angle * 100);
	quavalue.push_back(pitch_angle * 100);
	ptz_cmd.request.allvalue = quavalue;
	if(ptz_client.call(ptz_cmd))
		std::cout << "set xy degree success!" << std::endl;
	else
		std::cout << "set xy degree failed!" << std::endl;
	sleep(0.2);
    do_task = true;
}

int visible_control::get_zoomset(float distance, int device_width, int device_height, int zoom_scale)
{
    int finalzoom_set = 8;
	if((device_width == 0) || (device_height == 0))
    {
		finalzoom_set = 8;
		std::cout << "device size is 00000000000000000000000 !" << std::endl;
    }
	else
    {
		float f_width = camera_cmos_width * distance * 1000 / device_width / zoom_scale;
		float f_height = camera_cmos_height * distance * 1000 / device_height / zoom_scale;
        std::cout << "f_width result is: " << f_width << ", f_height is: " << f_height << "." << std::endl;

		int w_finalzoom_set = finalzoom_set;
		int h_finalzoom_set = finalzoom_set;
		double w_zoom_focus_dis = 0;
		double h_zoom_focus_dis = 0;
        if(f_width >= camera_focus_dis[camera_focus_dis.size() - 1]){
            w_finalzoom_set = camera_focus_set[camera_focus_dis.size() - 1];
			w_zoom_focus_dis = camera_focus_dis[camera_focus_dis.size() - 1];
        }else{
		    for(int i = 0; i < camera_focus_dis.size() - 1; i++)
            {
			    if ((f_width >= camera_focus_dis[i]) && (f_width <= camera_focus_dis[i+1]))
                {
				    w_finalzoom_set = camera_focus_set[i];
				    w_zoom_focus_dis = camera_focus_dis[i];
                    break;
                }
            }
        }

        if(f_height >= camera_focus_dis[camera_focus_dis.size() - 1]){
            h_finalzoom_set = camera_focus_set[camera_focus_dis.size() - 1];
			h_zoom_focus_dis = camera_focus_dis[camera_focus_dis.size() - 1];
        }else{
		    for(int i = 0; i < camera_focus_dis.size() - 1; i++)
            {
			    if ((f_height >= camera_focus_dis[i]) && (f_height <= camera_focus_dis[i+1]))
                {
				    h_finalzoom_set = camera_focus_set[i];
				    h_zoom_focus_dis = camera_focus_dis[i];
                    break;
                }
            }
        }
		finalzoom_set = w_finalzoom_set >= h_finalzoom_set ? h_finalzoom_set : w_finalzoom_set;
        focus_dis = w_zoom_focus_dis >= h_zoom_focus_dis ? h_zoom_focus_dis : w_zoom_focus_dis;
    }

	if(finalzoom_set < 8)
		finalzoom_set = 8;
	std::cout << "=====get_zoomset result is: " << finalzoom_set << "." << std::endl;
	return finalzoom_set;
}
