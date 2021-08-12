#!/usr/bin/env python
#coding:utf-8
from time import sleep
from rtsp_client import RtspClient,config_dict
from isapi import HK_Api
from daemon import Daemon
import os
import sys
import threading
from threading import Thread
import socket

class Obs(Daemon):

    def start_tcp_thread(self):
        print("start start_tcp_thread")
        self._tcp_thread = Thread(target=self._handle_tcp_receive)
        self._tcp_thread.setDaemon(True)
        self._tcp_thread.start()
        self.clients = {}
        print("end start_tcp_thread")
    
    def _handle_tcp_receive(self):
        tcp_server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # 监听端口:
        tcp_server.bind(('0.0.0.0', 9999))
        tcp_server.listen(5)
        print('Waiting for connection...')
        while True:
            # 接受一个新连接:
            sock, addr = tcp_server.accept()
            self.clients[addr] = sock
            # 创建新线程来处理TCP连接:
            t = threading.Thread(target=tcplink, args=(sock, addr))
            t.start()

    def tcplink(sock, addr):
        print('Accept new connection from %s:%s...' % addr)
        sock.send(b'Welcome!')
        while True:
            data = sock.recv(1024)
            time.sleep(1)
            if not data or data.decode('utf-8') == 'exit':
                break
            sock.send(('Hello, %s!' % data.decode('utf-8')).encode('utf-8'))
        sock.close()
        if addr in self.clients:
            del self.clients[addr]
        print('Connection from %s:%s closed.' % addr)

    def run(self):
        self.log("start run...")
        self.start_tcp_thread()
        api = HK_Api()
        print("start run...")
        device_ip = "192.168.1.68"
        device_port = 80
        device_username = "admin"
        device_password = "123qweasd"
        image_width = 384
        image_height = 288
        print(device_ip,device_port,device_username,device_password)
        config_dict["server_ip"] = device_ip
        config_dict["server_port"] = device_port
        config_dict["server_username"] = device_username
        config_dict["server_password"] = device_password
        #check param
        print("start check video type")
        check = True
        while(check):
            video_type,ok = api.get_streamParam(device_ip,1,device_username,device_password)
            if ok :
                if video_type == 'pixel-to-pixel_thermometry_data':
                    check = False
                else:
                    ok = api.put_streamParam(device_ip,1,device_username,device_password)
            sleep(1)
        print("finish check video type")
        try:
            #init rtsp client
            rtsp_client = RtspClient()
            succ = rtsp_client.connect_rtsp_server()
            if not succ:
                print("ros node shutdown")
                rtsp_client.teardown()
                return
            #init rosnode
            no_response = 0
            while True:
                data = rtsp_client.get_temp()
                if data != None:
                    no_response = 0
                    data_array = data[4:]
                    image_size = image_height * image_width * 4
                    if len(data_array)!=image_size:
                        print("error size:",len(data_array))
                        continue
                    data_size = len(data_array)
                    header_bytes = data_size.to_bytes(4, 'big')
                    print("ok size:",len(data_array),"header_bytes",header_bytes)
                    if len(self.clients) > 0:
                        try:
                            data_array.insert(0,header_bytes)
                            for k,v in self.clients.items():
                                v.sendall(data_array)
                        except Exception as e:
                            print("socket send error")
                            print(e)
                    #publish temp data
                    # image_temp=Image()
                    # image_temp.height=image_height
                    # image_temp.width=image_width
                    # image_temp.encoding='rgb8'
                    # image_temp.data= str(data_array)
                else:
                    no_response = no_response + 1
                    if no_response > 600:
                        print(["no_response:",no_response])
                        no_response = 0
                        break
            print("rtsp client teardown")
            rtsp_client.teardown()
        except KeyboardInterrupt:
            rtsp_client.teardown()
        self.log("exit")

if __name__ == '__main__':
    if len(sys.argv) == 1:
        print("Foreground run")
        d = Obs('obs_daemon.pid', verbose=2)
        d.run()
    if len(sys.argv) == 2:
        arg = sys.argv[1]
        print("arg:",arg)
        if arg in ('start', 'stop', 'restart'):
            d = Obs('obs_daemon.pid', verbose=2)
            getattr(d, arg)()
        

# if __name__ == '__main__':
#     try:
#         print("obs start ...")
#         api = HK_Api()
#         #read param
#         device_ip = "192.168.1.68"
#         device_port = 80
#         device_username = "admin"
#         device_password = "123qweasd"
#         image_width = 384
#         image_height = 288
#         print(device_ip,device_port,device_username,device_password)
#         config_dict["server_ip"] = device_ip
#         config_dict["server_port"] = device_port
#         config_dict["server_username"] = device_username
#         config_dict["server_password"] = device_password
#         #check param
#         print("start check video type")
#         check = True
#         while(check):
#             video_type,ok = api.get_streamParam(device_ip,1,device_username,device_password)
#             if ok :
#                 if video_type == 'pixel-to-pixel_thermometry_data':
#                     check = False
#                 else:
#                     ok = api.put_streamParam(device_ip,1,device_username,device_password)
#             sleep(1)
#         print("finish check video type")
#         #init rtsp client
#         rtsp_client = RtspClient()
#         succ = rtsp_client.connect_rtsp_server()
#         if not succ:
#             print("ros node shutdown")
#             rtsp_client.teardown()
#             rospy.signal_shutdown('rtsp client connect failed!')
#         #init rosnode
#         no_response = 0
#         while True:
#             data = rtsp_client.get_temp()
#             if data != None:
#                 no_response = 0
#                 data_array = data[4:]
#                 image_size = image_height * image_width * 4
#                 if len(data_array)!=image_size:
#                     print("error size:",len(data_array))
#                     continue
#                 print("ok size:",len(data_array))
#                 #publish temp data
#                 # image_temp=Image()
#                 # image_temp.height=image_height
#                 # image_temp.width=image_width
#                 # image_temp.encoding='rgb8'
#                 # image_temp.data= str(data_array)
#             else:
#                 no_response = no_response + 1
#                 if no_response > 600:
#                     print(["no_response:",no_response])
#                     no_response = 0
#                     break
#         print("rtsp client teardown")
#         rtsp_client.teardown()
#     except KeyboardInterrupt:
#         rtsp_client.teardown()
#     except rospy.ROSInterruptException:
#         rtsp_client.teardown()
#         pass