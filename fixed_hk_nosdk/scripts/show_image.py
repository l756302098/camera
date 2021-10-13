#!/usr/bin/env python
#coding:utf-8
import rospy
from sensor_msgs.msg import Image
import cv2
from cv_bridge import CvBridge, CvBridgeError
import numpy as np

class Image_Receiver:
    def __init__(self):
        rospy.Subscriber('/image/test', Image, callback=self.image_rgb_callback, queue_size=1)
        self.cv_bridge = CvBridge()
        rospy.spin()

    def image_rgb_callback(self, data):
        try:
            print(data.encoding,len(data.data))
            cv_image = self.cv_bridge.imgmsg_to_cv2(data,data.encoding)
        except CvBridgeError as e:
            print(e)
        print(cv_image.shape)
        #print(cv_image)
        #print(image_array.shape)
        #image_array.shape = (data.height,data.width)
        cv2.resizeWindow("Window", 1920, 1080)
        cv2.imshow("Window", cv_image)
        cv2.waitKey(3)

if __name__ == '__main__':
    rospy.init_node('show_image_node')
    Image_Receiver()