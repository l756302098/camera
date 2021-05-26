#include "thermal_proc/thermal_proc_node.h"

#include <algorithm>
#include <cmath>

//#include <opencv2/contrib/contrib.hpp>


namespace flir_gige {

ThermalProcNode::ThermalProcNode(const ros::NodeHandle &nh,
                                 const ros::NodeHandle &pnh)
    : nh_(nh), pnh_(pnh), it_(nh), cfg_server_(pnh) {
  //boost::shared_ptr<ThermalProcNode> foo_object(boost::make_shared<ThermalProcNode>());
  std::string topic;
  pnh.param<std::string>("temp_topic", topic, "");
  //ros::spin();
  if (!sub_camera_) {
    image_transport::TransportHints hints("raw", ros::TransportHints(), nh_);
    sub_camera_ = it_.subscribeCamera("image_raw", 2,
                                      &ThermalProcNode::CameraCb, this, hints);
  }
  pub_proc_ = it_.advertise("/fixed/decoder/infrared", 1);

  // image_transport::SubscriberStatusCallback connect_cb =
  //     boost::bind(&ThermalProcNode::ConnectCb, this);
  // pub_proc_ = it_.advertise("/fixed/decoder/infrared", 1, connect_cb, connect_cb);
  cfg_server_.setCallback(
      boost::bind(&ThermalProcNode::ConfigCb, this, _1, _2));
    temperature_buffer_pub_ = nh_.advertise<sensor_msgs::Image>(topic, 1);

    const int ciVisPictureBufSize = 4 * 256 * 320;
    ucAppendDataBuf_ = new char[ciVisPictureBufSize];
}

ThermalProcNode::~ThermalProcNode()
{
    delete[] ucAppendDataBuf_;
}

bool ThermalProcNode::GetTemperature(temperature::GetTemperature::Request &req,
        temperature::GetTemperature::Response &res){
	if(req.spot_x.size() == 1){
		int row = req.spot_x[0];
		int col = req.spot_y[0];
		res.temperature_c = temperature_[row][col];
	}
	else if(req.spot_x.size() == 2){
		std::cout << "req.spot_x.size() = " << req.spot_x.size() << std::endl;
		unsigned short col_begin = req.spot_x[0];
		std::cout << "col_begin = " << col_begin << std::endl;
		unsigned short row_begin = req.spot_y[0];
		std::cout << "row_begin = " << row_begin << std::endl;
		unsigned short col_end = req.spot_x[1];
		std::cout << "col_end = " << col_end << std::endl;
		unsigned short row_end = req.spot_y[1];
		std::cout << "row_end = " << row_end << std::endl;
		float temperature_max = temperature_[row_begin][col_begin];
		for(unsigned short i = row_begin; i <= row_end; i++){
			for(unsigned short j = col_begin; j <= col_end; j++){
				//std::cout << "i = " << std::endl;
				//std::cout << "j = " << std::endl;
				//std::cout << "t = " << temperature_[i][j] << std::endl;
				if(temperature_max < temperature_[i][j])
				{
					temperature_max = temperature_[i][j];

				}
			}
		}
		res.temperature_c = temperature_max;
	}
	std::cout << "res.temperature_c = " << res.temperature_c << std::endl;
	return true;
}

void ThermalProcNode::ConnectCb() {
  std::lock_guard<std::mutex> lock(connect_mutex_);
  if (!sub_camera_) {
    image_transport::TransportHints hints("raw", ros::TransportHints(), nh_);
    sub_camera_ = it_.subscribeCamera("image_raw", 2,
                                      &ThermalProcNode::CameraCb, this, hints);
  }
}

void ThermalProcNode::ConfigCb(ThermalProcDynConfig &config, int level) {
  if (level < 0) {
    ROS_INFO("%s: %s", pnh_.getNamespace().c_str(),
             "Initializaing reconfigure server");
  }
  // Make sure that max is greater than min
  config.celsius_max = (config.celsius_max > config.celsius_min)
                           ? config.celsius_max
                           : (config.celsius_min + 5);
  config_ = config;
}

void ThermalProcNode::CameraCb(
    const sensor_msgs::ImageConstPtr &image_msg,
    const sensor_msgs::CameraInfoConstPtr &cinfo_msg) {
  // Verify camera is actually calibrated
  /*if (cinfo_msg->K[0] == 0.0 || cinfo_msg->D[0] == 0.0) {
    ROS_ERROR_THROTTLE(5,
                       "Topic '%s' requested but "
                       "camera publishing '%s' is uncalibrated",
                       pub_proc_.getTopic().c_str(),
                       sub_camera_.getNumSubscribersgetInfoTopic().c_str());
    return;
  }*/

  //if (pub_proc_.getNumSubscribers()) {
    const Planck planck = GetPlanck(*cinfo_msg);
    // Get image using cv_bridge
    cv_bridge::CvImagePtr raw_ptr =
        cv_bridge::toCvCopy(image_msg, image_msg->encoding);
    cv::Mat color;
    if (image_msg->encoding == sensor_msgs::image_encodings::MONO8) {
      // Just do a color map conversion for 8 bit
      //cv::applyColorMap(raw_ptr->image, color, cv::COLORMAP_JET);
      cv::applyColorMap(raw_ptr->image, color, cv::COLORMAP_HOT);
    } else if (image_msg->encoding == sensor_msgs::image_encodings::MONO16) {
        //cv::Mat *heat;
        char* temp;
        for (int i = 0; i < raw_ptr->image.rows; ++i) {
            //uint16_t *pheat = heat->ptr<uint16_t>(i);
            const uint16_t *praw = raw_ptr->image.ptr<uint16_t>(i);
            for(int j = 0; j < raw_ptr->image.cols; ++j){
                float t = 0.0;
                t = planck.RawToCelsius(praw[j]);
                //std::cout << t << std::endl;
                temperature_[i][j] = t;
                temp=(char*)(&temperature_[i][j]);
                ucAppendDataBuf_[i * raw_ptr->image.cols * 4 + j * 4] = temp[0];
                ucAppendDataBuf_[i * raw_ptr->image.cols * 4 + j * 4 + 1] = temp[1];
                ucAppendDataBuf_[i * raw_ptr->image.cols * 4 + j * 4 + 2] = temp[2];
                ucAppendDataBuf_[i * raw_ptr->image.cols * 4 + j * 4 + 3] = temp[3];
            }
        }
        std::vector<unsigned char> vc(ucAppendDataBuf_, ucAppendDataBuf_+ raw_ptr->image.rows * raw_ptr->image.cols * 4);
        sensor_msgs::Image msg;
        msg.header.stamp = ros::Time::now();
        msg.height = raw_ptr->image.rows;
        msg.width = raw_ptr->image.cols;
        msg.data = vc;
        msg.step = raw_ptr->image.rows * raw_ptr->image.cols * 4;
        temperature_buffer_pub_.publish(msg);
        RawToJet(raw_ptr->image, planck, &color);
    } else {
      ROS_ERROR_THROTTLE(5, "Encoding not supported: %s",
                         image_msg->encoding.c_str());
      return;
    }
    cv_bridge::CvImage proc_cvimg(image_msg->header,
                                  sensor_msgs::image_encodings::BGR8, color);
    pub_proc_.publish(proc_cvimg.toImageMsg());
  //}
}

void ThermalProcNode::RawToJet(const cv::Mat &raw, const Planck &planck,
                               cv::Mat *color) const {
  const int raw_min = planck.CelsiusToRaw(config_.celsius_min);
  const int raw_max = planck.CelsiusToRaw(config_.celsius_max);
  ROS_ASSERT_MSG(raw_max > raw_min, "max is less than min");
  const double alpha = 255.0 / (raw_max - raw_min) * 3.5;
  const double beta = -alpha * raw_min;
  //std::cout << "raw_max: " << raw_max << " raw_min: " << raw_min << std::endl;
  //const double alpha = 255.0 / (raw_max - raw_min);
  //const double beta = -alpha * raw_min;
  //raw.convertTo(*color, CV_8UC1, alpha, beta);
  //cv::applyColorMap(*color, *color, cv::COLORMAP_JET);
  //cv::cvtColor(*color, *color, cv::COLOR_GRAY2RGB);
  raw.convertTo(*color, CV_8UC1, alpha, beta);
  cv::applyColorMap(*color, *color, cv::COLORMAP_JET);
}

Planck GetPlanck(const sensor_msgs::CameraInfo &cinfo_msg) {
  return Planck(cinfo_msg.R[0], cinfo_msg.R[1], cinfo_msg.R[2], cinfo_msg.R[3]);
}

void RawToHeat(const cv::Mat &raw, const Planck &planck, cv::Mat *heat) {
  for (int i = 0; i < raw.rows; ++i) {
    float *pheat = heat->ptr<float>(i);
    const uint16_t *praw = raw.ptr<uint16_t>(i);
    for (int j = 0; j < raw.cols; ++j) {
      pheat[j] = static_cast<uint16_t>(planck.RawToCelsius(praw[j]));
    }
  }
}

}  // namespace flir_gige
