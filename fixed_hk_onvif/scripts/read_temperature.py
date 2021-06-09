#!/usr/bin/env python
import rospy
import requests
import json
from std_msgs.msg import Header
from sensor_msgs.msg import Image
from requests.auth import HTTPDigestAuth
import os 
from io import BytesIO
import sys
print("python version:",sys.version)

def timer_callback(event):
    count = 1
	#print 'Timer called at:' + str(event.current_real)

if __name__ == '__main__':
    try:
        #init camera
        rospy.init_node('raw_temp_node', anonymous=True)
        device_ip = rospy.get_param("/raw_temp_node/device_ip")
        device_port = rospy.get_param("/raw_temp_node/device_port")
        device_username = rospy.get_param("/raw_temp_node/device_username")
        device_password = rospy.get_param("/raw_temp_node/device_password")
        url = rospy.get_param("/raw_temp_node/url")
        show_image = rospy.get_param("/raw_temp_node/show_image")
        image_width = 0
        image_height = 0
        print(device_ip,device_port,device_username,device_password,url)
       
        global temp_pub
        temp_pub = rospy.Publisher('/fixed/infrared/raw',Image,queue_size=1)
        rospy.Timer(rospy.Duration(0.1),timer_callback)
        rate = rospy.Rate(5)
        while not rospy.is_shutdown():
            #publish image
            try:
                resp = requests.get(url,auth=HTTPDigestAuth(device_username,device_password))
            except Exception,err:
                print("err:",err)
            print("code:",resp.status_code)
            if resp.status_code == 200:
                multipart = resp.content
                form_list = multipart.split("--boundary")
                #print("len:",len(form_list))
                for content in form_list:
                    content_list = content.split("\r\n")
                    #print("size:",len(content_list))
                    if "application/json" in content:
                        print("contain application/json")
                        content_len = len(content_list)
                        json_data = content_list[content_len - 2]
                        text = json.loads(json_data)
                        if text:
                            print(text)
                            #print(text["JpegPictureWithAppendData"]["jpegPicWidth"])
                            #print(text["JpegPictureWithAppendData"]["jpegPicHeight"])
                            image_width = text["JpegPictureWithAppendData"]["jpegPicWidth"]
                            image_height = text["JpegPictureWithAppendData"]["jpegPicHeight"]
                        # for index in range(content_len):
                        #     line = content_list[index]
                        #     print(line)
                    if "image/pjpeg" in content:
                        print("contain image/pjpeg")
                        if  not show_image:
                            continue
                        data_index = len(content_list)-2
                        data_array = content_list[data_index]
                    if "application/octet-stream" in content:
                        print("contain application/octet-stream")
                        data_index = len(content_list)-2
                        data_array = content_list[data_index]
                        print("len:",len(data_array))
                        print("type:",type(data_array))
                        #print(data_array)
                        # b = BytesIO(data_array)
                        # print("b type:",type(b))
                        # print("getvalue:",b.getvalue())
                        # print("str array len",len(data_array))
                        image_temp=Image()
                        image_temp.height=image_height
                        image_temp.width=image_width
                        image_temp.encoding='rgb8'
                        image_temp.data=data_array
                        temp_pub.publish(image_temp)
            rate.sleep()
    except rospy.ROSInterruptException:
        pass
