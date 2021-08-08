#!/usr/bin/env python
#coding:utf-8
from time import sleep
from rtsp_client import RtspClient,config_dict
from isapi import HK_Api

if __name__ == '__main__':
    try:
        print("obs start ...")
        api = HK_Api()
        #read param
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
        #init rtsp client
        rtsp_client = RtspClient()
        succ = rtsp_client.connect_rtsp_server()
        if not succ:
            print("ros node shutdown")
            rtsp_client.teardown()
            rospy.signal_shutdown('rtsp client connect failed!')
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
                print("ok size:",len(data_array))
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
    except rospy.ROSInterruptException:
        rtsp_client.teardown()
        pass