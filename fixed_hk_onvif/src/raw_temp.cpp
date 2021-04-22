/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-21 10:18:49
 * @LastEditors: li
 * @LastEditTime: 2021-04-21 17:06:17
 */
#include "fixed_hk_onvif/raw_temp.hpp"

raw_temp::raw_temp(const ros::NodeHandle &nh):nh_(nh)
{
}

raw_temp::~raw_temp()
{
}

void raw_temp::update(){
    
}

int raw_temp::get_url(std::string& url){
    try
    {
        std::string userpwd = "admin:abcd1234";
        std::string auth = rtsptool::base64_encode(userpwd);
        httplib::Headers headers = {{ "Authorization", "Basic "+auth }};

        httplib::Client cli("192.168.1.66", 80);
        //cli.set_basic_auth("admin","abcd1234");
        //cli.set_digest_auth("admin","abcd1234");
        
        auto res = cli.Get(url.c_str(),headers);
        std::cout << "status:" << res->status << std::endl;
        std::cout << "body:" << res->body << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return true;
}