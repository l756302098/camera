#!/usr/bin/env python
# *-* coding:utf-8 *-*

import sys
import rospy

from temperature.srv import *

def set_temperature_client(temperature_spot):
    rospy.wait_for_service('/thermal/get_temperature')
    try:
        set_tempspot_info = rospy.ServiceProxy('/thermal/get_temperature', GetTemperature)
        spot_x = [112, 134]
        spot_y = [157, 173]
        #spot_x = [163]
        #spot_y = [121]
        resp = set_tempspot_info(spot_x,spot_y)
        return resp.temperature_c
        pass
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

if __name__ == "__main__":
    temperature_spot = GetTemperature()
    temperature_spot.spot_x = [100, 110]
    temperature_spot.spot_y = [105, 105]
    print "temperature = %s" % set_temperature_client(temperature_spot)
    #set_temperature_client(temperature_spot)
    
    
