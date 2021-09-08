#!/usr/bin/env python
#coding:utf-8
import rospy
import requests
import json
from std_msgs.msg import Header
from sensor_msgs.msg import Image
from requests.auth import HTTPDigestAuth
import os 
from io import BytesIO
import sys
import cv2
import datetime
import time
print("python version:",sys.version)

if __name__ == '__main__':
    try:
        #init camera
        rospy.init_node('temp_time_node', anonymous=True)
        device_ip = rospy.get_param("/temp_time_node/device_ip")
        device_port = rospy.get_param("/temp_time_node/device_port")
        device_username = rospy.get_param("/temp_time_node/device_username")
        device_password = rospy.get_param("/temp_time_node/device_password")
        url = rospy.get_param("/temp_time_node/url")
        show_image = rospy.get_param("/temp_time_node/show_image")
        print(device_ip,device_port,device_username,device_password,url)
        ntp_year = ntp_month = ntp_day = ntp_hour = ntp_minute = ntp_second = ntp_milliSecond = timestamp_secs = timestamp_nsecs = 0
        image_width = image_height = 0
        global temp_pub
        temp_pub = rospy.Publisher('/fixed/infrared/raw',Image,queue_size=1)
        rate = rospy.Rate(5)
        while not rospy.is_shutdown():
            try:
                resp = requests.get(url,auth=HTTPDigestAuth(device_username,device_password))
            except Exception,err:
                print("err:",err)
            #print("code:",resp.status_code)
            if resp.status_code == 200:
                multipart = resp.content
                form_list = multipart.split("--boundary")
                for content in form_list:
                    content_list = content.split("\r\n")
                    if "application/json" in content:
                        #print("contain application/json")
                        content_len = len(content_list)
                        json_data = content_list[content_len - 1]
                        text = json.loads(json_data)
                        if text:
                            image_width = text["JpegPictureWithAppendData"]["jpegPicWidth"]
                            image_height = text["JpegPictureWithAppendData"]["jpegPicHeight"]
                            time_text = text["JpegPictureWithAppendData"]["stAbsTime"]
                            ntp_year = time_text["year"]
                            ntp_month = time_text["month"]
                            ntp_day = time_text["day"]
                            ntp_hour = time_text["hour"]
                            ntp_minute = time_text["minute"]
                            ntp_second = time_text["second"]
                            ntp_milliSecond = time_text["milliSecond"]
                            dt = datetime.datetime(year = ntp_year,month = ntp_month,day = ntp_day,hour=ntp_hour,minute=ntp_minute,second=ntp_second,microsecond=ntp_milliSecond)
                            timestamp_secs = int(time.mktime(dt.timetuple()))
                            timestamp_nsecs = ntp_milliSecond * 1000000
                            #print(timestamp_secs,timestamp_nsecs)
                    if "image/pjpeg" in content:
                        #print("contain image/pjpeg")
                        if  not show_image:
                            continue
                        data_index = len(content_list)-2
                        data_array = content_list[data_index]
                        try:
                            is_write = False
                            with open("/usr/local/temp.jpg", "wb") as f:
                                f.write(data_array)
                                f.close()
                                is_write = True
                            if is_write:
                                lena = cv2.imread('/usr/local/temp.jpg')
                                cv2.imshow('image', lena)
                                cv2.waitKey(1)
                        except Exception,e:
                            print("cv error",e)
                    if "application/octet-stream" in content:
                        #print("contain application/octet-stream")
                        data_index = len(content_list)-2
                        data_array = content_list[data_index]
                        image_temp=Image()
                        header = Header()
                        header.stamp.secs = timestamp_secs
                        header.stamp.nsecs = timestamp_nsecs
                        image_temp.header = header
                        image_temp.height=image_height
                        image_temp.width=image_width
                        image_temp.encoding='32FC1'
                        image_temp.data=data_array
                        temp_pub.publish(image_temp)
            rate.sleep()
    except rospy.ROSInterruptException:
        pass
