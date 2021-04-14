'''
Descripttion: 
version: 
Author: li
Date: 2021-04-08 15:18:13
LastEditors: li
LastEditTime: 2021-04-08 17:55:32
'''
#!/usr/bin/env python

from time import sleep
from onvif import ONVIFCamera
import threading

XMAX = 1
XMIN = -1
YMAX = 1
YMIN = -1
mycam = ONVIFCamera('192.168.1.66', 80, 'admin', 'abcd1234')
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
            print(res.Position[0]._x)
            print(res.Position[0]._y)
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

def absolute_move():
    print("absolute_move")
    status_req = ptz.create_type('GetStatus')
    move_req = ptz.create_type('AbsoluteMove')
    media_profile = media.GetProfiles()[0]
    status_req.ProfileToken = media_profile._token
    move_req.ProfileToken = media_profile._token
    if move_req.Position is None:
        move_req.Position = ptz.GetStatus(status_req).Position
    if move_req.Speed is None:
        move_req.Speed = ptz.GetStatus(status_req).Speed
    move_req.Position.PanTilt._x = 0
    move_req.Speed.PanTilt._x = 1
    move_req.Position.PanTilt._y = 0
    move_req.Speed.PanTilt._y = 1
    
    res = ptz.AbsoluteMove(move_req)
    print("move finish")
    print(res)