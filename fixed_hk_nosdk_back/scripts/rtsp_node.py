#!/usr/bin/env python
#coding:utf-8
import rospy
from time import sleep
from sensor_msgs.msg import Image
from std_msgs.msg import Header
from rtsp_client import RtspClient,config_dict
from isapi import HK_Api
import time

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
        xml_path = rospy.get_param("~xml_path","")
        print(device_ip,device_port,device_username,device_password,xml_path)
        config_dict["server_ip"] = device_ip
        config_dict["server_port"] = device_port
        config_dict["server_username"] = device_username
        config_dict["server_password"] = device_password
        #check param
        print("start check temp data lenght")
        check = True
        while(check):
            length,ok = api.get_pixelToPixelParam(device_ip,1,device_username,device_password)
            if ok :
                if length == '2':
                    check = False
                else:
                    ok = api.put_pixelToPixelParam(xml_path,device_ip,1,device_username,device_password)
            sleep(1)
        print("end check temp data lenght")
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
        while not rospy.is_shutdown():
            data = rtsp_client.get_temp()
            if data != None:
                no_response = 0
                time_array = data[0:16]
                # timeStr = ":".join("{:02x}".format(c) for c in time_array)
                # print("timeStr",timeStr)
                # years/mouth/week/day/hour/minutes/second/mils
                years = (time_array[0] << 8) + time_array[1]
                mouth = (time_array[2] << 8) + time_array[3]
                week = (time_array[4] << 8) + time_array[5]
                day = (time_array[6] << 8) + time_array[7]
                hour = (time_array[8] << 8) + time_array[9]
                minutes = (time_array[10] << 8) + time_array[11]
                second = (time_array[12] << 8) + time_array[13]
                mils = (time_array[14] << 8) + time_array[15]
                timestampFormat = "{:0>4d}-{:0>2d}-{:0>2d} {:0>2d}:{:0>2d}:{:0>2d}".format(years,mouth,day,hour,minutes,second)
                # print("timestamp format",timestampFormat)
                timeArray = time.strptime(timestampFormat, "%Y-%m-%d %H:%M:%S")
                timeStamp = int(time.mktime(timeArray))
                # timeStamp = timeStamp*1000 + mils
                # print("timeStamp:",timeStamp)
                data_array = data[20:]
                image_size = image_height * image_width * 2
                if len(data_array)!=image_size:
                    print("error size:",len(data_array))
                    continue
                # print("ok size:",len(data_array))
                #publish temp data
                image_temp=Image()
                header = Header()
                header.stamp.secs = timeStamp
                header.stamp.nsecs = mils * 1000000
                image_temp.header = header
                image_temp.height=image_height
                image_temp.width=image_width
                image_temp.encoding='rgb8'
                image_temp.data= str(data_array)
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
        check = False
        rtsp_client.teardown()
    except rospy.ROSInterruptException:
        rtsp_client.teardown()
        pass