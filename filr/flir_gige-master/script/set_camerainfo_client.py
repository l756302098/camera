#!/usr/bin/env python
# *-* coding:utf-8 *-*

import sys
import rospy

from sensor_msgs.msg import CameraInfo
from sensor_msgs.msg import RegionOfInterest
from sensor_msgs.srv import SetCameraInfo

def set_camera_info_client(camerainfo):
    rospy.wait_for_service('/thermal/set_camera_info')
    try:
        set_camera_info = rospy.ServiceProxy('/thermal/set_camera_info', SetCameraInfo)
        set_camera_info(camerainfo)
        pass
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

if __name__ == "__main__":
    camerainfo = CameraInfo()
    camerainfo.height = 256
    camerainfo.width = 320
    RoiInfo = RegionOfInterest()
    RoiInfo.x_offset = 20
    RoiInfo.y_offset = 20
    RoiInfo.height = 10
    RoiInfo.width = 10
    RoiInfo.do_rectify = True
    camerainfo.roi = RoiInfo
    set_camera_info_client(camerainfo)
    