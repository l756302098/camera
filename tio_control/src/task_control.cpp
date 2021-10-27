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
        task_context.device_id = 0;
        task_context.task_id = task_id;
        task_context.map_id = 0;
        task_context.resolution_flag = 0;
    }
}

void task_control::deal_callback(int module,int code,std::string msg,float percent){
    //任务状态 0:正常结束\1:终止\2:暂停\3:正在执行\4:未执行\5:超期\6:预执行\7:超时
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
            reset();
            break;
        case fsm::mapping_error_enum::MAP_QUIT:
            LOG(INFO) << "mapping quit ok.";
            task_context.task_status = 1;
            break;
        default:
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
            reset();
            break;
        case fsm::coloring_error_enum::COR_EXIT:
            LOG(INFO) << "coloring quit ok.";
            task_context.task_status = 1;
            break;
        default:
            LOG(ERROR) << "coloring fatal error occurred.";
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
}
void task_control::loop(const ros::TimerEvent &event){
    //LOG(WARNING) << "tio control node tick per second"
    if(brain_)
        brain_->loop();
}
void task_control::update(){
    
}
void task_control::load_params(){
    std::string mapping_file,coloring_finish_topic,test_coloring_file;
    int color_weight,motor_timeout,collect_timeout,mapping_color_timeout,coloring_timeout;
    nh_.param<int>("color_weight", color_weight, 60);
    nh_.param<int>("motor_timeout", motor_timeout, 60);
    nh_.param<int>("collect_timeout", collect_timeout, 60);
    nh_.param<int>("mapping_color_timeout", mapping_color_timeout, 60);
    nh_.param<int>("coloring_timeout", coloring_timeout, 60);
    nh_.param<std::string>("mapping_file", mapping_file, "");
    nh_.param<std::string>("coloring_finish_topic", coloring_finish_topic, "");
    nh_.param<std::string>("test_coloring_file", test_coloring_file, "");
    option_.log_path = "";
    option_.mapping_file = mapping_file;
    option_.color_weight = color_weight;
    option_.motor_timeout = motor_timeout;
    option_.collect_timeout = collect_timeout;
    option_.mapping_color_timeout = mapping_color_timeout;
    option_.coloring_timeout = coloring_timeout;
    option_.test_coloring_file = test_coloring_file;
    LOG(WARNING) << "mapping_file:" << mapping_file.c_str();
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
void task_control::launch_subscribers(){}
void task_control::launch_publishs(){
    task_status_pub = nh_.advertise<tio_control::task_status>("/tio/control/task_status", 1);
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
        auto f = std::bind(&task_control::deal_callback,this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
        brain_->get_current()->set_callback(f);
        bool ok = brain_->get_current()->play(fsm::mapping_enum::START,data);
        if(!ok){
            LOG(ERROR) <<  "mapping_task_cb request failed . mapping server no response";
            res.success = false;
            res.message = "mapping_task_cb request failed . mapping server no response";
            return false;
        }else{
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