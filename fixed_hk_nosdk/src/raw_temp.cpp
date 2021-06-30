/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-21 10:18:49
 * @LastEditors: li
 * @LastEditTime: 2021-04-23 15:20:38
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
        auto res = cli.Get(url.c_str(),headers);
        // std::string auth = "WWW-Authenticate";
        // bool is_exit = res->has_header(auth.c_str());
        // std::cout << "is_exit:" << is_exit << std::endl;
        //std::multimap<std::string, std::string, detail::ci> headers = res->headers;
        //std::cout << "headers:" << res->headers << std::endl;
        std::cout << "status:" << res->status << std::endl;
        std::cout << "body:" << res->body << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    return true;
}

bool raw_temp::auth(std::string name,std::string pwd){
    // auto host = "httpbin.org";
    // auto port = 443;
    // SSLClient cli(host, port);
    // {
    //     auto res = cli.Get("/digest-auth/auth/hello/world");
    //     std::cout << "status:" << res->status << std::endl;
    //     std::cout << "body:" << res->body << std::endl;
    // }
    // cli.set_digest_auth("hello", "world");
    // {
    //     auto res = cli.Get("/digest-auth/auth/hello/world");
    //     std::cout << "status:" << res->status << std::endl;
    //     std::cout << "body:" << res->body << std::endl;
    // }
    std::string host = "192.168.1.66";
    int port = 80;
    std::string url = "/ISAPI/Security/userCheck";
    // std::string host = "httpbin.org";
    // int port = 443;
    // std::string url = "/digest-auth/auth/hello/world";
    // name = "hello";
    // pwd = "world";


    httplib::Client cli(host.c_str(), port);
    auto res = cli.Get(url.c_str());
    std::string auth = "WWW-Authenticate";
    bool is_exit = res->has_header(auth.c_str());
    std:map<std::string,std::string> header_map;
    if(is_exit){
        std::string val = res->get_header_value(auth.c_str());
        std::cout << "header value:" << val << std::endl;
        //get qop realm nonce stale
        //Digest qop="auth", realm="IP Camera(F1249)", nonce="4d324a6d5a54526d5a5467365a6d45314d574d304e44413d", stale="FALSE"
        std::vector<std::string> list;
        SplitString(val,list,",");
        for(int i = 0;i < list.size();i++){
            std::string cval = list[i];
            std::cout << i << ":" << cval << std::endl;
            //split by :
            std::vector<std::string> clist;
            SplitString(cval,clist,"=");
            if(clist.size()==2){
                //std::cout << "key:" << clist[0] << " value:" << clist[1] << std::endl;
                std::string key = clist[0];
                size_t n = key.find_first_not_of(" \r\n\t");
                if (n != string::npos){
                    key.erase(0, n);
                }
                header_map.insert(make_pair(key,clist[1]));
            }
        }
    }
    // for(auto iter = header_map.begin(); iter != header_map.end(); iter++){
    //      std::cout << "first:" << iter->first<< " second:" << iter->second << std::endl;
    // }
    auto it = header_map.find("realm");
    if(it == header_map.end()){
        return false;
    }
    std::string realm = it->second;
    realm.erase(remove( realm.begin(), realm.end(), '\"' ),realm.end());
    auto nonce_it = header_map.find("nonce");
    if(nonce_it == header_map.end()){
        return false;
    }
    std::string nonce = nonce_it->second;
    nonce.erase(remove( nonce.begin(), nonce.end(), '\"' ),nonce.end());
    //opaque
    std::string opaque = "";
    auto opaque_it = header_map.find("opaque");
    if(opaque_it != header_map.end()){
        opaque = opaque_it->second;
    }
    std::cout << "opaque:" << opaque << std::endl;
    std::cout << "status:" << res->status << std::endl;
    std::cout << "body:" << res->body << std::endl;
    //与安全相关的数据的A1
    std::string a1 = name +":"+ realm +":"+ pwd;
    std::string a1md5 = MD5(a1);
    std::cout << "a1:" << a1 << std::endl;
    std::string a2 = "GET:"+ url;
    std::string a2md5 = MD5(a2);
    std::cout << "a2:" << a2 << std::endl;
    std::string cnonce = rtsptool::base64_encode(get_system_cur_time());
    std::string nc = "00000001";
    //response=MD5(MD5(A1):<nonce>:<nc>:<cnonce>:<qop>:MD5(A2))
    std::string resstr = a1md5 +":"+ nonce +":"+ nc +":"+ cnonce +":"+auth+":"+a2md5;
    std::cout << "resstr:" << resstr << std::endl; 
    std::string response = MD5(resstr);
    std::cout << "nonce:" << nonce << std::endl;
    std::cout << "cnonce:" << cnonce << std::endl;
    std::cout << "response:" << response << std::endl;
    std::string digest = "Digest username=\"" + name + "\", realm=\""+realm+"\"";
    digest = digest + ", nonce=\"" +nonce + "\", uri=\"" + url + "\", cnonce=\""+cnonce+"\"";
    digest = digest +", nc="+ nc +", algorithm=MD5, qop=auth, response=\""+response+"\"";
    if(opaque.size()>0)
        digest = digest +",opaque="+ opaque;

    std::cout << "digest:" << digest << std::endl;
    httplib::Headers headers = {
        { "Accept:", "*/*" },
        { "Accept-Encoding:", "gzip, deflate" },
        { "Host", "192.168.1.66" },
        { "User-Agent:", "Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko" },
        { "Proxy-Connection:", " keep-alive" },
        { "Authorization", digest }
    };
    //res = cli.Get(url.c_str(),headers);
    // std::string authorization = "Authorization";
    // if(res.has_request_header(authorization.c_str())){
    //     std::cout << "Authorization:" << res.get_request_header_value(authorization.c_str()) << std::endl;
    // }
    // std::cout << "status:" << res->status << std::endl;
    // std::cout << "body:" << res->body << std::endl;
    return true;
}

string raw_temp::MD5(const string& src){
    MD5_CTX ctx;
    string md5_string;
    unsigned char md[16] = { 0 };
    char tmp[33] = { 0 };
 
    MD5_Init( &ctx );
    MD5_Update( &ctx, src.c_str(), src.size() );
    MD5_Final( md, &ctx );
 
    for( int i = 0; i < 16; ++i )
    {   
        memset( tmp, 0x00, sizeof( tmp ) );
        sprintf( tmp, "%02X", md[i] );
        md5_string += tmp;
    }   
    return md5_string;    
}
