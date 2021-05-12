#!/usr/bin/env python

import rospy
from std_msgs.msg import String
from time import sleep
import math
from onvif import ONVIFCamera
import threading
from fixed_msg.srv import cp_control,cp_controlResponse

CX = 0
CY = 0
CZ = 0
XMAX = 1
XMIN = -1
YMAX = 1
YMIN = -1
mycam = ONVIFCamera('192.168.1.131', 80, 'admin', '123qweasd')
media = mycam.create_media_service()
ptz = mycam.create_ptz_service()
request = ptz.create_type('GetConfigurationOptions')

def get_ptz():
    mycam = ONVIFCamera('192.168.1.66', 80, 'admin', 'abcd1234')
    media = mycam.create_media_service()
    ptz = mycam.create_ptz_service()
    request = ptz.create_type('GetStatus')
    media_profile = media.GetProfiles()[0]
    request.ProfileToken = media_profile._token
    while 1:
        try:
            res = ptz.GetStatus(request)
            #print(res)
            rospy.loginfo(res.Position)
            # print(res.Position)
            # print(res.Position[0]._y)
            sleep(0.1)
        except Exception:
            pass
def perform_move(ptz, request, timeout):
    # Start continuous move
    ptz.ContinuousMove(request)
    # Wait a certain time
    sleep(timeout)
    # Stop continuous move
    ptz.Stop({'ProfileToken': request.ProfileToken})

def move_right(ptz, request, timeout=1):
    print 'move right...'
    request.Velocity.PanTilt._x = XMAX
    request.Velocity.PanTilt._y = 0
    perform_move(ptz, request, timeout)

def continuous_move():
    # move right
    move_right(ptz, request)

def onvif_init():
    global mycam,media,ptz,request
    # Get target profile
    media_profile = media.GetProfiles()[0]
    # Get PTZ configuration options for getting continuous move range
    request.ConfigurationToken = media_profile.PTZConfiguration._token
    ptz_configuration_options = ptz.GetConfigurationOptions(request)

    request = ptz.create_type('ContinuousMove')
    request.ProfileToken = media_profile._token
    ptz.Stop({'ProfileToken': media_profile._token})

    # Get range of pan and tilt
    # NOTE: X and Y are velocity vector
    global XMAX, XMIN, YMAX, YMIN
    XMAX = ptz_configuration_options.Spaces.ContinuousPanTiltVelocitySpace[0].XRange.Max
    XMIN = ptz_configuration_options.Spaces.ContinuousPanTiltVelocitySpace[0].XRange.Min
    YMAX = ptz_configuration_options.Spaces.ContinuousPanTiltVelocitySpace[0].YRange.Max
    YMIN = ptz_configuration_options.Spaces.ContinuousPanTiltVelocitySpace[0].YRange.Min

def getStatus():
    status_req = ptz.create_type('GetStatus')
    media_profile = media.GetProfiles()[0]
    status_req.ProfileToken = media_profile._token
    res = ptz.GetStatus(status_req)
    #print(res)
    print(res.Position[0]._x)
    print(res.Position[0]._y)

def absolute_move(x,y,z):
    print("absolute_move",x,y,z)
    media_profile = media.GetProfiles()[0]
    move_req = ptz.create_type('AbsoluteMove')
    move_req.ProfileToken = media_profile._token
    ptz.Stop({'ProfileToken': media_profile._token})
    
    status = ptz.GetStatus({'ProfileToken': media_profile._token})
    print status
    status.Position.PanTilt._x = x
    status.Position.PanTilt._y = y
    status.Position.Zoom._x = z
    move_req.Position = status.Position
    ptz.AbsoluteMove(move_req)
    print("move finish")

def handle_ptz(req):
    print("handle_ptz")
    V = [req.action, req.type, req.value, req.allvalue]
    print(V)
    if req.action == 0:
        return cp_controlResponse(0)
    #ready to move
    if req.type == 0:
        print("type 0")
        absolute_move(0,req.value,0)
    elif req.type == 1:
        print("type 1")
        absolute_move(0,req.value,0)
    return cp_controlResponse(1)

if __name__ == '__main__':
    try:
        onvif_init()
        thread = threading.Thread(target = get_ptz)
        thread.daemon = True
        thread.start()
        absolute_move(0,0,0.5)
        sleep(5)
        absolute_move(0.5, 0, 0.3)
        sleep(3)
        absolute_move(0, 0, 0.5)
        pub = rospy.Publisher('chatter', String, queue_size=10)
        ptz_server = rospy.Service('/fixed/platform/cmd', cp_control, handle_ptz)
        rospy.init_node('onvif_control_node', anonymous=True)
        rate = rospy.Rate(10) # 10hz
        while not rospy.is_shutdown():
            hello_str = "hello world %s" % rospy.get_time()
            #rospy.loginfo(hello_str)
            pub.publish(hello_str)
            rate.sleep()
            #continuous_move()
    except rospy.ROSInterruptException:
        pass