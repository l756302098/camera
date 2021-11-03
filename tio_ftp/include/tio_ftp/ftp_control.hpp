/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-22 13:21:17
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 14:43:09
 */
#ifndef __FTP_CONTROL_H__
#define __FTP_CONTROL_H__

#include <ros/ros.h>
#include "std_msgs/String.h"
#include <thread>
#include <fstream>
#include <iostream>
#include "glog/logging.h"
#include "gflags/gflags.h"
#include "tio_ftp/upload.h"
#include "tio_ftp/status.h"
#include "ftp/FTPClient.h"

#define PRINT_LOG [](const std::string& strLogMsg) { std::cout << strLogMsg << std::endl; }

class ftp_control
{
private:
	/* data */
	ros::NodeHandle nh_;
	ros::Publisher ftp_status_pub;
	ros::ServiceServer ftp_srv;
	std::shared_ptr<embeddedmz::CFTPClient> ftp_client;
	std::atomic<bool> is_upload;
	int task_id,ftp_port;
	std::string ftp_username,ftp_pwd,ftp_path,upload_file,target_file,ftp_host;
private:
	void load_params();
private:
	void launch_server();
	void launch_publishs();
	bool ftp_srv_cb(tio_ftp::upload::Request &req,tio_ftp::upload::Response &res);
    void pub_ftp_status(float percent,int status = 0);
	void reset();
public:
	ftp_control(const ros::NodeHandle &nh = ros::NodeHandle("~"));
	~ftp_control(){}
	void tick(const ros::TimerEvent &event);
	void test();
	static int ftp_callback(void *owner,double dTotalToDownload, double dNowDownloaded, double dTotalToUpload, double dNowUploaded);
};
#endif