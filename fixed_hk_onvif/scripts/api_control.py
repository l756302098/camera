#!/usr/bin/env python
#coding=utf-8
import rospy
from std_msgs.msg import String
from time import sleep
import math
import threading
import Queue
from fixed_msg.srv import cp_control,cp_controlResponse
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Point, Pose, Quaternion, Twist, Vector3
from std_msgs.msg import Int32
from isapi import HK_Api
pan = 0
tilt = 0
zoom = 0
cmd_queue = Queue.PriorityQueue(maxsize=10)
api = HK_Api()

def read_ptz(bq):
    print("read ptz")
    while True:
        #get current ptz
        t_pan, t_tilt, t_zoom = api.get_status()
        global pan,tilt,zoom,status_pt,status_z
        pan = t_pan
        tilt = t_tilt
        zoom = t_zoom
        sleep(0.1)
        #check platform isreach

def write_ptz(bq):
    print("write ptz")
    while True:
        if bq.qsize() >= 1:
            rlist = bq.get()
            #resolve string
            if rlist is None:
                continue
            level = rlist[0]
            r = rlist[1].split('/')
            print("level:",level,"cmd:",rlist[1])
            global pan,tilt,zoom
            id = int(r[0])
            action = int(r[1])
            ttype = int(r[2])
            value = int(r[3])
            if ttype == 0:
                pan = value / 100
            if ttype == 1:
                tilt = value / 100
            if ttype == 2:
                zoom = value / 100
            if ttype == 3:
                pan = int(r[4]) / 100
                tilt = int(r[5]) / 100
            if ttype == 4:
                pan = int(r[4]) / 100
                tilt = int(r[5]) / 100
                zoom = int(r[6]) / 100
            #set_camera(ttype,pan,tilt,zoom)
        sleep(0.1)

def handle_ptz(req):
    print("handle_ptz")
    V = [req.action, req.type, req.value, req.allvalue]
    print(V)
    if req.action == 0:
        return cp_controlResponse(0)
    read_cmd = str(req.id) + "/" + str(req.action) + "/" + str(req.type) + "/" + str(req.value)
    #ready to move
    if req.type == 0:
        print("type 0")
        if req.value < 0  or req.value > 36000:
            return cp_controlResponse(0)
    elif req.type == 1:
        print("type 1")
        if req.value < 0  or req.value > 36000:
            return cp_controlResponse(0)
    elif req.type == 2:
        print("type 2 zoom")
    elif req.type == 3:
        read_cmd = read_cmd + "/" + str(req.allvalue[0]) + "/" + str(req.allvalue[1])
    elif req.type == 4:
        read_cmd = read_cmd + "/" + str(req.allvalue[0]) + "/" + str(req.allvalue[1]) + "/" + str(req.allvalue[2])
    cmd_queue.put([1,read_cmd])
    return cp_controlResponse(1)

def timer_callback(event):
    count = 1
	#print 'Timer called at:' + str(event.current_real)

if __name__ == '__main__':
    try:
        #init camera
        device_ip = rospy.get_param("/control_onvif/device_ip")
        device_port = rospy.get_param("/control_onvif/device_port")
        device_username = rospy.get_param("/control_onvif/device_username")
        device_password = rospy.get_param("/control_onvif/device_password")
        print(device_ip,device_port,device_username,device_password)
        read_thread = threading.Thread(target = read_ptz,name='read_ptz_thread',args=(cmd_queue, ))
        read_thread.daemon = True
        read_thread.start()
        write_thread = threading.Thread(target = write_ptz,name='write_ptz_thread',args=(cmd_queue, ))
        write_thread.daemon = True
        write_thread.start()
        global isreach_pub
        isreach_pub = rospy.Publisher('/fixed/platform/isreach', Int32, queue_size=1)
        ptz_pub = rospy.Publisher('/fixed/platform/position', Odometry, queue_size=1)
        ptz_server = rospy.Service('/fixed/platform/cmd', cp_control, handle_ptz)
        rospy.init_node('isapi_control_node', anonymous=True)
        rospy.Timer(rospy.Duration(0.1),timer_callback)
        rate = rospy.Rate(30) # 30hz
        while not rospy.is_shutdown():
            #publish ptz attitude
            odom = Odometry()
            odom.header.stamp = rospy.Time.now()
            odom.header.frame_id = "map"
            # odom.pose.pose.position = Point(pan * 100, zoom* 100, tilt* 100)
            # ptz_pub.publish(odom)
            rate.sleep()
    except rospy.ROSInterruptException:
        pass