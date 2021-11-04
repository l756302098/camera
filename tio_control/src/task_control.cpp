#include "tio_control/task_control.hpp"

NodeOption task_control::option_;
task_control::task_control(const ros::NodeHandle &nh):nh_(nh)
{
    load_params();
    //load action
    std::unique_ptr<fsm::manager> m(new fsm::manager());
    brain_ = std::move(m);
    std::shared_ptr<fsm::action_idle> action_idle_ptr(new fsm::action_idle());
    brain_->add_state(action_idle_ptr);
    std::shared_ptr<fsm::action_coloring> action_coloring_ptr(new fsm::action_coloring());
    brain_->add_state(action_coloring_ptr);
    std::shared_ptr<fsm::action_mapping> action_mapping_ptr(new fsm::action_mapping());
    brain_->add_state(action_mapping_ptr);
    std::shared_ptr<fsm::action_upload> action_upload_ptr(new fsm::action_upload());
    brain_->add_state(action_upload_ptr);
    brain_->set_state(fsm::fsm_enum::IDLE);
    reset();
    launch_client();
    launch_server();
    launch_subscribers();
    launch_publishs();
    if(!option_.test_coloring_file.empty()){
        int task_id = load_task_file(option_.test_coloring_file);
        brain_->set_state(fsm::fsm_enum::COLORRING);
        std::vector<std::string> data;
        auto f = std::bind(&task_control::deal_callback,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        brain_->get_current()->set_callback(f);
        bool ok = brain_->get_current()->play(fsm::coloring_enum::COR_START,data);
        task_context.task_id = task_id;
        task_context.map_id = 0;
        task_context.resolution_flag = 0;
    }
}

void task_control::deal_callback(int module,int code,std::string msg,float percent){
    //任务状态 0:正常结束\1:终止\2:暂停\3:正在执行\4:未执行\5:超期\6:预执行\7:超时
    LOG(WARNING) << "task_control::deal_callback " << module << " " << code << " " << msg << " " << percent;
    task_context.task_name = task_name();
    task_context.task_progress = percent;
    task_context.task_code = code;
    task_context.message = msg;
    if(module == fsm::fsm_enum::MAPPING){
        switch (code)
        {
        case fsm::mapping_error_enum::NORMAL:
            LOG(INFO) << "mapping ok";
            task_context.task_status = 3;
            break;
        case fsm::mapping_error_enum::MAP_FINISH:
            LOG(INFO) << "mapping finish.";
            task_context.task_status = 0;
            break;
        case fsm::mapping_error_enum::MAP_QUIT:
            LOG(INFO) << "mapping quit ok.";
            task_context.task_status = 1;
            break;
        default:
            task_context.task_status = 1;
            LOG(ERROR) << "mapping fatal error occurred.";
            break;
        }
    }else if(module == fsm::fsm_enum::COLORRING){
        switch (code)
        {
        case fsm::coloring_error_enum::COR_NORMAL:
            LOG(INFO) << "coloring ok";
            task_context.task_status = 3;
            break;
        case fsm::coloring_error_enum::COR_FINISH:
            LOG(INFO) << "coloring finish.";
            task_context.task_status = 0;
            break;
        case fsm::coloring_error_enum::COR_EXIT:
            LOG(INFO) << "coloring quit ok.";
            task_context.task_status = 1;
            break;
        default:
            task_context.task_status = 1;
            LOG(ERROR) << "coloring fatal error occurred.";
            break;
        }
    }else if(module == (int)fsm::fsm_enum::UPLOAD){
        switch (code)
        {
        case (int)fsm::upload_error_enum::NORMAL:
            LOG(INFO) << "upload ok";
            task_context.task_status = 3;
            break;
        case (int)fsm::upload_error_enum::FINISH:
            LOG(INFO) << "upload finish.";
            task_context.task_status = 0;
            break;
        case (int)fsm::upload_error_enum::QUIT:
            LOG(INFO) << "upload quit ok.";
            task_context.task_status = 1;
            break;
        default:
            task_context.task_status = 1;
            LOG(ERROR) << "upload fatal error occurred.";
            break;
        }
    }
}
void task_control::tick(const ros::TimerEvent &event){
    //LOG(WARNING) << "tio control node tick per second"
    tio_control::task_status tt;
    tt.device_id = task_context.device_id;
    tt.task_id = task_context.task_id;
    tt.map_id = task_context.map_id;
    tt.task_name = task_context.task_name;
    tt.task_code = task_context.task_code;
    tt.task_progress = task_context.task_progress;
    tt.task_status = task_context.task_status;
    tt.message = task_context.message;
    task_status_pub.publish(tt);
    pub_status_web();
    pub_progress_web();
}

void task_control::pub_status_web(){
    rapidjson::Document doc;
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator(); 
    Value data(kObjectType);
    data.AddMember("robotId", task_context.device_id, allocator);
	data.AddMember("taskHistoryId", task_context.task_id, allocator);
	data.AddMember("taskId", task_context.task_id, allocator);
	data.AddMember("taskStatus", task_context.task_status, allocator);
    rapidjson::Value message_value;
    message_value.SetString(StringRef(task_context.message.c_str()));
	data.AddMember("messgae", message_value, allocator);
    doc.SetObject();
	doc.AddMember("logId", "1", allocator);
	rapidjson::Value gatewayId,timestamp;
    char buffer[30];
    int len = sprintf(buffer, "%i", task_context.device_id);
    gatewayId.SetString(buffer, len, allocator);
    memset(buffer, 0, sizeof(buffer));
	doc.AddMember("gatewayId", gatewayId, allocator);
    len = sprintf(buffer, "%i", ros::WallTime::now().sec);
    timestamp.SetString(buffer, len, allocator);
    memset(buffer, 0, sizeof(buffer));
	doc.AddMember("timestamp", timestamp, allocator);
    doc.AddMember("type", 0, allocator);
	doc.AddMember("version", "v1.0", allocator);
    doc.AddMember("data", data, allocator);

	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	doc.Accept(writer);
    //LOG(INFO) <<  "json_str:"<<json_str;
	std_msgs::String msg;
    msg.data = std::string(s.GetString());
	web_status_pub.publish(msg);
}

void task_control::pub_progress_web(){
    rapidjson::Document doc;
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator(); 
	doc.SetObject();
    Value data(kObjectType);
	data.AddMember("robotId", task_context.device_id, allocator);
	data.AddMember("taskHistoryId", task_context.task_id, allocator);
	data.AddMember("taskId", task_context.task_id, allocator);
	data.AddMember("taskProgress", task_context.task_progress, allocator);
	doc.AddMember("logId", "1", allocator);
    rapidjson::Value gatewayId,timestamp;
    char buffer[30];
    int len = sprintf(buffer, "%i", task_context.device_id);
    gatewayId.SetString(buffer, len, allocator);
    memset(buffer, 0, sizeof(buffer));
	doc.AddMember("gatewayId", gatewayId, allocator);
    len = sprintf(buffer, "%i", ros::WallTime::now().sec);
    timestamp.SetString(buffer, len, allocator);
    memset(buffer, 0, sizeof(buffer));
	doc.AddMember("timestamp", timestamp, allocator);
    doc.AddMember("type", 0, allocator);
	doc.AddMember("version", "v1.0", allocator);
    doc.AddMember("data", data, allocator);

	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	doc.Accept(writer);
    //LOG(INFO) <<  "json_str:"<<json_str;
	std_msgs::String msg;
    msg.data = std::string(s.GetString());
	web_progress_pub.publish(msg);
}

void task_control::loop(const ros::TimerEvent &event){
    //LOG(WARNING) << "tio control node tick per second"
    if(brain_)
        brain_->loop();
}
void task_control::update(){
    
}
void task_control::load_params(){
    std::string mapping_file,coloring_finish_topic,test_coloring_file,map_save_path;
    int device_id,color_weight,motor_timeout,collect_timeout,mapping_color_timeout,coloring_timeout,upload_timeout;
    nh_.param<int>("device_id", device_id, 1);
    nh_.param<int>("color_weight", color_weight, 60);
    nh_.param<int>("motor_timeout", motor_timeout, 60);
    nh_.param<int>("collect_timeout", collect_timeout, 60);
    nh_.param<int>("mapping_color_timeout", mapping_color_timeout, 60);
    nh_.param<int>("coloring_timeout", coloring_timeout, 60);
    nh_.param<int>("upload_timeout", upload_timeout, 60);
    nh_.param<std::string>("mapping_file", mapping_file, "");
    nh_.param<std::string>("map_save_path", map_save_path, "");
    nh_.param<std::string>("coloring_finish_topic", coloring_finish_topic, "");
    nh_.param<std::string>("test_coloring_file", test_coloring_file, "");
    option_.log_path = "";
    option_.mapping_file = mapping_file;
    option_.map_save_path = map_save_path;
    option_.color_weight = color_weight;
    option_.motor_timeout = motor_timeout;
    option_.collect_timeout = collect_timeout;
    option_.mapping_color_timeout = mapping_color_timeout;
    option_.coloring_timeout = coloring_timeout;
    option_.test_coloring_file = test_coloring_file;
    option_.upload_timeout = upload_timeout;
    LOG(WARNING) << "mapping_file:" << mapping_file.c_str();
    task_context.device_id = device_id;
    //load json
    load_from_file(option_.mapping_file);
}
void task_control::launch_client(){}
void task_control::launch_server(){
    //coloring
    coloring_srv = nh_.advertiseService("/tio/control/coloring_start",  &task_control::coloring_task_cb, this);
    control_coloring_srv = nh_.advertiseService("/tio/control/coloring_control",  &task_control::control_coloring_cb, this);
    //mapping
    mapping_srv = nh_.advertiseService("/tio/control/mapping_start",  &task_control::mapping_task_cb, this);
    control_mapping_srv = nh_.advertiseService("/tio/control/mapping_control",  &task_control::control_mapping_cb, this);
}
void task_control::launch_subscribers(){
    web_data_sub = nh_.subscribe("/tio/task/data/down", 1, &task_control::web_data_cb,this);
    web_command_sub = nh_.subscribe("/tio/task/command/down", 1, &task_control::web_command_cb,this);
}
void task_control::launch_publishs(){
    task_status_pub = nh_.advertise<tio_control::task_status>("/tio/control/task_status", 1);
    web_status_pub = nh_.advertise<std_msgs::String>("/tio/control/web_status", 1);
    web_progress_pub = nh_.advertise<std_msgs::String>("/tio/control/web_progress", 1);
}
bool task_control::mapping_task_cb(tio_control::mapping_task::Request &req,tio_control::mapping_task::Response &res){
    LOG(INFO)   <<"mapping_task_cb recieve request web_data: "
                <<" device_id is "<< req.device_id
                <<" task_id is "<<req.task_id
                <<" map_id is "<<req.map_id
                <<" resolution is "<<req.flag;
    if(task_context.task_id != 0){
        res.success = false;
        res.message = "last task is not over yet";
        LOG(ERROR) << "last task is not over yet" ;
        return false;
    }
    if(req.flag>=0 && req.flag<3){
        LOG(INFO) << "mapping_task_cb start mapping action" ;
        brain_->set_state(fsm::fsm_enum::MAPPING);
        std::vector<std::string> data;
        data.push_back(std::to_string(req.device_id));
        data.push_back(std::to_string(req.task_id));
        data.push_back(std::to_string(req.map_id));
        data.push_back(std::to_string(req.flag));
        bool ok = brain_->get_current()->play(fsm::mapping_enum::START,data);
        if(!ok){
            LOG(ERROR) <<  "mapping_task_cb request failed . mapping server no response";
            res.success = false;
            res.message = "mapping_task_cb request failed . mapping server no response";
            return false;
        }else{
            auto f = std::bind(&task_control::deal_callback,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
            brain_->get_current()->set_callback(f);
            task_context.task_status = 3;
            task_context.device_id = req.device_id;
            task_context.task_id = req.task_id;
            task_context.map_id = req.map_id;
            task_context.resolution_flag = req.flag;
        }
    }
    else{
        LOG(ERROR) << "web_task request flag not valid, flag = " << req.flag;
        res.success =false;
        res.message = "web_task request flag not valid";
        return false;
    }
    LOG(INFO) << "web_task request success" ;
    res.success = true;
    res.message = "ok";
    return true;
}
bool task_control::control_mapping_cb(tio_control::control_task::Request &req,tio_control::control_task::Response &res){
    LOG(INFO)   <<"control_mapping_cb recieve request web_data: "
                <<" device_id is "<< req.device_id
                <<" task_id is "<<req.task_id
                <<" flag is "<<req.flag;
    if(task_context.task_id!= 0 && task_context.task_id !=req.task_id){
        LOG(ERROR) << "coloring_task_cb request task_id not valid, current task_id = " << task_context.task_id << " request task_id:" << req.task_id;
        res.success =false;
        res.message = "coloring_task_cb request task_id not valid.";
        return false;
    }
    std::vector<std::string> data;
    if(req.flag==1)
    {
        LOG(INFO)   <<"Request PAUSE";
        brain_->get_current()->play(fsm::mapping_enum::PAUSE,data);
    }
    else if(req.flag==2)
    {
        LOG(INFO)   <<"Request QUTI";
        bool ok = brain_->get_current()->play(fsm::mapping_enum::QUIT,data);
        if(!ok){
            LOG(ERROR) <<  "web_task_control request QUTI failed . mapping server no response";
            res.success = false;
            res.message = "web_task_control request QUTI failed . mapping server no response";
            return false;
        }else{
            reset();
            brain_->set_state(fsm::fsm_enum::IDLE);
        }
    }
    else if(req.flag==3)
    {
        LOG(INFO)   <<"Request GO ON";
        brain_->get_current()->play(fsm::mapping_enum::GOON,data);
    }else{
        LOG(ERROR) << "web_task request flag not valid, flag = " << req.flag;
        res.success =false;
        res.message = "web_task_control request flag not valid";
        return false;
    }
    LOG(INFO) << "web_task_control request success" ;
    res.success = true;
    res.message = "ok";
    return true;
}
bool task_control::coloring_task_cb(tio_control::coloring_task::Request &req,tio_control::coloring_task::Response &res){
    LOG(INFO)   <<"coloring_task_cb recieve request: "
                <<" device_id is "<< req.device_id
                <<" task_id is "<<req.task_id
                <<" plan is "<<req.plan;
    if(task_context.task_id != 0){
        res.success = false;
        res.message = "last task is not over yet";
        LOG(ERROR) << "last task is not over yet" ;
        return false;
    }
    std::string list = req.plan;
	std::cout << req.plan << std::endl;
    //std::ifstream in(req.plan,ios::in|ios::binary);
    int task_id = load_from_stream(list);
    if(task_id==-1){
        res.message = "resolve json failed";
        res.success = false;
    }else{
        brain_->set_state(fsm::fsm_enum::COLORRING);
        std::vector<std::string> data;
        auto f = std::bind(&task_control::deal_callback,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        brain_->get_current()->set_callback(f);
        bool ok = brain_->get_current()->play(fsm::coloring_enum::COR_START,data);
        task_context.device_id = 0;
        task_context.task_id = task_id;
        task_context.map_id = 0;
        task_context.resolution_flag = 0;
        res.success = true;
        res.message = "ok";
        return true;
    }
}
bool task_control::control_coloring_cb(tio_control::control_task::Request &req,tio_control::control_task::Response &res){
    LOG(INFO)   <<"control_coloring_cb recieve request web_data: "
                <<" device_id is "<< req.device_id
                <<" task_id is "<<req.task_id
                <<" flag is "<<req.flag;
    if(task_context.task_id!= 0 && task_context.task_id !=req.task_id){
        LOG(ERROR) << "coloring_task_cb request task_id not valid, current task_id = " << task_context.task_id << " request task_id:" << req.task_id;
        res.success =false;
        res.message = "coloring_task_cb request task_id not valid.";
        return false;
    }
    std::vector<std::string> data;
    if(req.flag==1)
    {
        LOG(INFO)   <<"Request PAUSE";
        brain_->get_current()->play(fsm::coloring_enum::COR_PAUSE,data);
    }
    else if(req.flag==2)
    {
        LOG(INFO)   <<"Request QUTI";
        bool ok = brain_->get_current()->play(fsm::coloring_enum::COR_QUIT,data);
        if(!ok){
            LOG(ERROR) <<  "web_task_control request QUTI failed . mapping server no response";
            res.success = false;
            res.message = "web_task_control request QUTI failed . mapping server no response";
            return false;
        }else{
            reset();
            brain_->set_state(fsm::fsm_enum::IDLE);
        }
    }
    else if(req.flag==3)
    {
        LOG(INFO)   <<"Request GO ON";
        brain_->get_current()->play(fsm::coloring_enum::COR_GOON,data);
    }else{
        LOG(ERROR) << "web_task request flag not valid, flag = " << req.flag;
        res.success =false;
        res.message = "web_task_control request flag not valid";
        return false;
    }
    LOG(INFO) << "web_task_control request success" ;
    res.success = true;
    res.message = "ok";
    return true;
}

void task_control::web_data_cb(const std_msgs::String::ConstPtr& msg){
    LOG(INFO)   <<"task_control::web_down_cb " << msg->data;
    try
    {
        std::shared_ptr<TaskData> td = resolve_task_data(msg->data.c_str());
        if(td){
            reset();
            if(td->type == 1){
                std::shared_ptr<ColorJson> color = resolve_color_json(td->taskData.c_str());
                if(color){
                    task_control::option_.cj = color;
                    brain_->set_state(fsm::fsm_enum::COLORRING);
                    std::vector<std::string> data;
                    data.push_back(std::to_string(color->stationMapId));
                    bool ok = brain_->get_current()->play(fsm::coloring_enum::COR_START,data);
                    if(ok){
                        auto f = std::bind(&task_control::deal_callback,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
                        brain_->get_current()->set_callback(f);
                        task_context.device_id = color->RobotId;
                        task_context.task_id = color->TaskHistoryId;
                        task_context.map_id = color->stationMapId;
                        task_context.task_status = 3;
                    }else{
                        LOG(ERROR) <<  "coloring play failed.";
                        return;
                    }
                }else{
                    LOG(ERROR) << "resolve_color_json return null ptr";
                }
            }
            else if(td->type == 3){
                std::shared_ptr<MapJson> map = resolve_map_json(td->taskData.c_str());
                //check device id
                int resolution_flag = atoi(map->density.c_str());
                if(map->RobotId != task_context.device_id){
                    LOG(ERROR) << "current device_id is " << task_context.device_id << " RobotId is " <<map->RobotId;
                    return;
                }
                if(map){
                    //start map task
                    brain_->set_state(fsm::fsm_enum::MAPPING);
                    std::vector<std::string> data;
                    data.push_back(std::to_string(task_context.device_id));
                    data.push_back(std::to_string(map->TaskHistoryId));
                    data.push_back(std::to_string(map->stationMapId));
                    data.push_back(map->density);
                    bool ok = brain_->get_current()->play(fsm::mapping_enum::START,data);
                    if(!ok){
                        LOG(ERROR) <<  "mapping play failed.";
                        return;
                    }else{
                        auto f = std::bind(&task_control::deal_callback,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
                        brain_->get_current()->set_callback(f);
                        task_context.task_status = 3;
                        task_context.device_id = task_context.device_id;
                        task_context.task_id = map->TaskHistoryId;
                        task_context.map_id = map->stationMapId;
                        task_context.resolution_flag = resolution_flag;
                    }
                }else{
                    LOG(ERROR) << "resolve_task_json return null ptr";
                }
            }else if(td->type == 4){
                std::shared_ptr<UploadMapJson> umj = resolve_upload_map_json(td->taskData.c_str());
                if(umj->RobotId != task_context.device_id){
                    LOG(ERROR) << "current device_id is " << task_context.device_id << " RobotId is " <<umj->RobotId;
                    return;
                }
                if(umj){
                    //get file
                    int map_id = umj->stationMapId;
                    std::string search_path = option_.map_save_path + "/" + std::to_string(map_id) + "/ply/";
                    std::string map_file;
                    bool ok = readNewFile(search_path,map_file,".ply");
                    if(!ok){
                        LOG(ERROR) << "search map failed in " << search_path;
                    }
                    std::string upload_file = search_path + map_file;
                    LOG(INFO) << "search map success in " << upload_file;
                    brain_->set_state(fsm::fsm_enum::UPLOAD);
                    std::vector<std::string> data;
                    data.push_back(std::to_string(umj->TaskHistoryId));
                    data.push_back(std::to_string(umj->stationMapId));
                    data.push_back(map_file);
                    data.push_back(upload_file);
                    ok = brain_->get_current()->play((int)fsm::upload_enum::START,data);
                    if(ok){
                        auto f = std::bind(&task_control::deal_callback,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
                        brain_->get_current()->set_callback(f);
                        task_context.task_status = 3;
                        task_context.device_id = task_context.device_id;
                        task_context.task_id = umj->TaskHistoryId;
                        task_context.map_id = umj->stationMapId;
                    }else{
                        LOG(ERROR) <<  "upload play failed.";
                    }
                }else{
                    LOG(ERROR) << "resolve_upload_map_json return null ptr";
                }
            }
        }else{
            LOG(ERROR) << "resolve_task_data return null ptr";
        }
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
    }
}

void task_control::web_command_cb(const std_msgs::String::ConstPtr& msg){
    LOG(INFO)   <<"task_control::web_command_cb " << msg->data;
    try
    {
        std::shared_ptr<TaskCommand> tc = resolve_task_command(msg->data.c_str());
        if(tc){
            if(tc->robotId != task_context.device_id){
                LOG(ERROR) << "current device_id is " << task_context.device_id << " RobotId is " <<tc->robotId;
                return;
            }
            if(tc->taskHistoryId != task_context.task_id && task_context.task_id != 0){
                LOG(ERROR) << "current task_id is " << task_context.task_id << " taskHistoryId is " <<tc->taskHistoryId;
                return;
            }
            switch (brain_->get_current()->status)
            {
                case fsm::fsm_enum::COLORRING:
                    control_coloring(tc->flag);
                    break;
                case fsm::fsm_enum::MAPPING:
                    control_mapping(tc->flag);
                    break;
                case fsm::fsm_enum::UPLOAD:
                    control_mapping(tc->flag);
                    break;
                default:
                    break;
            }
        }else{
            LOG(ERROR) << "resolve_task_command return null ptr";
        }
    }
    catch(const std::exception& e)
    {
        LOG(ERROR) << e.what();
    }
}

bool task_control::control_mapping(int flag){
    LOG(INFO)   <<"task_control::control_mapping " << flag;
    std::vector<std::string> data;
    if(flag==1)
    {
        LOG(INFO)   <<"Request PAUSE";
        brain_->get_current()->play(fsm::mapping_enum::PAUSE,data);
    }
    else if(flag==2)
    {
        LOG(INFO)   <<"Request QUTI";
        bool ok = brain_->get_current()->play(fsm::mapping_enum::QUIT,data);
        if(!ok){
            LOG(ERROR) <<  "web_task_control request QUTI failed . mapping server no response";
            return false;
        }else{
            reset();
            brain_->set_state(fsm::fsm_enum::IDLE);
        }
    }
    else if(flag==3)
    {
        LOG(INFO)   <<"Request GO ON";
        brain_->get_current()->play(fsm::mapping_enum::GOON,data);
    }else{
        LOG(ERROR) << "flag not valid, flag = " << flag;
        return false;
    }
    return true;
}
bool task_control::control_coloring(int flag){
    LOG(INFO)   <<"task_control::control_coloring " << flag;
    std::vector<std::string> data;
    if(flag==1)
    {
        LOG(INFO)   <<"Request PAUSE";
        brain_->get_current()->play(fsm::coloring_enum::COR_PAUSE,data);
    }
    else if(flag==2)
    {
        LOG(INFO)   <<"Request QUTI";
        bool ok = brain_->get_current()->play(fsm::coloring_enum::COR_QUIT,data);
        if(!ok){
            LOG(ERROR) <<  "web_task_control request QUTI failed . mapping server no response";
            return false;
        }else{
            reset();
            brain_->set_state(fsm::fsm_enum::IDLE);
        }
    }
    else if(flag==3)
    {
        LOG(INFO)   <<"Request GO ON";
        brain_->get_current()->play(fsm::coloring_enum::COR_GOON,data);
    }else{
        LOG(ERROR) << "flag not valid, flag = " << flag;
        return false;
    }
    return true;
}

bool task_control::control_upload(int flag){
    LOG(INFO)   <<"task_control::control_upload " << flag;
    std::vector<std::string> data;
    if(flag==1)
    {
        LOG(INFO)   <<"Request PAUSE";
        brain_->get_current()->play((int)fsm::upload_enum::PAUSE,data);
    }
    else if(flag==2)
    {
        LOG(INFO)   <<"Request QUTI";
        bool ok = brain_->get_current()->play((int)fsm::upload_enum::QUIT,data);
        if(!ok){
            LOG(ERROR) <<  "web_task_control request QUTI failed . ftp server no response";
            return false;
        }else{
            reset();
            brain_->set_state(fsm::fsm_enum::IDLE);
        }
    }
    else if(flag==3)
    {
        LOG(INFO)   <<"Request GO ON";
        brain_->get_current()->play((int)fsm::upload_enum::GOON,data);
    }else{
        LOG(ERROR) << "flag not valid, flag = " << flag;
        return false;
    }
    return true;
}