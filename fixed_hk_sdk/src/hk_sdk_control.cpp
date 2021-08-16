/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-01 13:11:04
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 15:07:44
 */
#include "ros/ros.h"
#include "fixed_hk_sdk/hk_sdk_control.hpp"

using namespace std;

hk_sdk_control::hk_sdk_control(const ros::NodeHandle &nh):nh_(nh)
{
    nh_.param<bool>("auto_zoom", auto_zoom, false);
    nh_.param<bool>("ptz_option", ptz_option, false);
    //nh_.param<std::string>("haikang_topic_name", visible_topic_str, "/haikangsdk/image_proc");
    nh_.param<std::string>("sdkcom_dir", sdkcom_dir_str, "/home/yd/workspace/src/yd_camerastream_capture/libs");
    nh_.param<std::string>("heartbeat_topic_string", heartbeat_topic_str, "/fixed/heartbeat");
    nh_.param<std::string>("device_ip", m_device_ip, "192.168.1.64");
    nh_.param<std::string>("device_port", m_device_port, "8000");
    nh_.param<std::string>("device_username", m_device_username, "admin");
    nh_.param<std::string>("device_password", m_device_password, "abcd1234");
    //nh_.param<bool>("set_infrared_focus", m_set_infrared_focus, true);
    nh_.param<int>("set_infrared_focus_waittime", _set_infrared_focus_waittime, 30);
    nh_.param<std::string>("ptz_topic", ptz_topic, "/fixed/platform/position");
    nh_.param<std::string>("ptz_server_name", ptz_server_name, "/fixed/platform/cmd");
    nh_.param<bool>("pub_raw_temp", pub_raw_temp, true);
    nh_.param<std::string>("raw_temp_topic", raw_temp_topic, "/fixed/infrared/raw");
    nh_.param<int>("infrared_channel", infrared_channel, 1);
    std::string pan_range_str,tilt_range_str;
    std::vector<std::string> pan_range_list,tilt_range_list;
    nh_.param<std::string>("pan_range", pan_range_str, "0,0");
    SplitString(pan_range_str, pan_range_list, ",");
    pan_min = atoi(pan_range_list[0].c_str());
    pan_max = atoi(pan_range_list[1].c_str());
    nh_.param<std::string>("tilt_range", tilt_range_str, "0,0");
    SplitString(pan_range_str, tilt_range_list, ",");
    tilt_min = atoi(tilt_range_list[0].c_str());
    tilt_max = atoi(tilt_range_list[1].c_str());

    heartbeat_pub_ = nh_.advertise<diagnostic_msgs::DiagnosticArray>(heartbeat_topic_str, 1);
    if(ptz_option){
        isreach_pub_ = nh_.advertise<std_msgs::Int32>("/fixed/platform/isreach", 1);
        ptz_pub_ = nh_.advertise<nav_msgs::Odometry>(ptz_topic, 1);
        detect_sub = nh_.subscribe("/detect_rect", 1, &hk_sdk_control::detect_rect_callback, this);
        ptz_server = nh_.advertiseService(ptz_server_name, &hk_sdk_control::handle_cloudplatform, this);
    }
    raw_temp_pub_ = nh_.advertise<sensor_msgs::Image>(raw_temp_topic,1);

    init_sdk();
    ROS_INFO("init finish");
}

hk_sdk_control::~hk_sdk_control()
{

}

void hk_sdk_control::update()
{
    nav_msgs::Odometry msg_pos;
	msg_pos.header.stamp = ros::Time::now();
	msg_pos.pose.pose.position.x = g_now_xyposition;
	msg_pos.pose.pose.position.z = g_now_zposition;
    msg_pos.pose.pose.position.y = g_now_zoom;
	ptz_pub_.publish(msg_pos);

    if(!ptz_option) return;
    if((g_xy_reach_flag == 1) && (g_z_reach_flag == 1))
    {
        g_xy_reach_flag = 0;
        g_z_reach_flag = 0;
        g_reach_flag = 1;
        std_msgs::Int32 res_msg;
		res_msg.data = 1;
		isreach_pub_.publish(res_msg);
        std::cout << "isreach publish" << std::endl;
    }
}


void hk_sdk_control::pub_heartbeat(int level, string message)
{
    diagnostic_msgs::DiagnosticArray log;
    log.header.stamp = ros::Time::now();
    diagnostic_msgs::DiagnosticStatus s;
    s.name = app_name;             // 这里写节点的名字
    s.level = level;                   // 0 = OK, 1 = Warn, 2 = Error
    if (!message.empty())
    {
        s.message = message;           // 问题描述
    }
    s.hardware_id = "";   // 硬件信息
    log.status.push_back(s);
    heartbeat_pub_.publish(log);
}

void hk_sdk_control::init_sdk(){
    char* device_ip = new char[100];
    strcpy(device_ip, m_device_ip.c_str());
    char* device_username = new char[100];
    strcpy(device_username, m_device_username.c_str());
    char* device_password = new char[100];
    strcpy(device_password, m_device_password.c_str());

    lUserID = -1;
    lRealPlayHandle = -1;

    NET_DVR_LOCAL_SDK_PATH struComPath = {0};
    sprintf(struComPath.sPath, "%s", sdkcom_dir_str.c_str());
    NET_DVR_SetSDKInitCfg(NET_SDK_INIT_CFG_SDK_PATH, &struComPath);

    NET_DVR_Init();
    //登录参数，包括设备地址、登录用户、密码等
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    struLoginInfo.bUseAsynLogin = 0;                    //同步登录方式
    strcpy(struLoginInfo.sDeviceAddress, device_ip);    //设备IP地址
    struLoginInfo.wPort = atoi(m_device_port.c_str());  //设备服务端口
    strcpy(struLoginInfo.sUserName, device_username);   //设备登录用户名
    strcpy(struLoginInfo.sPassword, device_password);   //设备登录密码
    
    NET_DVR_DEVICEINFO_V40 struDeviceInfo = {0};
    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfo);
    if (lUserID < 0)
    {
        printf("ip: %s---Login error,  %d\n", m_device_ip.c_str(), NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        ros::shutdown();
        return;
    }
        NET_DVR_PTZCFG m_ptzcfg;
        NET_DVR_GetPTZProtocol(lUserID, &m_ptzcfg);
        std::cout << "支持协议的个数：" << m_ptzcfg.dwPtzNum << std::endl;
        for(int i = 0; i < m_ptzcfg.dwPtzNum; i++)
        {
            printf("info is %s:", m_ptzcfg.struPtz[i].byDescribe);
            std::cout << "decoder type is :" << m_ptzcfg.struPtz[i].dwType << std::endl;
        }

        //获取水平、垂直及变倍值范围值
        NET_DVR_PTZSCOPE m_ptzScope;
        DWORD dwOutBufferSize = 1000;
        DWORD lpBytesReturn = 0;
        if(NET_DVR_GetDVRConfig(0, NET_DVR_GET_PTZSCOPE, 0, &m_ptzScope, dwOutBufferSize, &lpBytesReturn))
        {
            std::cout << "wPanPosMin:" << m_ptzScope.wPanPosMin << std::endl;
            std::cout << "wPanPosMax:" << m_ptzScope.wPanPosMax << std::endl;
            std::cout << "wTiltPosMin:" << m_ptzScope.wTiltPosMin << std::endl;
            std::cout << "wTiltPosMax:" << m_ptzScope.wTiltPosMax << std::endl;
            std::cout << "wZoomPosMin:" << m_ptzScope.wZoomPosMin << std::endl;
            std::cout << "wZoomPosMax:" << m_ptzScope.wZoomPosMax << std::endl;
            std::cout << "lpBytesReturn:" << lpBytesReturn << std::endl;
        }
        
        //获取当前水平、垂直及变倍值
        NET_DVR_PTZPOS m_ptzPosNow;
        if(NET_DVR_GetDVRConfig(0, NET_DVR_GET_PTZPOS, 0, &m_ptzPosNow, dwOutBufferSize, &lpBytesReturn))
        {
            std::cout << "wPanPos:" << m_ptzPosNow.wPanPos << std::endl;
            std::cout << "wTiltPos:" << m_ptzPosNow.wTiltPos << std::endl;
            std::cout << "wZoomPos:" << m_ptzPosNow.wZoomPos << std::endl;
            std::cout << "lpBytesReturn:" << lpBytesReturn << std::endl;
        }

        DWORD dwChannel = 1;  //热成像通道
        char *m_pOutBuf = new char[ISAPI_OUT_LEN];
        memset(m_pOutBuf, 0, ISAPI_OUT_LEN);
        char *m_pStatusBuf = new char[ISAPI_STATUS_LEN];
        memset(m_pStatusBuf, 0, ISAPI_STATUS_LEN);

        //测温能力集
        NET_DVR_STD_ABILITY struStdAbility = {0};
        struStdAbility.lpCondBuffer = &dwChannel;
        struStdAbility.dwCondSize = sizeof(DWORD);
        struStdAbility.lpOutBuffer    = m_pOutBuf;
        struStdAbility.dwOutSize      = ISAPI_OUT_LEN;
        struStdAbility.lpStatusBuffer = m_pStatusBuf;
        struStdAbility.dwStatusSize   = ISAPI_STATUS_LEN;
        //NET_DVR_GetSTDAbility 获取设备能力集
        if(!NET_DVR_GetSTDAbility(lUserID,NET_DVR_GET_THERMAL_CAPABILITIES,&struStdAbility)){
            printf("net_dvr_get_thermal_capabilities failed, error code: %d\n", NET_DVR_GetLastError());
        }
        else{
            printf("net_dvr_get_thermal_capabilities is successful!\n");
        }

        if(!NET_DVR_GetSTDAbility(lUserID,NET_DVR_GET_THERMOMETRY_BASICPARAM_CAPABILITIES,&struStdAbility)){
            printf("net_dvr_thermometry_basicparam_capabilities failed, error code: %d\n", NET_DVR_GetLastError());
        }
        else{
            printf("net_dvr_thermometry_basicparam_capabilities is successful!\n");
        }

        if(!NET_DVR_GetSTDAbility(lUserID,NET_DVR_GET_THERMOMETRY_SCENE_CAPABILITIES,&struStdAbility)){
            printf("net_dvr_get_thermometry_scene_capabilities failed, error code: %d\n", NET_DVR_GetLastError());
        }
        else{
            printf("net_dvr_get_thermometry_scene_capabilities is successful!\n");
        }
        
        if(!NET_DVR_GetSTDAbility(lUserID,NET_DVR_GET_THERMOMETRY_MODE_CAPABILITIES,&struStdAbility)){
            printf("net_dvr_get_thermometry_mode_capabilities failed, error code: %d\n", NET_DVR_GetLastError());
        }
        else{
            printf("net_dvr_get_thermometry_mode_capabilities is successful!\n");
        }

        //设置测温模式
        NET_DVR_STD_CONFIG m_stdset_config = {0};
        m_stdset_config.lpCondBuffer = &dwChannel;
        m_stdset_config.dwCondSize = sizeof(DWORD);
        NET_DVR_THERMOMETRY_MODE m_mode_info;
        m_mode_info.dwSize = sizeof(m_mode_info);
        m_mode_info.byMode = 1;
        m_stdset_config.lpInBuffer = &m_mode_info;
        m_stdset_config.dwInSize = sizeof(m_mode_info);
        if(!NET_DVR_SetSTDConfig(lUserID, NET_DVR_SET_THERMOMETRY_MODE, &m_stdset_config)){
            printf("NET_DVR_SET_MODE failed, error code: %d\n", NET_DVR_GetLastError());
        }
}

void hk_sdk_control::write_hk(){
    while(true)
    {
        sleep(0.1);
        std::string work_cmd = "";
        //std::cout << "this:" << this << std::endl;
        if(this->_cmd_control_queue.size() > 0)
        {
            this->write_mtx.lock();
            std::cout << "before execute, _cmd_control_queue size is : " << this->_cmd_control_queue.size() << std::endl;
            work_cmd = this->_cmd_control_queue[0];
            this->_cmd_control_queue.pop_front();
            std::cout << "after execute, _cmd_control_queue size is : " << this->_cmd_control_queue.size() << std::endl;
            this->write_mtx.unlock();

            if(work_cmd != "")
            {
                vector<std::string> cmd_value_strv;
                SplitString(work_cmd, cmd_value_strv, "/");

                int id = atoi(cmd_value_strv[0].c_str());
                int action = atoi(cmd_value_strv[1].c_str());
                int type = atoi(cmd_value_strv[2].c_str());
                int value = atoi(cmd_value_strv[3].c_str());
                std::cout << "id: " << id << ", action: " << action << ", type: " << type << ", value: " << value << std::endl;

                int xy_value = 0;
                int z_value = 0;
                int zoom_value = 0;
                if(type == 3)
                {
                    xy_value = atoi(cmd_value_strv[4].c_str());
                    z_value = atoi(cmd_value_strv[5].c_str());
                }
                if(type == 4)
                {
                    std::cout << "in type 4!" << std::endl;
                    xy_value = atoi(cmd_value_strv[4].c_str());
                    z_value = atoi(cmd_value_strv[5].c_str());
                    zoom_value = atoi(cmd_value_strv[6].c_str());
                }

                g_action = action;
                if((type == 0) || (type == 1))
                {
                    g_control_type = type;
                    if(g_control_type == 0)
                        g_xy_goal = value;
                    else if(g_control_type == 1)
                        g_z_goal = value;
                }
                else if((type == 3) || (type == 4))
                {
                    g_control_type = type;
                    g_xy_goal = xy_value;
                    g_z_goal = z_value;
                }
            }
        }
        else if(this->_cmd_read_queue.size() > 0)
        {
            this->read_mtx.lock();
            //std::cout << "before execute, _cmd_read_queue size is : " << this->_cmd_read_queue.size() << std::endl;
            work_cmd = this->_cmd_read_queue[0];
            this->_cmd_read_queue.pop_front();
            //std::cout << "after execute, _cmd_read_queue size is : " << this->_cmd_read_queue.size() << std::endl;
            this->read_mtx.unlock();
        }
        if(work_cmd != "")
        {
            unsigned int result_value = 0;
            vector<std::string> cmd_value_strv;
            SplitString(work_cmd, cmd_value_strv, "/");

            //action:0 获取  action:1 设置
            //type:0 水平  type:1 垂直  type:2 变倍
            int id = atoi(cmd_value_strv[0].c_str());
            int action = atoi(cmd_value_strv[1].c_str());
            int type = atoi(cmd_value_strv[2].c_str());
            int value = atoi(cmd_value_strv[3].c_str());
            int xy_value = 0;
            int z_value = 0;
            int zoom_value = 0;
            if((action==1) && (type == 3))
            {
                xy_value = atoi(cmd_value_strv[4].c_str());
                z_value = atoi(cmd_value_strv[5].c_str());
            }
            if((action==1) && (type == 4))
            {
                xy_value = atoi(cmd_value_strv[4].c_str());
                z_value = atoi(cmd_value_strv[5].c_str());
                zoom_value = atoi(cmd_value_strv[6].c_str());
            }
            //此处分开设置水平、垂直及变倍值是为兼容通过485遵循pelco-d协议设置的方式
            if(action == 1){
                this->set_action(id, type, value, xy_value, z_value, zoom_value);
            }
            else if(action == 0){
                result_value = this->get_action(id, type);
            }
            else if(action == 2){
                if(type == 0){
                    //云台控制:云台右转 0开始 1结束
                    NET_DVR_PTZControl_Other(lUserID, 1, PAN_RIGHT, 0);
                }
                else if(type == 1){
                    NET_DVR_PTZControl_Other(lUserID, 1, PAN_LEFT, 0);
                }
                else if(type == 2){
                    NET_DVR_PTZControl_Other(lUserID, 1, TILT_UP, 0);
                }
                else if(type == 3){
                    NET_DVR_PTZControl_Other(lUserID, 1, TILT_DOWN, 0);
                }
            }
            else if(action == 3){
                NET_DVR_PTZControl_Other(lUserID, 1, PAN_RIGHT, 1);
                NET_DVR_PTZControl_Other(lUserID, 1, PAN_LEFT, 1);
                NET_DVR_PTZControl_Other(lUserID, 1, TILT_UP, 1);
                NET_DVR_PTZControl_Other(lUserID, 1, TILT_DOWN, 1);
            }
            if((action == 0) && (type == 0))
            {
                g_now_xyposition = result_value;
            }
            else if((action == 0) && (type == 1))
            {
                g_now_zposition = result_value;
            }
            else if((action == 0) && (type == 2))
            {
                g_now_zoom = result_value;
            }
        }
        
    }  
}

void hk_sdk_control::read_hk(){
    int z_diff_val = 0, xy_diff_val = 0;
    while(true)
    {
        sleep(0.1);
        if(!ptz_option) continue;
        if(this->_cmd_read_queue.size() < 10)
        {
            this->read_mtx.lock();
            std::string get_xydegree = this->device_id + "/0/0/0";
            this->_cmd_read_queue.push_back(get_xydegree);
            sleep(0.2);
            std::string get_zdegree = this->device_id + "/0/1/0";
            this->_cmd_read_queue.push_back(get_zdegree);
            sleep(0.2);
            this->read_mtx.unlock();
        }

        if((g_action == 1) &&   ((g_control_type == 0) || (g_control_type == 1) || (g_control_type == 3) || (g_control_type == 4)))
        {
            if((g_xy_goal != 0) || (g_z_goal != 0)){
                xy_diff_val = g_xy_goal - g_now_xyposition;
                // std::cout << "g_xy_goal: " << g_xy_goal << std::endl;
                // std::cout << "g_now_xyposition: " << g_now_xyposition << std::endl;
                // std::cout << "xy_val: " << xy_diff_val << std::endl;
                if((xy_diff_val > 35900) || (xy_diff_val < -35900))
                    xy_diff_val = 0;
                z_diff_val = g_z_goal - g_now_zposition;
                // std::cout << "g_z_goal: " << g_z_goal << std::endl;
                // std::cout << "g_now_zposition: " << g_now_zposition << std::endl;
                // std::cout << "z_val: " << z_diff_val << std::endl;
                if((z_diff_val > 35900) || (z_diff_val < -35900))
                    z_diff_val = 0;
                if(((z_diff_val < 100) && (z_diff_val > -100)) && ((xy_diff_val < 100) && (xy_diff_val > -100)))
                {
                    g_action = -1;
                    g_xy_reach_flag = 1;
                    g_z_reach_flag = 1;
                }
            }
        }
    }    
}

bool hk_sdk_control::set_action(int id, int type, int value, int xy_value, int z_value, int zoom_value)
{
    //设置水平、垂直、变倍
    float z_degree = 0.0;
    float xy_degree = 0.0;
    float focus_adjust = 0.0;
    NET_DVR_PTZPOS m_ptzPos;
    if(type == 0){
        m_ptzPos.wAction = 2;
        xy_degree = value;
    }
    else if(type == 1){
        m_ptzPos.wAction = 3;
        z_degree = value;
    }
    else if(type == 2){
        m_ptzPos.wAction = 4;
        focus_adjust = value;   
    }
    else if(type == 3){
        m_ptzPos.wAction = 5;
        xy_degree = xy_value;
        z_degree = z_value;
    }
    else if(type == 4){
        m_ptzPos.wAction = 1;
        xy_degree = xy_value;
        z_degree = z_value;
        focus_adjust = zoom_value;
    }
    int xy_idegree = xy_degree/10;
    int z_idegree = z_degree/10;
    int focus_iadjust = focus_adjust / 100;
    std::cout << "wPanPos:" << xy_idegree << std::endl;
    std::cout << "wTiltPos:" << z_idegree << std::endl;
    std::cout << "wZoomPos:" << focus_iadjust << std::endl;
    //进制转换
    m_ptzPos.wPanPos = DECToHEX(xy_idegree);
    m_ptzPos.wTiltPos = DECToHEX(z_idegree);
    m_ptzPos.wZoomPos = DECToHEX(focus_iadjust);
    std::cout << "wPanPos:" << m_ptzPos.wPanPos << std::endl;
    std::cout << "wTiltPos:" << m_ptzPos.wTiltPos << std::endl;
    std::cout << "wZoomPos:" << m_ptzPos.wZoomPos << std::endl;
    //设置PTZ参数
    if (!NET_DVR_SetDVRConfig(0, NET_DVR_SET_PTZPOS, 0, &m_ptzPos, sizeof(NET_DVR_PTZPOS))){
        std::cout << "set error!" << std::endl;
    }
}

int hk_sdk_control::get_action(int id, int type)
{
    // NET_DVR_PTZPOS 
    // 获取当前水平、垂直及变倍值
    // wAction
    // 操作类型，仅在设置时有效。1-定位PTZ参数，2-定位P参数，3-定位T参数，4-定位Z参数，5-定位PT参数
    // wPanPos
    // P参数（水平参数）
    // wTiltPos
    // T参数（垂直参数）
    // wZoomPos
    // Z参数（变倍参数）
    NET_DVR_PTZPOS m_ptzPosNow;
    DWORD dwOutBufferSize = 1000;
    DWORD lpBytesReturn = 0;
    if(NET_DVR_GetDVRConfig(0, NET_DVR_GET_PTZPOS, 0, &m_ptzPosNow, dwOutBufferSize, &lpBytesReturn))
    {
        //std::cout << "wPanPos:" << HEXToDEC(m_ptzPosNow.wPanPos)*10 << std::endl;
        //std::cout << "wTiltPos:" << (3600-HEXToDEC(m_ptzPosNow.wTiltPos))*10 << std::endl;
        //std::cout << "wZoomPos:" << HEXToDEC(m_ptzPosNow.wZoomPos) << std::endl;
        //std::cout << "lpBytesReturn:" << lpBytesReturn << std::endl;
    }
    if(type == 0){
        return HEXToDEC(m_ptzPosNow.wPanPos)*10;
    }
    else if(type == 1){
        //return (3600-HEXToDEC(m_ptzPosNow.wTiltPos))*10;
        return HEXToDEC(m_ptzPosNow.wTiltPos)*10;
    }
    else if(type == 2){
        return HEXToDEC(m_ptzPosNow.wZoomPos);   
    }

    return 0;
}

void hk_sdk_control::read_raw_temp(){
    //获取全图温度
    NET_DVR_JPEGPICTURE_WITH_APPENDDATA struJpegWithAppendData = { 0 };
    int i = sizeof(NET_DVR_JPEGPICTURE_WITH_APPENDDATA);
    const int ciPictureBufSize = 2 * 1024 * 1024;//2M
    const int ciVisPictureBufSize = 4 * 1024 * 1024;//2M
    char* ucJpegBuf = new char[ciPictureBufSize];
    char* ucAppendDataBuf = new char[ciPictureBufSize];
    char* ucvisJpegBuf = new char[ciVisPictureBufSize];

    memset(ucJpegBuf, 0, ciPictureBufSize);
    memset(ucAppendDataBuf, 0, ciPictureBufSize);
    memset(ucvisJpegBuf, 0, ciVisPictureBufSize);
    
    struJpegWithAppendData.pJpegPicBuff = ucJpegBuf;
    struJpegWithAppendData.pP2PDataBuff = ucAppendDataBuf;

    while(pub_raw_temp)
    {
        //获取热图的SDK接口
        if (!NET_DVR_CaptureJPEGPicture_WithAppendData(lUserID, infrared_channel, &struJpegWithAppendData))
        {
            std::cout << "NET_DVR_CaptureJPEGPicture_WithAppendData get hot picture failed!" << std::endl;
            continue;
        }
        //判断抓图数据是否正确
        if (struJpegWithAppendData.dwP2PDataLen != 4 * struJpegWithAppendData.dwJpegPicWidth * struJpegWithAppendData.dwJpegPicHeight)
        {
            std::cout << "NET_DVR_CaptureJPEGPicture_WithAppendData get data error!" << std::endl;
            continue;
        }

        char* g_Buffer = struJpegWithAppendData.pP2PDataBuff;
        unsigned int g_len = struJpegWithAppendData.dwP2PDataLen;

        vector<unsigned char> vc(struJpegWithAppendData.pP2PDataBuff, struJpegWithAppendData.pP2PDataBuff+struJpegWithAppendData.dwP2PDataLen);
        sensor_msgs::Image msg;
        msg.header.stamp = ros::Time::now();
        msg.height = struJpegWithAppendData.dwJpegPicHeight;
        msg.width = struJpegWithAppendData.dwJpegPicWidth;
        msg.data = vc;
        msg.step = struJpegWithAppendData.dwP2PDataLen;
        this->raw_temp_pub_.publish(msg);

        /*int iIndex = 0;
        FILE* fp = NULL;

        float minTemp = 10000.0;
        float maxTemp = -10000.0;

        //std::cout << "struJpegWithAppendData.dwJpegPicHeight:" << struJpegWithAppendData.dwJpegPicHeight << std::endl;
        //std::cout << "struJpegWithAppendData.dwJpegPicWidth: " << struJpegWithAppendData.dwJpegPicWidth << std::endl;

        for (int iWriteHeight = 0; iWriteHeight < struJpegWithAppendData.dwJpegPicHeight; ++iWriteHeight)
        {
            for (int iWriteWidth = 0; iWriteWidth < struJpegWithAppendData.dwJpegPicWidth; ++iWriteWidth)
            {
                float fTemp = *((float*)(struJpegWithAppendData.pP2PDataBuff + iIndex));

                //判断fTemp是否是一个正常值，不是则赋值最大或最小，防止设备崩溃
                fTemp = (9999 < fTemp) ? 9999 : ((-9999 > fTemp) ? -9999 : fTemp);

                minTemp = (minTemp > fTemp) ? fTemp : minTemp;
                maxTemp = (maxTemp > fTemp) ? maxTemp : fTemp;

                printf("iWriteHeight,iWriteWidth:[%d, %d], [%.2f] \n", iWriteHeight, iWriteWidth, fTemp);

                iIndex += 4;
            }
        }*/
    }
    delete ucJpegBuf;
    delete ucAppendDataBuf;
    delete ucvisJpegBuf;
}

void hk_sdk_control::set_focusmode()
{
    NET_DVR_FOCUSMODE_CFG m_setfocusmode = {0};
    DWORD dwOutBufferSize = 1000;
    DWORD lpBytesReturn = 0;
    if(NET_DVR_GetDVRConfig(lUserID, NET_DVR_GET_FOCUSMODECFG, 2, &m_setfocusmode, dwOutBufferSize, &lpBytesReturn))
    {
        std::cout << "m_setfocusmode.byFocusMode: " << (int)m_setfocusmode.byFocusMode << std::endl;
    }
    m_setfocusmode.byFocusMode = 2;
    //设置红外聚焦模式为半自动
    if (!NET_DVR_SetDVRConfig(lUserID, NET_DVR_SET_FOCUSMODECFG, 2, &m_setfocusmode, sizeof(NET_DVR_FOCUSMODE_CFG))){
        std::cout << "set focus mode error" << std::endl;
    }
    else{
        std::cout << "set focus mode success!" << std::endl;
    }
    sleep(this->_set_infrared_focus_waittime);
    m_setfocusmode.byFocusMode = 1;
    //设置红外聚焦模式为手动
    if (!NET_DVR_SetDVRConfig(lUserID, NET_DVR_SET_FOCUSMODECFG, 2, &m_setfocusmode, sizeof(NET_DVR_FOCUSMODE_CFG))){
        std::cout << "set focus mode success!" << std::endl;
    }
    else{
        std::cout << "set focus mode success!" << std::endl;
    }
}

bool hk_sdk_control::handle_cloudplatform(fixed_msg::cp_control::Request &req, fixed_msg::cp_control::Response &res)
{
    this->write_mtx.lock();
    std::stringstream ss;
	ss.clear();
	ss.str("");
	ss << req.id << "/" << req.action << "/" << req.type << "/" << req.value;
    if(req.type == 3){
        ss << "/" << req.allvalue[0] << "/" << req.allvalue[1];
    }
    if(req.type == 4){
        std::cout << "1111111111111" << std::endl;
        ss << "/" << req.allvalue[0] << "/" << req.allvalue[1] << "/" << req.allvalue[2];
    }
    std::string s_cmd = ss.str();
    std::cout << s_cmd << std::endl;
    _cmd_control_queue.push_back(s_cmd);
    this->write_mtx.unlock();
	return true;
}

void hk_sdk_control::detect_rect_callback(const fixed_msg::detect_result &msg)
{
    if(!auto_zoom) return;
    //设置要进行ptz区域
    NET_DVR_POINT_FRAME  posdata;  
    posdata.xTop = (int)(msg.xmin * 255 / 1920 );   
    // (int)((target_tmp.x + target_tmp.width) / std_cols * 255); 
    int diff_x_value = (int)(msg.xmax - msg.xmin) * 255 / 1920;     
    if(diff_x_value > 2){
        posdata.xBottom = posdata.xTop + diff_x_value;
    }
    else{
        posdata.xBottom = posdata.xTop + 2;
    }       
    posdata.yTop = (int)(msg.ymin * 255 / 1080);
    // (int)((target_tmp.y + target_tmp.height) / std_rows * 255); 
    int diff_y_value = (int)(msg.ymax - msg.ymin) * 255 / 1080;
    if(diff_y_value > 2){
        posdata.yBottom = posdata.yTop + diff_y_value;
    }
    else{
        posdata.yBottom = posdata.yTop + 2;
    }   
    posdata.bCounter = 1;    
    if (!NET_DVR_PTZSelZoomIn_EX(0, 1, &posdata)){
        std::cout << "set pzt zoom failed！" << std::endl;
    }
    else{
        std::cout << "set ptz zoom success!" << std::endl;
    }

}