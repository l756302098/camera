#!/usr/bin/env python
#coding=utf-8
import socket
import hashlib
import base64
import threading
import time
from threading import Thread
import sys
if sys.version > '3':
    import queue as Queue
else:
    import Queue

config_dict = {
    'server_username': 'admin',                 #RTSP用户名
    'server_password': 'abcd1234',              #RTSP用户名对应密码
    'server_ip': '192.168.1.65',                #RTSP服务器IP地址
    'server_port': 554,                         #RTSP服务器使用端口
    'server_path': '/ISAPI/Streaming/thermal/channels/2/streamType/pixel-to-pixel_thermometry_data',  #URL中端口之后的部份，测试发现不同服务器对这部份接受的值是不一样的，也就是说自己使用时很可能得自己修改这部份的值
    'cseq': 0,                                  #RTSP使用的请求起始序列码，不需要改动
    'user_agent': 'LibVLC/3.0.2',               #自定义请求头部
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

def isUse(port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        s.connect(('127.0.0.1', int(port)))
        s.shutdown(2)#shutdown参数表示后续可否读写
        # print '%d is ok' % port
        return True
    except Exception:
        return False

class RtspClient():
    def __init__(self):
        self.queue = Queue.Queue(maxsize=20)
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
        self._rtp_port = 50166
        isuse = True
        while isuse:
            isuse = isUse(self._rtp_port)
            if isuse:
                self._rtp_port = self._rtp_port + 2
        self._rtcp_port = 50167
        isuse = True
        while isuse:
            isuse = isUse(self._rtcp_port)
            if isuse:
                self._rtcp_port = self._rtcp_port + 2
        global config_dict
        public_method = 'SETUP'
        str_setup_header  = 'SETUP rtsp://' + config_dict['server_ip'] + ':' + str(config_dict['server_port']) + config_dict['server_path'] + '/trackID=5 RTSP/1.0\r\n'
        str_setup_header += 'CSeq: ' + str(config_dict['cseq'] + 3) + '\r\n'
        if config_dict['auth_method'] == 'Basic':
            auth_64 = base64.b64encode((config_dict['server_username'] + ":" + config_dict['server_password']).encode("utf-8")).decode()
            str_setup_header += 'Authorization: Basic '+auth_64 + ' \r\n'
        else:
            response_value = self.gen_response_value(url, public_method, realm, nonce)
            str_setup_header += 'Authorization: Digest username="'+config_dict['server_username']+'", realm="'+realm+'", nonce="'+nonce+'", uri="'+url+'", response="'+response_value+'"\r\n'
        str_setup_header += 'User-Agent: ' + config_dict['user_agent'] + '\r\n'
        str_setup_header += 'Transport: RTP/AVP/UDP;unicast;client_port='+str(self._rtp_port)+'-'+str(self._rtcp_port)+'\r\n'
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
    
    def get_temp(self):
        #print("queue size:",self.queue.qsize())
        if self.queue.qsize()>0:
            data = self.queue.get()
            return data
        return None
    
    def start_rtp_receive_thread(self):
        print("start start_rtp_receive_thread")
        self._rtp_receive_thread = Thread(target=self._handle_rtp_receive)
        self._rtp_receive_thread.setDaemon(True)
        self._rtp_receive_thread.start()

        self._rtp2_receive_thread = Thread(target=self._handle_rtcp_receive)
        self._rtp2_receive_thread.setDaemon(True)
        self._rtp2_receive_thread.start()
        print("end start_rtp_receive_thread")
    
    def _handle_rtp_receive(self):
        try:
            print("start _handle_rtp_receive")
            self._rtp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self._rtp_socket.bind(("0.0.0.0", self._rtp_port))
            self._rtp_socket.settimeout(30.)
            print('Bind UDP on 50166...')
            temp_bytes = bytes()
            buffer_max = 10*1024*1024
            while True:
                if not self.is_receiving_rtp:
                    break
                #print("receive rtp data")
                try:
                    recv,addr = self._rtp_socket.recvfrom(buffer_max)
                    if(len(recv)==0):
                        continue
                    recv_bytes = bytearray(recv)
                    header = recv_bytes[:12]
                    payload = recv_bytes[12:]
                    mark = header[1] >> 7
                    payload_type = header[1] & 0x7F
                    if payload_type != 0x6d:
                        print("payload_type",payload_type)
                        continue
                    #print("byte size:",len(recv)," mark:",mark," payload_type:",payload_type)
                    if(len(recv)==140):
                        pass
                    else:
                        temp_bytes += payload
                    if mark == 1:
                        #print("receive one frame image.data size:",len(temp_bytes))
                        self.queue.put(temp_bytes)
                        temp_bytes = b''
                    #print("byte size:",len(recv))
                except socket.timeout:
                    print("time out")
                    continue
        except Exception:
            print("udp listening error")
            #raw = packet.payload
        self._rtp_socket.close()
        print("exit 50166")

    def _handle_rtcp_receive(self):
        self._rtcp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            self._rtcp_socket.bind(("0.0.0.0", self._rtcp_port))
            self._rtcp_socket.settimeout(30.)
            print('Bind UDP on 50167...')
            while True:
                if not self.is_receiving_rtp:
                    break
                print("receive rtcp data")
                try:
                    recv = self._rtcp_socket.recv(2048)
                    print(recv)
                except socket.timeout:
                    print("time out")
                    continue
        except Exception:
            print("udp listening error")
        self._rtcp_socket.close()
        print("exit 50167")

    #执行一次完整的rtsp播放请求，OPTIONS/DESCRIBE/SETUP/PLAY/GET PARAMETER/TEARDOWN，如果某个请求不正确则中止
    #此方法推荐用于则试服务端是否正确实现RTSP服务
    def exec_full_request(self):
        self.socket_send = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket_send.settimeout(5)
        self.socket_send.connect((config_dict['server_ip'], config_dict['server_port']))
        self.socket_open = True
        
        self.url = 'rtsp://' + config_dict['server_ip'] + ':' + str(config_dict['server_port']) + config_dict['server_path']
        print("url:",self.url)
        print('now start to check options operation')
        str_options_header = self.gen_options_header()
        print("str_options_header:",str_options_header)
        self.socket_send.send(str_options_header.encode())
        msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
        msg_recv_dict = msg_recv.split('\r\n')
        print("receive:")
        for item in msg_recv_dict:
            print(item)
        if '200 OK' in msg_recv:
            print('OPTIONS request is OK')
        else:
            print('OPTIONS request is BAD')
            return False
        str_describe_header = self.gen_describe_header()
        print("first describe:",str_describe_header)
        self.socket_send.send(str_describe_header.encode())
        msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
        msg_recv_dict = msg_recv.split('\r\n')
        print("receive:")
        for item in msg_recv_dict:
            print(item)
        if msg_recv.find('401 Unauthorized') == -1 & False:
            msg_recv_dict = msg_recv.split('\r\n')
            print('first DESCRIBE request occur error: ')
            print(msg_recv_dict[0])
        else:
            print('first DESCRIBE is ok,now we will execute second DESCRIBE for auth')
            realm_pos = msg_recv.find('realm')
            realm_value_begin_pos = msg_recv.find('"', realm_pos)+1
            realm_value_end_pos = msg_recv.find('"', realm_pos + 8)
            self.realm_value = msg_recv[realm_value_begin_pos:realm_value_end_pos]
            nonce_pos = msg_recv.find('nonce')
            nonce_value_begin_pos = msg_recv.find('"', nonce_pos)+1
            nonce_value_end_pos = msg_recv.find('"', nonce_pos + 8)
            self.nonce_value = msg_recv[nonce_value_begin_pos:nonce_value_end_pos]
            str_describe_auth_header = self.gen_describe_auth_header(self.url, self.realm_value, self.nonce_value)
            print("second describe:",str_describe_auth_header)
            self.socket_send.send(str_describe_auth_header.encode())
            msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
            msg_recv_dict = msg_recv.split('\r\n')
            print("2 receive:")
            for item in msg_recv_dict:
                print(item)
            if msg_recv.find('200 OK') == -1:
                msg_recv_dict = msg_recv.split('\r\n')
                print('second DESCRIBE request occur error: ')
                print(msg_recv_dict[0])
                return False
            else:
                print('second DESCRIBE is ok,now we will execute first SETUP for session')
                str_setup_header = self.gen_setup_header(self.url, self.realm_value, self.nonce_value)
                print("first SETUP:",str_setup_header)
                self.socket_send.send(str_setup_header.encode())
                # setup udp listen
                self.is_receiving_rtp = True
                self.start_rtp_receive_thread()
                msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
                msg_recv_dict = msg_recv.split('\r\n')
                print("3 receive:")
                for item in msg_recv_dict:
                    print(item)
                if msg_recv.find('200 OK') == -1:
                    msg_recv_dict = msg_recv.split('\r\n')
                    print('first SETUP request occur error: ')
                    print(msg_recv_dict[0])
                    return False
                else:
                    print('first SETUP is ok,now we will execute PLAY')
                    session_pos = msg_recv.find('Session')
                    session_value_begin_pos = msg_recv.find(' ',session_pos+8)+1
                    session_value_end_pos = msg_recv.find(';',session_pos+8)
                    self.session_value = msg_recv[session_value_begin_pos:session_value_end_pos]
                    str_play_header = self.gen_play_header(self.url, self.realm_value, self.nonce_value, self.session_value)
                    print("PLAY:",str_play_header)
                    self.socket_send.send(str_play_header.encode())
                    msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
                    msg_recv_dict = msg_recv.split('\r\n')
                    print("4 receive:",msg_recv)
                    for item in msg_recv_dict:
                        print(item)
                    if msg_recv.find('200 OK') == -1:
                        print('PLAY request occur error: ')
                        print(msg_recv)
                        msg_recv_dict = msg_recv.split('\r\n')
                        print(msg_recv_dict[0])
                        return False
                    else:
                        print('PLAY is ok')
                        return True
                        # try:
                        #     while True:
                        #         time.sleep(1)
                        # except KeyboardInterrupt:
                        #     pass
                        # print('\nnow we will execute TEARDOWN to disconnect with server')
                        # str_teardown_header = self.gen_teardown_header(self.url, self.realm_value, self.nonce_value, self.session_value)
                        # self.socket_send.send(str_teardown_header.encode())
                        # msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
                        # print(msg_recv)
                        # print('program execute finished, thank you')

        # self.is_receiving_rtp = False                  
        # self.socket_send.close()

    def teardown(self):
        if self.socket_open:
            print('\nnow we will execute TEARDOWN to disconnect with server')
            str_teardown_header = self.gen_teardown_header(self.url, self.realm_value, self.nonce_value, self.session_value)
            try:
                self.socket_send.send(str_teardown_header.encode())
                msg_recv = self.socket_send.recv(config_dict['buffer_len']).decode()
                print(msg_recv)
                print('program execute finished, thank you')
            except Exception:
                print('program execute error, thank you')
            self.is_receiving_rtp = False                  
            self.socket_send.close()
            self.socket_open = False

    def __del__(self):
        # self.socket_send.close()
        pass

    def connect_rtsp_server(self):
        try:
            return self.exec_full_request()
        except Exception:
            print("connect_rtsp_server error")
            return False

# if __name__ == '__main__':
#     rtsp_client = RtspClient()
#     rtsp_client.exec_full_request()
