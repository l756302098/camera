#include "tio_ftp/ftp_control.hpp"

ftp_control::ftp_control(const ros::NodeHandle &nh):nh_(nh),is_upload(false),task_id(0){
    load_params();
    launch_server();
    launch_publishs();
    ftp_client.reset(new embeddedmz::CFTPClient(PRINT_LOG));
    bool is_login = ftp_client->InitSession(ftp_host, ftp_port, ftp_username, ftp_pwd,embeddedmz::CFTPClient::FTP_PROTOCOL::FTP, embeddedmz::CFTPClient::SettingsFlag::ALL_FLAGS);
    ftp_client->SetInsecure(true);
    if(is_login){
        LOG(INFO) << "login success";
    }else{
        LOG(ERROR) << "login failed";
        exit(1);
    }
    //test();
}

void ftp_control::load_params(){
    nh_.param<std::string>("ftp_host", ftp_host, "");
    nh_.param<int>("ftp_port", ftp_port, 21);
    nh_.param<std::string>("ftp_username", ftp_username, "");
    nh_.param<std::string>("ftp_pwd", ftp_pwd, "");
    nh_.param<std::string>("ftp_path", ftp_path, "");
}

void ftp_control::launch_server(){
    ftp_srv = nh_.advertiseService("/tio/ftp/upload",  &ftp_control::ftp_srv_cb, this);
}
void ftp_control::launch_publishs(){
    ftp_status_pub = nh_.advertise<tio_ftp::status>("/tio/ftp/status", 1);
}
bool ftp_control::ftp_srv_cb(tio_ftp::upload::Request &req,tio_ftp::upload::Response &res){
    if(task_id!=0){
        res.success = false;
        res.message = "Task is continuing";
        return true;
    }
    if(req.flag == 1){
        is_upload = true;
        upload_file = req.src_name;
        task_id = req.task_id;
        target_file = ftp_path + "/"+ std::to_string(req.map_id)+ "/" + req.target_name;
    }else if(req.flag == 2){
        reset();
    }
    res.success = true;
    res.message = "ok";
    return true;
}
void ftp_control::pub_ftp_status(float percent,int status){
    tio_ftp::status ftp_status;
    ftp_status.task_id = task_id;
    ftp_status.progress = percent;
    ftp_status.status = status;
    ftp_status_pub.publish(ftp_status);
}
void ftp_control::tick(const ros::TimerEvent &event){
    if(is_upload){
        LOG(INFO) << "start upload " << upload_file << " to ftp " << target_file;
        ftp_client->SetActive(true);
        ftp_client->SetProgressFnCallback(this,&ftp_control::ftp_callback);
        bool ok = ftp_client->UploadFile(upload_file, target_file,true);
        if(!ok){
            LOG(ERROR) << "upload failed";
            reset();
            pub_ftp_status(1,2);
        }else{
            LOG(INFO) << "upload success";
            pub_ftp_status(1,1);
            reset();
        }
    }
}

int ftp_control::ftp_callback(void *owner,double dTotalToDownload, double dNowDownloaded, double dTotalToUpload, double dNowUploaded){
    LOG(WARNING) << dTotalToDownload << " " << dNowDownloaded << " " << dTotalToUpload << " " << dNowUploaded;
    if(owner){
        ftp_control *fc = (ftp_control*)owner;
        float percent = 1.0 * dNowUploaded / dTotalToUpload;
        fc->pub_ftp_status(percent);
    }
    return 0;
}

void ftp_control::test(){
    if(ftp_client){
        ftp_client->SetActive(true);
        ftp_client->SetProgressFnCallback(this,&ftp_control::ftp_callback);
        LOG(INFO) << "start upload";
        bool ok = ftp_client->UploadFile("/home/li/bag/ply_8_20211008112615.ply", "/file/input/2.ply",true);
        if(!ok){
            LOG(ERROR) << "upload failed";
            pub_ftp_status(1,2);
        }else{
            LOG(INFO) << "upload success";
            pub_ftp_status(1,1);
        }
    }
}

void ftp_control::reset(){
    is_upload = false;
    task_id = 0;
    upload_file = "";
}