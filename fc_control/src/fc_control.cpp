#include "fc_control/fc_control.hpp"

fc_control::fc_control(const ros::NodeHandle &nh):nh_(nh)
{
    //load action
    std::unique_ptr<fsm::manager> m(new fsm::manager());
    brain_ = std::move(m);
    std::shared_ptr<fsm::action_idle> action_idle_ptr(new fsm::action_idle("idle"));
    brain_->add_state(action_idle_ptr);
    actions_[fsm::action_enum::IDLE] = "idle";
    std::shared_ptr<fsm::action_coloring> action_coloring_ptr(new fsm::action_coloring("coloring"));
    brain_->add_state(action_coloring_ptr);
    actions_[fsm::action_enum::COLORRING] = "coloring";
    std::shared_ptr<fsm::action_mapping> action_mapping_ptr(new fsm::action_mapping("mapping"));
    brain_->add_state(action_mapping_ptr);
    actions_[fsm::action_enum::MAPPING] = "mapping";
    brain_->set_state(actions_[fsm::action_enum::IDLE]);

    load_params();
    launch_client();
    launch_server();
    launch_subscribers();
    launch_publishs();
}

fc_control::~fc_control()
{
}

void fc_control::tick(const ros::TimerEvent &event){
    LOG(INFO) << "fc control node tick per second";
}
void fc_control::update(){
    if(brain_)
        brain_->loop();
}
void fc_control::load_params(){
    std::string mapping_file;
    //nh_.param<int>("task_timeout", task_timeout, 60);
    nh_.param<std::string>("mapping_file", mapping_file, "");
    option_.log_path = "";
    option_.mapping_file = mapping_file;
}
void fc_control::launch_client(){}
void fc_control::launch_server(){
    // task_srv = nh_.advertiseService("/fixed/map/task_start",  &fc_control::web_task, this);//接收建图参数建图任务
    // task_control_srv = nh_.advertiseService("/fixed/map/task_control",  &fc_control::web_taskcontrol, this);//控制
}
void fc_control::launch_subscribers(){}
void fc_control::launch_publishs(){}