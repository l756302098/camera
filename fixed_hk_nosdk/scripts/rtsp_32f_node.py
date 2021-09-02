#!/usr/bin/env python
#coding:utf-8
import rospy
from time import sleep
from sensor_msgs.msg import Image
from rtsp_client_4 import RtspClient,config_dict
from isapi import HK_Api
import cv2
from cv_bridge import CvBridge, CvBridgeError
import numpy as np

if __name__ == '__main__':
    try:
        rospy.init_node('rtsp_temp_node')
        nodename = rospy.get_name()
        rospy.loginfo("%s started" % nodename)
        api = HK_Api()
        #read param
        device_ip = rospy.get_param('~device_ip',"127.0.0.1")
        device_port = rospy.get_param("~device_port",80)
        device_username = rospy.get_param("~device_username","admin")
        device_password = rospy.get_param("~device_password","123qweasd")
        image_width = rospy.get_param("~image_width",1920)
        image_height = rospy.get_param("~image_height",1080)
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
        #init rtsp client
        rtsp_client = RtspClient()
        succ = rtsp_client.connect_rtsp_server()
        if not succ:
            print("ros node shutdown")
            rtsp_client.teardown()
            rospy.signal_shutdown('rtsp client connect failed!')
        #init rosnode
        no_response = 0
        global temp_pub
        temp_pub = rospy.Publisher('/fixed/infrared/raw',Image,queue_size=1)
        rate = rospy.Rate(10)
        # cv bridge
        bridge = CvBridge()

        # 得到ndarray数据
        # arr = np.empty((4, 2, 3), dtype=np.float32)
        # print(arr)
        # # 打印字节串
        # b_arr = arr.tostring()
        # print(b_arr)
        # # 字节串转ndarray数据
        # arr_2 = np.frombuffer(b_arr, dtype=np.float32)
        # arr_2.shape = (4, 2, 3)
        # print(arr_2)

        while not rospy.is_shutdown():
            data = rtsp_client.get_temp()
            if data != None:
                no_response = 0
                data_array = data[4:]
                image_size = image_height * image_width * 4
                if len(data_array)!=image_size:
                    print("error size:",len(data_array),image_size)
                    continue
                print("ok size:",len(data_array))
                #publish temp data
                image_temp=Image()
                image_temp.height=image_height
                image_temp.width=image_width
                image_temp.encoding='32FC1'
                image_temp.data= str(data_array)

                # # 字节串转ndarray数据
                image_array = np.frombuffer(data_array, dtype=np.float32)
                # print(image_array[0],image_array[1],image_array[2],image_array[3])
                # image_array.shape = (image_height,image_width)
                # cv2.imwrite('/home/li/test.jpg', image_array)
                # image_temp.data = image_array.tobytes()
                temp_pub.publish(image_temp)
            else:
                no_response = no_response + 1
                if no_response > 600:
                    print(["no_response:",no_response])
                    no_response = 0
                    break
            rate.sleep()
        print("rtsp client teardown")
        rtsp_client.teardown()
    except KeyboardInterrupt:
        rtsp_client.teardown()
    except rospy.ROSInterruptException:
        rtsp_client.teardown()
        pass