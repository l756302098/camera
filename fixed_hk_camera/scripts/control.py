#!/usr/bin/env python

import rospy
from std_msgs.msg import String
from time import sleep
import math
import threading
import queue
from onvif_sdk import camera
from fixed_msg.srv import cp_control,cp_controlResponse
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Point, Pose, Quaternion, Twist, Vector3

camera_object = camera('192.168.1.66', 80, 'admin', 'abcd1234', 0, 0, -0.3, 0, 0, 0 )
cmd_queue = queue.PriorityQueue(maxsize=10)
device_id = 1
#ptz status
yaw = 0
pitch = 0
zoom = 0

yaw_goal = 0
pitch_goal = 0
zoom_goal = 0

def set_camera(type,value,xy_value,z_value,zoom_value):
    print("set camera")

def read_ptz(bq):
    print("read ptz")
    while True:
        if bq.qsize() < 8:
            read_cmd = str(device_id) + "/0/0/0"
            bq.put([5,read_cmd])
            read_cmd = str(device_id) + "/0/1/0"
            bq.put([5,read_cmd])
        sleep(0.1)

def write_ptz(bq):
    print("write ptz")
    while True:
        xy_value = 0
        z_value = 0
        zoom_value = 0
        if bq.qsize() >= 1:
            rlist = bq.get()
            #resolve string
            if rlist is None:
                continue
            level = rlist[0]
            r = rlist[1].split('/')
            print("level:",level,"cmd:",rlist[1])
            id = int(r[0])
            action = int(r[1])
            ttype = int(r[2])
            value = int(r[3])
            if ttype == 0:
                xy_value = value
                if level == 1:
                    yaw_goal = value
            if ttype == 1:
                z_value = value
                if level == 1:
                    pitch_goal = value
            if ttype == 2:
                zoom_value = value
                if level == 1:
                    zoom_goal = value
            if ttype == 3:
                xy_value = int(r[4])
                z_value = int(r[5])
                if level == 1:
                    yaw_goal = xy_value
                    pitch_goal = z_value
            if ttype == 4:
                xy_value = int(r[4])
                z_value = int(r[5])
                zoom_value = int(r[6])
                if level == 1:
                    yaw_goal = xy_value
                    pitch_goal = z_value
                    zoom_goal = zoom_value
            set_camera(ttype,value,xy_value,z_value,zoom_value)
        sleep(0.1)

def update_ptz_status():
    print("update ptz status")

def handle_ptz(req):
    print("handle_ptz")
    V = [req.action, req.type, req.value, req.allvalue]
    print(V)
    if req.action == 0:
        return cp_controlResponse(0)
    read_cmd = str(req.id) + "/" + str(req.action) + "/" + str(req.type) + "/" + str(req.value)
    #ready to move
    if req.type == 3:
        read_cmd = read_cmd + "/" + str(req.allvalue[0]) + "/" + str(req.allvalue[1])
    elif req.type == 1:
        read_cmd = read_cmd + "/" + str(req.allvalue[0]) + "/" + str(req.allvalue[1]) + "/" + str(req.allvalue[2])
    bq.put([1,read_cmd])
    return cp_controlResponse(1)

def timer_callback(event):
    count = 1
	#print 'Timer called at:' + str(event.current_real)

if __name__ == '__main__':
    try:
        #init camera
        read_thread = threading.Thread(target = read_ptz,name='read_ptz_thread',args=(cmd_queue, ))
        read_thread.daemon = True
        read_thread.start()
        write_thread = threading.Thread(target = write_ptz,name='write_ptz_thread',args=(cmd_queue, ))
        write_thread.daemon = True
        write_thread.start()
        # update_thread = threading.Thread(target = update_ptz_status,name='update_ptz_status_thread',args=None)
        # update_thread.daemon = True
        # update_thread.start()
        ptz_pub = rospy.Publisher('/fixed/yuntai/position', Odometry, queue_size=1)
        ptz_server = rospy.Service('/fixed/internal/platform_cmd', cp_control, handle_ptz)
        rospy.init_node('onvif_control_node', anonymous=True)
        rospy.Timer(rospy.Duration(0.1),timer_callback)
        rate = rospy.Rate(30) # 30hz
        while not rospy.is_shutdown():
            #print(rospy.get_time())
            #publish ptz attitude
            odom = Odometry()
            odom.header.stamp = rospy.Time.now()
            odom.header.frame_id = "map"
            odom.pose.pose.position = Point(yaw, 0, pitch)
            ptz_pub.publish(odom)
            rate.sleep()
    except rospy.ROSInterruptException:
        pass