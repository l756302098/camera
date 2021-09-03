#!/usr/bin/env python
#coding=utf-8
from genpy import message
import rospy
from time import sleep
import threading
import sys
if sys.version > '3':
    import queue as Queue
else:
    import Queue
from yd_cloudplatform.srv import CloudPlatControl,CloudPlatControlResponse
from yidamsg.msg import Detect_Result
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Point
from std_msgs.msg import Int32
from isapi import HK_Api
from diagnostic_msgs.msg import DiagnosticArray,DiagnosticStatus,KeyValue

pan = 0
tilt = 0
zoom = 0
pan_tartget = 0
tilt_tartget = 0
cmd_queue = Queue.Queue(maxsize=10)
api = HK_Api()
moving = False
err_level = 0
err_message = "ok"

def read_ptz(bq):
    print("read ptz")
    while True:
        #get current ptz
        t_pan, t_tilt, t_zoom = api.get_status(device_ip,1,device_username,device_password)
        if t_tilt > 0:
            t_tilt = 3600 - t_tilt
        elif t_tilt < 0:
            t_tilt = -1 * t_tilt
        global pan,tilt,zoom
        pan = t_pan
        tilt = t_tilt
        zoom = t_zoom
        sleep(0.1)

def write_ptz(bq):
    print("write ptz")
    while True:
        if bq.qsize() >= 1:
            rlist = bq.get()
            #resolve string
            if rlist is None:
                continue
            #level = rlist[0]
            print(rlist)
            print("cmd:",rlist[0])
            r = rlist[0].split('/')
            global pan,tilt,zoom
            pan_c = pan 
            tilt_c = tilt
            zoom_c = zoom
            id = int(r[0])
            action = int(r[1])
            ttype = int(r[2])
            value = int(r[3])
            if ttype == 0:
                pan_c = value / 10
            if ttype == 1:
                tilt_c = value / 10
            if ttype == 2:
                zoom_c = value
            if ttype == 3:
                pan_c = int(r[4]) / 10
                tilt_c = int(r[5]) / 10
            if ttype == 4:
                pan_c = int(r[4]) / 10
                tilt_c = int(r[5]) / 10
                zoom_c = int(r[6])
            if tilt_c < 1800:
                tilt_c = -1 * tilt_c
            elif tilt_c > 1800:
                tilt_c = 3600 - tilt_c
            ok = api.put_status(device_ip,pan_c,tilt_c,zoom_c,1,device_username,device_password)
            if ok:
                if tilt_c > 0:
                    tilt_c = 3600 - tilt_c
                elif tilt_c < 0:
                    tilt_c = -1 * tilt_c
                global pan_tartget,tilt_tartget
                pan_tartget = pan_c
                tilt_tartget = tilt_c
                global moving
                moving = True
                print("set ptz ok")
        sleep(0.1)

def handle_ptz(req):
    V = [req.action, req.type, req.value, req.allvalue]
    print(V)
    if req.action == 0:
        return CloudPlatControlResponse(0)
    elif req.action == 2:
        if req.type == 0:
            ok = api.put_continuous(device_ip,req.value,0,0,1,device_username,device_password)
        elif req.type == 0:
            ok = api.put_continuous(device_ip,0,req.value,0,1,device_username,device_password)
        if ok:
            print("continous success")
        return CloudPlatControlResponse(1)
    try:
        read_cmd = str(req.id) + "/" + str(req.action) + "/" + str(req.type) + "/" + str(req.value)
        #ready to move
        if req.type == 0:
            print("type 0")
            if req.value < 0  or req.value > 36000:
                return CloudPlatControlResponse(0)
        elif req.type == 1:
            print("type 1")
            if req.value < -36000  or req.value > 36000:
                return CloudPlatControlResponse(0)
        elif req.type == 2:
            print("type 2 zoom")
        elif req.type == 3:
            if req.allvalue[0] < 0  or req.allvalue[0] > 36000:
                return CloudPlatControlResponse(0)
            read_cmd = read_cmd + "/" + str(req.allvalue[0]) + "/" + str(req.allvalue[1])
        elif req.type == 4:
            read_cmd = read_cmd + "/" + str(req.allvalue[0]) + "/" + str(req.allvalue[1]) + "/" + str(req.allvalue[2])
        cmd_queue.put([read_cmd])
        return CloudPlatControlResponse(1)
    except Exception:
        return CloudPlatControlResponse(0)

def timer_callback(event):
    pub_heartbeat()
    #check is reach
    global moving
    if moving:
        pan_abs = abs(pan_tartget - pan)
        tilt_abs = abs(tilt_tartget - tilt)
        if pan_abs < 5 and tilt_abs < 5:
            global isreach_pub
            print("pub isreach")
            isreach_pub.publish(data=1)
            moving = False
	#print 'Timer called at:' + str(event.current_real)

def detect_callback(data):
    print("get detect callback",data)
    #calc left bottom
    x_min = (int)(data.xmin * 255 / 1920 )
    y_min = (int)((1080 - data.ymin) * 255 / 1080 )
    #calc right top
    x_off = (int)(data.xmax - data.xmin) * 255 / 1920
    y_off = (int)(data.ymax - data.ymin) * 255 / 1080
    print("get detect callback",x_min,y_min,x_min+x_off,y_min+y_off)
    ok = api.put_position(device_ip,x_min,y_min,x_min+x_off,y_min+y_off,1,device_username,device_password)
    if ok:
        print("set detect success")

def pub_heartbeat():
    #print(err_level,message)
    global heartbeat_pub
    msg = DiagnosticArray()
    msg.header.stamp = rospy.get_rostime()
    usage_stat = DiagnosticStatus()
    usage_stat.name = 'isapi_ptz_node'
    usage_stat.level = err_level
    usage_stat.hardware_id = ''
    usage_stat.message = err_message
    # usage_stat.values = [ KeyValue(key = 'Update Status', value = 'No Data' ),
    #                         KeyValue(key = 'Time Since Last Update', value = 'N/A') ]
    msg.status.append(usage_stat)
    heartbeat_pub.publish(msg)

if __name__ == '__main__':
    try:
        rospy.init_node('isapi_ptz_node')
        #init camera
        global device_ip,device_port,device_username,device_password
        device_ip = rospy.get_param("~device_ip")
        device_port = rospy.get_param("~device_port")
        device_username = rospy.get_param("~device_username")
        device_password = rospy.get_param("~device_password")
        print(device_ip,device_port,device_username,device_password)
        read_thread = threading.Thread(target = read_ptz,name='read_ptz_thread',args=(cmd_queue, ))
        read_thread.daemon = True
        read_thread.start()
        write_thread = threading.Thread(target = write_ptz,name='write_ptz_thread',args=(cmd_queue, ))
        write_thread.daemon = True
        write_thread.start()
        global isreach_pub,heartbeat_pub
        isreach_pub = rospy.Publisher('/yida/platform_isreach', Int32, queue_size=1)
        heartbeat_pub = rospy.Publisher('/yd/heartbeat', DiagnosticArray, queue_size=1)
        ptz_pub = rospy.Publisher('/yida/yuntai/position', Odometry, queue_size=1)
        ptz_server = rospy.Service('/yida/internal/platform_cmd', CloudPlatControl, handle_ptz)
        dect_sub = rospy.Subscriber("/detect_rect", Detect_Result, detect_callback)
        rospy.Timer(rospy.Duration(0.1),timer_callback)
        rate = rospy.Rate(20) # 20hz
        while not rospy.is_shutdown():
            #publish ptz attitude
            odom = Odometry()
            odom.header.stamp = rospy.Time.now()
            odom.header.frame_id = "map"
            odom.pose.pose.position = Point(pan * 10, zoom, tilt* 10)
            ptz_pub.publish(odom)
            rate.sleep()
    except rospy.ROSInterruptException:
        pass
