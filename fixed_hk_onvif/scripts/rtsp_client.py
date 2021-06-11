#!/usr/bin/env python
#coding=utf-8
import socket
import hashlib
import base64
import time

config_dict = {
    'server_username': 'admin',                 #RTSP用户名
    'server_password': 'abcd1234',              #RTSP用户名对应密码
    'server_ip': '192.168.1.65',                 #RTSP服务器IP地址
    'server_port': 554,                         #RTSP服务器使用端口
    'server_path': '/ISAPI/Streaming/thermal/channels/2/streamType/real-time_raw_data',  #URL中端口之后的部份，测试发现不同服务器对这部份接受的值是不一样的，也就是说自己使用时很可能得自己修改这部份的值
    'cseq': 0,                                  #RTSP使用的请求起始序列码，不需要改动
    'user_agent': 'LibVLC/3.0.2 (LIVE555 Streaming Media v2016.11.28)', #自定义请求头部
    'buffer_len': 1024,                         #用于接收服务器返回数据的缓冲区的大小
    'auth_method': 'Digest',                    #RTSP使用的认证方法，Basic/Digest
    'header_normal_modify_allow': False,        #是否允许拼接其他协议规定的请求头的总开关，请些请求头的值为正常值（大多是RFC给出的示例）
    'header_overload_modify_allow': False,      #是否允许拼接其他协议规定的请求头的总开关，请些请求头的值为超长字符串
    'options_header_modify': True,              #OPTIONS请求中，是否允许拼接其他协议规定的请求头的开关
    'describe_header_modify': True,             #第一次DESCRIBE请求中，是否允许拼接其他协议规定的请求头的开关
    'describe_auth_header_modify': True,        #第二次DESCRIBE请求中，是否允许拼接其他协议规定的请求头的开关
    'setup_header_modify': True,                #第一次SETUP请求中，是否允许拼接其他协议规定的请求头的开关
    'setup_session_header_modify': True,        #第二次SETUP请求中，是否允许拼接其他协议规定的请求头的开关
    'play_header_modify': True,                 #PLAY请求中，是否允许拼接其他协议规定的请求头的开关
    'get_parameter_header_modify': True,        #GET PARAMETER请求中，是否允许拼接其他协议规定的请求头的开关
    'teardown_header_modify': True              #TEARDOWN请求中，是否允许拼接其他协议规定的请求头的开关
}

class RtspClient():
    def __init__(self):
        # self.socket_send = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # self.socket_send.settimeout(5)
        # self.socket_send.connect((config_dict['server_ip'], config_dict['server_port']))
        pass

    #用于Digest认证方式时生成response的值
    def gen_response_value(self,url,public_method,realm,nonce):
        frist_pre_md5_value = hashlib.md5((config_dict['server_username'] + ':' + realm + ':' + config_dict['server_password']).encode()).hexdigest()
        first_post_md5_value = hashlib.md5((public_method+':' + url).encode()).hexdigest()
        response_value = hashlib.md5((frist_pre_md5_value + ':' + nonce + ':' + first_post_md5_value).encode()).hexdigest()
        return response_value
    
    #生成options请求头部
    def gen_options_header(self):
        global config_dict
        str_options_header = 'OPTIONS rtsp://' + config_dict['server_ip'] + ':' + str(config_dict['server_port']) + config_dict[
            'server_path'] + ' RTSP/1.0\r\n'
        str_options_header += 'CSeq: ' + str(config_dict['cseq']) + '\r\n'
        str_options_header += 'User-Agent: ' + config_dict['user_agent'] + '\r\n'
        str_options_header += '\r\n'
        return str_options_header
    
    #生成第一次describe请求头部
    def gen_describe_header(self):
        global config_dict
        str_describe_header = 'DESCRIBE rtsp://' + config_dict['server_ip'] + ':' + str(config_dict['server_port']) + config_dict[
            'server_path'] + ' RTSP/1.0\r\n'
        str_describe_header += 'CSeq: ' + str(config_dict['cseq'] + 1) + '\r\n'
        str_describe_header += 'User-Agent: ' + config_dict['user_agent'] + '\r\n'
        str_describe_header += 'Authorization: ' + '\r\n'
        str_describe_header += 'Accept: application/sdp\r\n'
        str_describe_header += '\r\n'
        return str_describe_header
    
    #生成第二次describe请求头部
    def gen_describe_auth_header(self,url,realm,nonce):
        global config_dict
        public_method = 'DESCRIBE'
        str_describe_auth_header = 'DESCRIBE rtsp://' + config_dict['server_ip'] + ':' + str(config_dict['server_port']) + config_dict['server_path'] + ' RTSP/1.0\r\n'
        str_describe_auth_header += 'CSeq: ' + str(config_dict['cseq'] + 2) + '\r\n'
        if config_dict['auth_method'] == 'Basic':
            auth_64 = base64.b64encode((config_dict['server_username'] + ":" + config_dict['server_password']).encode("utf-8")).decode()
            str_describe_auth_header += 'Authorization: Basic '+auth_64 + ' \r\n'
        else:
            response_value = self.gen_response_value(url, public_method, realm, nonce)
            str_describe_auth_header += 'Authorization: Digest username="'+config_dict['server_username']+'", realm="'+realm+'", nonce="'+nonce+'", uri="'+url+'", response="'+response_value+'"\r\n'
        str_describe_auth_header += 'User-Agent: ' + config_dict['user_agent'] + '\r\n'
        str_describe_auth_header += 'Accept: application/sdp\r\n'
        str_describe_auth_header += '\r\n'
        return str_describe_auth_header
    
    #生成第一次setup请求头部
    def gen_setup_header(self,url, realm, nonce):
        global config_dict
        public_method = 'SETUP'
        str_setup_header  = 'SETUP rtsp://' + config_dict['server_ip'] + ':' + str(config_dict['server_port']) + config_dict['server_path'] + 'trackID=0 RTSP/1.0\r\n'
        str_setup_header += 'CSeq: ' + str(config_dict['cseq'] + 3) + '\r\n'
        if config_dict['auth_method'] == 'Basic':
            auth_64 = base64.b64encode((config_dict['server_username'] + ":" + config_dict['server_password']).encode("utf-8")).decode()
            str_setup_header += 'Authorization: Basic '+auth_64 + ' \r\n'
        else:
            response_value = self.gen_response_value(url, public_method, realm, nonce)
            str_setup_header += 'Authorization: Digest username="'+config_dict['server_username']+'", realm="'+realm+'", nonce="'+nonce+'", uri="'+url+'", response="'+response_value+'"\r\n'
        str_setup_header += 'User-Agent: ' + config_dict['user_agent'] + '\r\n'
        str_setup_header += 'Transport: RTP/AVP;unicast;client_port=50166-50167\r\n'
        str_setup_header += '\r\n'
        return str_setup_header
    
    #生成第二次setup请求头部
    def gen_setup_session_header(self,url, realm, nonce,session):
        global config_dict
        public_method = 'SETUP'
        str_setup_session_header = 'SETUP rtsp://' + config_dict['server_ip'] + ':' + str(config_dict['server_port']) + config_dict[
            'server_path'] + 'trackID=1 RTSP/1.0\r\n'
        str_setup_session_header += 'CSeq: ' + str(config_dict['cseq'] + 4) + '\r\n'
        if config_dict['auth_method'] == 'Basic':
            auth_64 = base64.b64encode((config_dict['server_username'] + ":" + config_dict['server_password']).encode("utf-8")).decode()
            str_setup_session_header += 'Authorization: Basic '+auth_64 + ' \r\n'
        else:
            response_value = self.gen_response_value(url, public_method, realm, nonce)
            str_setup_session_header += 'Authorization: Digest username="'+config_dict['server_username']+'", realm="'+realm+'", nonce="'+nonce+'", uri="'+url+'", response="'+response_value+'"\r\n'
        str_setup_session_header += 'User-Agent: ' + config_dict['user_agent'] + '\r\n'
        str_setup_session_header += 'Transport: RTP/AVP;unicast;client_port=50168-50169\r\n'
        str_setup_session_header += 'Session: '+session+'\r\n'
        str_setup_session_header += '\r\n'
        return str_setup_session_header
    
    #生成play请求头部
    def gen_play_header(self,url, realm, nonce,session):
        global config_dict
        public_method = 'PLAY'
        str_play_header = 'PLAY rtsp://' + config_dict['server_ip'] + ':' + str(config_dict['server_port']) + config_dict[
            'server_path'] + ' RTSP/1.0\r\n'
        str_play_header += 'CSeq: ' + str(config_dict['cseq'] + 5) + '\r\n'
        if config_dict['auth_method'] == 'Basic':
            auth_64 = base64.b64encode((config_dict['server_username'] + ":" + config_dict['server_password']).encode("utf-8")).decode()
            str_play_header += 'Authorization: Basic '+auth_64 + ' \r\n'
        else:
            response_value = self.gen_response_value(url, public_method, realm, nonce)
            str_play_header += 'Authorization: Digest username="'+config_dict['server_username']+'", realm="'+realm+'", nonce="'+nonce+'", uri="'+url+'", response="'+response_value+'"\r\n'
        str_play_header += 'User-Agent: ' + config_dict['user_agent'] + '\r\n'
        str_play_header += 'Session: '+session+'\r\n'
        str_play_header += 'Range: npt=0.000-\r\n'
        str_play_header += '\r\n'
        return str_play_header
    
    #生成GET_PARAMETER请求头部
    def gen_get_parameter_header(self,url, realm, nonce, session,count):
        global config_dict
        public_method = 'GET_PARAMETER'
        str_get_parameter_header = 'GET_PARAMETER rtsp://' + config_dict['server_ip'] + ':' + str(config_dict['server_port']) + config_dict[
            'server_path'] + ' RTSP/1.0\r\n'
        str_get_parameter_header += 'CSeq: ' + str(config_dict['cseq'] + 6+int(count)) + '\r\n'
        if config_dict['auth_method'] == 'Basic':
            auth_64 = base64.b64encode((config_dict['server_username'] + ":" + config_dict['server_password']).encode("utf-8")).decode()
            str_get_parameter_header += 'Authorization: Basic '+auth_64 + ' \r\n'
        else:
            response_value = self.gen_response_value(url, public_method, realm, nonce)
            str_get_parameter_header += 'Authorization: Digest username="' + config_dict[
            'server_username'] + '", realm="' + realm + '", nonce="' + nonce + '", uri="' + url + '", response="' + response_value + '"\r\n'
        str_get_parameter_header += 'User-Agent: ' + config_dict['user_agent'] + '\r\n'
        str_get_parameter_header += 'Session: ' + session + '\r\n'
        str_get_parameter_header += '\r\n'
        return str_get_parameter_header
    
    #生成teardown请求头部
    def gen_teardown_header(self,url, realm, nonce, session):
        global config_dict
        public_method = 'TEARDOWN'
        str_teardown_header = 'TEARDOWN rtsp://' + config_dict['server_ip'] + ':' + str(config_dict['server_port']) + config_dict['server_path'] + ' RTSP/1.0\r\n'
        str_teardown_header += 'CSeq: ' + str(config_dict['cseq'] + 11) + '\r\n'
        if config_dict['auth_method'] == 'Basic':
            auth_64 = base64.b64encode((config_dict['server_username'] + ":" + config_dict['server_password']).encode("utf-8")).decode()
            str_teardown_header += 'Authorization: Basic '+auth_64 + ' \r\n'
        else:
            response_value = self.gen_response_value(url, public_method, realm, nonce)
            str_teardown_header += 'Authorization: Digest username="' + config_dict[
            'server_username'] + '", realm="' + realm + '", nonce="' + nonce + '", uri="' + url + '", response="' + response_value + '"\r\n'
        str_teardown_header += 'User-Agent: ' + config_dict['user_agent'] + '\r\n'
        str_teardown_header += 'Session: ' + session + '\r\n'
        str_teardown_header += '\r\n'
        return str_teardown_header
    
    #拼接rtsp协议的其他请求头，以测试程序对这些请求头部的处理是否有问题；这个方法与add_overload_header_according_to_protocol是互斥的
    def add_normal_header_according_to_protocol(self,str_header):
        str_header = str_header[0:len(str_header)-2]
        str_header += 'Accept: application/rtsl, application/sdp;level=-2'
        str_header += 'Accept-Encoding: gzip;q=1.0, identity; q=0.5, *;q=0\r\n'
        str_header += 'Accept-Language: da, en-gb;q=0.8, en;q=0.7\r\n'
        str_header += 'Bandwidth: 4000 \r\n'
        str_header += 'Blocksize: 4000 \r\n'
        str_header += 'Cache-Control: no-cache;max-stale \r\n'
        str_header += 'Conference: 199702170042.SAA08642@obiwan.arl.wustl.edu%20Starr \r\n'
        str_header += 'Connection: close\r\n'
        str_header += 'Content-Base: gzip\r\n'
        str_header += 'Content-Encoding: gzip\r\n'
        str_header += 'Content-Language: mi,en\r\n'
        str_header += 'Content-Length: 2034953454546565 \r\n'
        str_header += 'Content-Location: /etc/passwd\r\n'
        str_header += 'Content-Type: text/html; charset=ISO-8859-4gg\r\n'
        str_header += 'Date: Tue, 15 Nov 1995x 08:12:31 GMT\r\n'
        str_header += 'Expires: Thu, 01 Dec 1994 16:00:00 GMT \r\n'
        str_header += 'From: webmaster@w3.org\r\n'
        str_header += 'If-Modified-Since: Sat, 29 Oct 1994 19:43:31 GMT \r\n'
        str_header += 'Last-Modified: Tue, 15 Nov 1994 12:45:26 GMT\r\n'
        str_header += 'Proxy-Require: funky-feature\r\n'
        str_header += 'Referer: http://www.w3.org/hypertext/DataSources/Overview.html\r\n'
        str_header += 'Require: funky-feature \r\n'
        str_header += 'Scale: -3.5 \r\n'
        str_header += 'Speed: 2.5 \r\n'
        str_header += 'Transport: RTP/AVP;unicast;client_port=3456-3457;mode="PLAY" \r\n'
        str_header += 'Via: 1.0 fred, 1.1 nowhere.com (Apache/1.1)\r\n'
        str_header += 'Range: npt=2\r\n'
        str_header += '\r\n'
        return str_header

    #拼接rtsp协议的其他请求头，并将这些请求头的字赋为超长字符串，以测试程序对这些请求头部的处理是否有缓冲区溢出问题；这个方法与add_normal_header_according_to_protocol是互斥的
    def add_overload_header_according_to_protocol(self,str_header):
        str_header = str_header[0:len(str_header) - 2]
        str_header += 'Accept: application/rtsl012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789, application/sdp;level=-2012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789'
        str_header += 'Accept-Encoding: gzip01234567890123456789012345678901234567890123456789;q=1.0012345678901234567890123456789012345678901234567890123456789, identity; q=0.5012345678901234567890123456789, *;q=0012345678901234567890123456789\r\n'
        str_header += 'Accept-Language: da, en-gb;q=0.80123456789012345678901234567890123456789, en;q=0.7012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789\r\n'
        str_header += 'Bandwidth: 400001234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 \r\n'
        str_header += 'Blocksize: 4000012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 \r\n'
        str_header += 'Cache-Control: no-cache;max-stale \r\n'
        str_header += 'Conference: 199702170042.SAA08642@obiwan.arl.wustl.edu%20Starr01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 \r\n'
        str_header += 'Connection: 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789close\r\n'
        str_header += 'Content-Base: 0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789gzip\r\n'
        str_header += 'Content-Encoding: 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789gzip\r\n'
        str_header += 'Content-Language: 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789mi,en\r\n'
        str_header += 'Content-Length: 203495345454656501234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 \r\n'
        str_header += 'Content-Location: /etc/passwd01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789\r\n'
        str_header += 'Content-Type: text/html012345678901234567890123456789012345678901234567890123456789; charset=012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789ISO-8859-4gg\r\n'
        str_header += 'Date: Tue, 15 Nov 1995x 08:12:310123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 GMT\r\n'
        str_header += 'Expires: Thu, 01 Dec 1994 16012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789:00:00 GMT \r\n'
        str_header += 'From: webmaster@w30123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789.org\r\n'
        str_header += 'If-Modified-Since: Sat, 29 Oct 1994 19:43:31012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 GMT \r\n'
        str_header += 'Last-Modified: Tue, 15 Nov 1994 120123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789:45:26 GMT\r\n'
        str_header += 'Proxy-Require: funky-feature\r\n'
        str_header += 'Referer: http://www.w3.org/hypertext/DataSources/Overview.html\r\n'
        str_header += 'Require: funky-feature0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 \r\n'
        str_header += 'Scale: -0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567893.5 \r\n'
        str_header += 'Speed: 20123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789.5 \r\n'
        str_header += 'Transport: RTP/AVP;unicast;client_port=3456-345012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567897;mode="01234567890123456789PLAY" \r\n'
        str_header += 'Via: 1.0 fred, 0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567891.1 nowhere.com (Apache/0123456789012345678901234567891.1)\r\n'
        str_header += 'Range: npt=20123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789\r\n'
        str_header += '\r\n'
        return str_header
    
    #执行一次完整的rtsp播放请求，OPTIONS/DESCRIBE/SETUP/PLAY/GET PARAMETER/TEARDOWN，如果某个请求不正确则中止
    #此方法推荐用于则试服务端是否正确实现RTSP服务
    def exec_full_request(self):
        self.socket_send = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket_send.settimeout(5)
        self.socket_send.connect((config_dict['server_ip'], config_dict['server_port']))
        
        url = 'rtsp://' + config_dict['server_ip'] + ':' + str(config_dict['server_port']) + config_dict['server_path']
        print("url:",url)
        print('now start to check options operation')
        str_options_header = self.gen_options_header()
        if config_dict['header_normal_modify_allow'] & config_dict['options_header_modify']:
            str_options_header = self.add_normal_header_according_to_protocol(str_options_header)
        elif config_dict['header_overload_modify_allow'] & config_dict['options_header_modify']:
            str_options_header = self.add_overload__header_according_to_protocol(str_options_header)
        print("str_options_header:",str_options_header)
        self.socket_send.send(str_options_header.encode())
        msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
        print("receive:",msg_recv)
        if '200 OK' in msg_recv:
            print('OPTIONS request is OK')
        else:
            print('OPTIONS request is BAD')
        str_describe_header = self.gen_describe_header()
        if config_dict['header_normal_modify_allow'] & config_dict['describe_header_modify']:
            str_describe_header = self.add_normal_header_according_to_protocol(str_describe_header)
        elif config_dict['header_overload_modify_allow'] & config_dict['describe_header_modify']:
            str_describe_header = self.add_overload__header_according_to_protocol(str_describe_header)
        print("first describe:",str_describe_header)
        self.socket_send.send(str_describe_header.encode())
        msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
        print("receive:",msg_recv)
        if msg_recv.find('401 Unauthorized') == -1 & False:
            msg_recv_dict = msg_recv.split('\r\n')
            print('first DESCRIBE request occur error: ')
            print(msg_recv_dict[0])
        else:
            print('first DESCRIBE is ok,now we will execute second DESCRIBE for auth')
            realm_pos = msg_recv.find('realm')
            realm_value_begin_pos = msg_recv.find('"', realm_pos)+1
            realm_value_end_pos = msg_recv.find('"', realm_pos + 8)
            realm_value = msg_recv[realm_value_begin_pos:realm_value_end_pos]
            nonce_pos = msg_recv.find('nonce')
            nonce_value_begin_pos = msg_recv.find('"', nonce_pos)+1
            nonce_value_end_pos = msg_recv.find('"', nonce_pos + 8)
            nonce_value = msg_recv[nonce_value_begin_pos:nonce_value_end_pos]
            str_describe_auth_header = self.gen_describe_auth_header(url, realm_value, nonce_value)
            if config_dict['header_normal_modify_allow'] & config_dict['describe_auth_header_modify']:
                str_describe_auth_header = self.add_normal_header_according_to_protocol(str_describe_auth_header)
            elif config_dict['header_overload_modify_allow'] & config_dict['describe_auth_header_modify']:
                str_describe_auth_header = self.add_overload__header_according_to_protocol(str_describe_auth_header)
            self.socket_send.send(str_describe_auth_header.encode())
            msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
            if msg_recv.find('200 OK') == -1:
                msg_recv_dict = msg_recv.split('\r\n')
                print('second DESCRIBE request occur error: ')
                print(msg_recv_dict[0])
            else:
                print('second DESCRIBE is ok,now we will execute first SETUP for session')
                str_setup_header = self.gen_setup_header(url, realm_value, nonce_value)
                if config_dict['header_normal_modify_allow'] & config_dict['setup_header_modify']:
                    str_setup_header = self.add_normal_header_according_to_protocol(str_setup_header)
                elif config_dict['header_overload_modify_allow'] & config_dict['setup_header_modify']:
                    str_setup_header = self.add_overload__header_according_to_protocol(str_setup_header)
                self.socket_send.send(str_setup_header.encode())
                msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
                if msg_recv.find('200 OK') == -1:
                    msg_recv_dict = msg_recv.split('\r\n')
                    print('first SETUP request occur error: ')
                    print(msg_recv_dict[0])
                else:
                    print('first SETUP is ok,now we will execute second SETUP')
                    session_pos = msg_recv.find('Session')
                    session_value_begin_pos = msg_recv.find(' ',session_pos+8)+1
                    session_value_end_pos = msg_recv.find(';',session_pos+8)
                    session_value = msg_recv[session_value_begin_pos:session_value_end_pos]
                    str_setup_session_header = self.gen_setup_session_header(url, realm_value, nonce_value,session_value)
                    if config_dict['header_normal_modify_allow'] & config_dict['setup_session_header_modify']:
                        str_setup_session_header = self.add_normal_header_according_to_protocol(str_setup_session_header)
                    elif config_dict['header_overload_modify_allow'] & config_dict['setup_session_header_modify']:
                        str_setup_session_header = self.add_overload_header_according_to_protocol(str_setup_session_header)
                    self.socket_send.send(str_setup_session_header.encode())
                    msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
                    if msg_recv.find('200 OK') == -1:
                        msg_recv_dict = msg_recv.split('\r\n')
                        print('second SETUP request occur error: ')
                        print(msg_recv_dict[0])
                    else:
                        print('second SETUP is ok, now we wil execute PLAY')
                        str_play_header = self.gen_play_header(url, realm_value, nonce_value, session_value)
                        if config_dict['header_normal_modify_allow'] & config_dict['play_header_modify']:
                            str_play_header = self.add_normal_header_according_to_protocol(str_play_header)
                        elif config_dict['header_overload_modify_allow'] & config_dict['play_header_modify']:
                            str_play_header = self.add_overload_header_according_to_protocol(str_play_header)
                        self.socket_send.send(str_play_header.encode())
                        msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
                        if msg_recv.find('200 OK') == -1:
                            msg_recv_dict = msg_recv.split('\r\n')
                            print('PLAY request occur error: ')
                            print(msg_recv_dict[0])
                        else:
                            print('PLAY is ok, we will execute GET_PARAMETER every 10 seconds and 5 times total')
                            for i in range(2):
                                str_get_parameter_header = self.gen_get_parameter_header(url, realm_value, nonce_value, session_value,str(i))
                                if config_dict['header_normal_modify_allow'] & config_dict['get_parameter_header_modify']:
                                    str_get_parameter_header = self.add_normal_header_according_to_protocol(str_get_parameter_header)
                                elif config_dict['header_overload_modify_allow'] & config_dict['get_parameter_header_modify']:
                                    str_get_parameter_header = self.add_overload_header_according_to_protocol(str_get_parameter_header)
                                self.socket_send.send(str_get_parameter_header.encode())
                                msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
                                msg_recv_dict = msg_recv.split('\r\n')
                                print(str(i)+'*10:'+msg_recv_dict[0])
                                time.sleep(10)
                            print('\nnow we will execute TEARDOWN to disconnect with server')
                            str_teardown_header = self.gen_teardown_header(url, realm_value, nonce_value, session_value)
                            if config_dict['header_normal_modify_allow'] & config_dict['teardown_header_modify']:
                                str_teardown_header = self.add_normal_header_according_to_protocol(str_teardown_header)
                            elif config_dict['header_overload_modify_allow'] & config_dict['teardown_header_modify']:
                                str_teardown_header = self.add_overload_header_according_to_protocol(str_teardown_header)
                            self.socket_send.send(str_teardown_header.encode())
                            msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
                            print(msg_recv)
                            print('program execute finished, thank you')
        self.socket_send.close()

    
    #强制依次执行OPTIONS/DESCRIBE/SETUP/PLAY/GET PARAMETER/TEARDOWN，某个请求报错也进行下一个请求
    #此方法推荐用于测试服务端是否有溢出漏洞
    def exec_force_request(self):
        self.socket_send = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket_send.settimeout(5)
        self.socket_send.connect((config_dict['server_ip'], config_dict['server_port']))
    
        url = 'rtsp://' + config_dict['server_ip'] + ':' + str(config_dict['server_port']) + config_dict['server_path']
    
        print('now start to check options operation')
        str_options_header = self.gen_options_header()
        if config_dict['header_normal_modify_allow'] & config_dict['options_header_modify']:
            str_options_header = self.add_normal_header_according_to_protocol(str_options_header)
        elif config_dict['header_overload_modify_allow'] & config_dict['options_header_modify']:
            str_options_header = self.add_overload_header_according_to_protocol(str_options_header)
        self.socket_send.send(str_options_header.encode())
        msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
        msg_recv_dict = msg_recv.split('\r\n')
        print('the OPTIONS response is :')
        print(msg_recv_dict[0])
        print('\nnow we will execute first DESCRIBE request')
        str_describe_header = self.gen_describe_header()
        if config_dict['header_normal_modify_allow'] & config_dict['describe_header_modify']:
            str_describe_header = self.add_normal_header_according_to_protocol(str_describe_header)
        elif config_dict['header_overload_modify_allow'] & config_dict['describe_header_modify']:
            str_describe_header = self.add_overload_header_according_to_protocol(str_describe_header)
        self.socket_send.send(str_describe_header.encode())
        msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
        msg_recv_dict = msg_recv.split('\r\n')
        print('the first DESCRIBE response is :')
        print(msg_recv_dict[0])
        print('\nnow we will execute second DESCRIBE for auth')
        realm_value = 'RTSP SERVER'
        nonce_value = 'e29027990294f81b941b04a7ac78c196'
        str_describe_auth_header = self.gen_describe_auth_header(url, realm_value, nonce_value)
        if config_dict['header_normal_modify_allow'] & config_dict['describe_auth_header_modify']:
            str_describe_auth_header = self.add_normal_header_according_to_protocol(str_describe_auth_header)
        elif config_dict['header_overload_modify_allow'] & config_dict['describe_auth_header_modify']:
            str_describe_auth_header = self.add_overload_header_according_to_protocol(str_describe_auth_header)
        self.socket_send.send(str_describe_auth_header.encode())
        msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
        msg_recv_dict = msg_recv.split('\r\n')
        print('the second DESCRIBE response is :')
        print(msg_recv_dict[0])
        print('\nnow we will execute first SETUP for session')
        str_setup_header = self.gen_setup_header(url, realm_value, nonce_value)
        if config_dict['header_normal_modify_allow'] & config_dict['setup_header_modify']:
            str_setup_header = self.add_normal_header_according_to_protocol(str_setup_header)
        elif config_dict['header_overload_modify_allow'] & config_dict['setup_header_modify']:
            str_setup_header = self.add_overload_header_according_to_protocol(str_setup_header)
        self.socket_send.send(str_setup_header.encode())
        msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
        msg_recv_dict = msg_recv.split('\r\n')
        print('the first SETUP response is :')
        print(msg_recv_dict[0])
        print('\nnow we will execute second SETUP')
        session_value = '464314414585882'
        str_setup_session_header = self.gen_setup_session_header(url, realm_value, nonce_value, session_value)
        if config_dict['header_normal_modify_allow'] & config_dict['setup_session_header_modify']:
            str_setup_session_header = self.add_normal_header_according_to_protocol(str_setup_session_header)
        elif config_dict['header_overload_modify_allow'] & config_dict['setup_session_header_modify']:
            str_setup_session_header = self.add_overload_header_according_to_protocol(str_setup_session_header)
        self.socket_send.send(str_setup_session_header.encode())
        msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
        msg_recv_dict = msg_recv.split('\r\n')
        print('the second SETUP response is :')
        print(msg_recv_dict[0])
        print('\nnow we wil execute PLAY')
        str_play_header = self.gen_play_header(url, realm_value, nonce_value, session_value)
        if config_dict['header_normal_modify_allow'] & config_dict['play_header_modify']:
            str_play_header = self.add_normal_header_according_to_protocol(str_play_header)
        elif config_dict['header_overload_modify_allow'] & config_dict['play_header_modify']:
            str_play_header = self.add_overload_header_according_to_protocol(str_play_header)
        self.socket_send.send(str_play_header.encode())
        msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
        msg_recv_dict = msg_recv.split('\r\n')
        print('the PLAY response is :')
        print(msg_recv_dict[0])
        print('\nnow we will execute GET_PARAMETER')
        str_get_parameter_header = self.gen_get_parameter_header(url, realm_value, nonce_value, session_value, '1')
        if config_dict['header_normal_modify_allow'] & config_dict['get_parameter_header_modify']:
            str_get_parameter_header = self.add_normal_header_according_to_protocol(str_get_parameter_header)
        elif config_dict['header_overload_modify_allow'] & config_dict['get_parameter_header_modify']:
            str_get_parameter_header = self.add_overload_header_according_to_protocol(str_get_parameter_header)
        self.socket_send.send(str_get_parameter_header.encode())
        msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
        msg_recv_dict = msg_recv.split('\r\n')
        print('the GET_PARAMETER response is :')
        print(msg_recv_dict[0])
        print('\nnow we will execute TEARDOWN to disconnect with server')
        str_teardown_header = self.gen_teardown_header(url, realm_value, nonce_value, session_value)
        if config_dict['header_normal_modify_allow'] & config_dict['teardown_header_modify']:
            str_teardown_header = self.add_normal_header_according_to_protocol(str_teardown_header)
        elif config_dict['header_overload_modify_allow'] & config_dict['teardown_header_modify']:
            str_teardown_header = self.add_overload_header_according_to_protocol(str_teardown_header)
        self.socket_send.send(str_teardown_header.encode())
        msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
        msg_recv_dict = msg_recv.split('\r\n')
        print('the GET_PARAMETER response is :')
        print(msg_recv_dict[0])
        print('program execute finished, thank you')
        self.socket_send.close()

    def __del__(self):
        # self.socket_send.close()
        pass


if __name__ == '__main__':
    rtsp_client = RtspClient()
    rtsp_client.exec_full_request()
    #rtsp_client.exec_force_request()
