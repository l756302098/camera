#!/usr/bin/env python

import sys
import rospy
from temperature.srv import *

def get_temper(gettemper):
    y = 0.0
    rospy.wait_for_service('/thermal/get_temperature')
    try:
        GetTemperature_fun = rospy.ServiceProxy('/thermal/get_temperature', GetTemperature)
        gettemper=GetTemperature_fun(gettemper.spot_x,gettemper.spot_y)
        print gettemper.temperature_c
    except rospy.ServiceException, e:
        print "Service call failed: %s"%e

if __name__ == "__main__":
    gettemper = GetTemperature()
    x = [130,190]
    y = [100, 140]
    gettemper.spot_x = x
    gettemper.spot_y = y
    #get_temper(gettemper)
    GetTemperature_fun = rospy.ServiceProxy('/thermal/get_temperature', GetTemperature)
    gettemper=GetTemperature_fun(gettemper.spot_x,gettemper.spot_y)
    print gettemper.temperature_c