#!/usr/bin/env python
#coding:utf-8
import rospy
from time import sleep
import socket

if __name__ == '__main__':
    try:
        rospy.init_node('obs_client_node')
        nodename = rospy.get_name()
        rospy.loginfo("%s started" % nodename)
        #read param
        device_ip = rospy.get_param('~device_ip',"127.0.0.1")
        device_port = rospy.get_param("~device_port",9999)
        image_width = rospy.get_param("~image_width",1920)
        image_height = rospy.get_param("~image_height",1080)
        image_size = image_width * image_height * 4
        print(device_ip,device_port,image_width,image_height)
        # start tcp client
        tcp_client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # 建立连接:
        tcp_client.connect((device_ip, device_port))
        # 发送数据:
        tcp_client.send(b'hello world')
        buffer = []
        while True:
            d = tcp_client.recv(1500 * 10)
            if d:
                buffer.append(d)
                if len(buffer) == image_size:
                    print("get a frame data")
                    buffer.clear()
            else:
                break
        #data = b''.join(buffer)
        # 关闭连接:
        tcp_client.close()
    except KeyboardInterrupt:
        tcp_client.close()
    except rospy.ROSInterruptException:
        tcp_client.close()