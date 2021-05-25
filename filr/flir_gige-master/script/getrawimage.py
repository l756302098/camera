#!/usr/bin/env python
# -*- coding: utf-8 -*-

import thread
import rospy
import cv2
import numpy as np

from sensor_msgs.msg import CompressedImage

flag = 0

def callback(data):
	np_arr = np.fromstring(data.data, np.uint8)
	global image_show
	image_show = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)
	global flag 
	flag = 1

def fun(no, flag):
	rospy.Subscriber("thermal/image_raw/compressed", CompressedImage, callback)
	rospy.spin()

if __name__ == '__main__':
	flag1 = 1
	rospy.init_node('getimage', anonymous=True)
	thread.start_new_thread(fun, (2,flag1))
	while not rospy.is_shutdown():
		if not flag:
			#print "flag", flag
			pass
		else:
			cv2.imshow("image_show", image_show)
			k=cv2.waitKey(1)
			if k==27:
				cv2.destroyAllWindows()
				break