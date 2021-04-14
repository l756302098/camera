#!/usr/bin/env python

import rospy
from std_msgs.msg import String
from time import sleep
import math
import threading
from onvif_sdk import camera
from fixed_msg.srv import cp_control,cp_controlResponse

def get_ptz():
    print("get ptz")

def handle_ptz(req):
    print("handle_ptz")
    V = [req.action, req.type, req.value, req.allvalue]
    print(V)
    if req.action == 0:
        return cp_controlResponse(0)
    #ready to move
    if req.type == 0:
        print("type 0")
    elif req.type == 1:
        print("type 1")    
    return cp_controlResponse(1)

if __name__ == '__main__':
    try:
        #init camera
        thread = threading.Thread(target = get_ptz)
        thread.daemon = True
        thread.start()
        pub = rospy.Publisher('chatter', String, queue_size=10)
        ptz_server = rospy.Service('/fixed/internal/platform_cmd', cp_control, handle_ptz)
        rospy.init_node('onvif_control_node', anonymous=True)
        rate = rospy.Rate(30) # 30hz
        while not rospy.is_shutdown():
            hello_str = "hello world %s" % rospy.get_time()
            #print(rospy.get_time())
            #rospy.loginfo(hello_str)
            pub.publish(hello_str)
            rate.sleep()
    except rospy.ROSInterruptException:
        pass