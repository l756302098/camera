#!/usr/bin/env python
#coding:utf-8
import rospy
from time import sleep
from sensor_msgs.msg import Image
from rtsp_client import RtspClient,config_dict

if __name__ == '__main__':
    try:
        rospy.init_node('rtsp_temp_node')
        nodename = rospy.get_name()
        rospy.loginfo("%s started" % nodename)
        #read param
        device_ip = rospy.get_param('~device_ip',"127.0.0.1")
        device_port = rospy.get_param("~device_port",80)
        device_username = rospy.get_param("~device_username","admin")
        device_password = rospy.get_param("~device_password","123qweasd")
        image_width = rospy.get_param("~image_width",1920)
        image_height = rospy.get_param("~image_height",1080)
        print(device_ip,device_port,device_username,device_password)
        print(config_dict["server_ip"])
        config_dict["server_ip"] = device_ip
        print(config_dict["server_ip"])
        config_dict["server_port"] = device_port
        config_dict["server_username"] = device_username
        config_dict["server_password"] = device_password
        #init rtsp client
        rtsp_client = RtspClient()
        rtsp_client.exec_full_request()
        #init rosnode
        global temp_pub
        temp_pub = rospy.Publisher('/fixed/infrared/raw',Image,queue_size=1)
        rate = rospy.Rate(5) # 30hz
        while not rospy.is_shutdown():
            data = rtsp_client.get_temp()
            if data != None:
                #print(type(data))
                data_array = data[4:]
                #print(type(data_array))
                image_size = image_height * image_width * 4
                if len(data_array)!=image_size:
                    print("error size:",len(data_array))
                    continue
                #publish temp data
                image_temp=Image()
                image_temp.height=image_height
                image_temp.width=image_width
                image_temp.encoding='rgb8'
                image_temp.data= str(data_array)
                temp_pub.publish(image_temp)
            rate.sleep()
        print("rtsp client teardown")
        rtsp_client.teardown()
    except KeyboardInterrupt:
        rtsp_client.teardown()
    except rospy.ROSInterruptException:
        rtsp_client.teardown()
        pass