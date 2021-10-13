#!/usr/bin/env python
#coding:utf-8
import rospy
from PIL import Image as PIamge
from sensor_msgs.msg import Image
import cv2
from cv_bridge import CvBridge, CvBridgeError
import numpy as np
import sys
sys.path.append('/home/li/nvidia/VideoProcessingFramework/install/bin')
print('Module Search Path:')
print(sys.path)

import PyNvCodec as nvc

nvdec = nvc.PyNvDecoder(int(1920), int(1080), nvc.PixelFormat.RGB, 
                        nvc.CudaVideoCodec.H264, 0)

raw_frame = np.ndarray(shape=(0), dtype=np.uint8)

class Image_Receiver:
    def __init__(self):
        rospy.Subscriber('/fixed/visible/h264', Image, callback=self.h264_callback, queue_size=1)
        self.image_pub = rospy.Publisher('/image/test',Image,queue_size=1)
        self.cv_bridge = CvBridge()
        rospy.spin()

    def h264_callback(self, data):
        try:
            print(data.encoding,len(data.data))
            enc_packet = np.frombuffer(buffer=data.data, dtype=np.uint8)
            
        except CvBridgeError as e:
            print(e)
       
        #print(image_array.shape)
        #image_array.shape = (data.height,data.width)
        # cv2.imshow("Image Window", cv_image)
        # cv2.waitKey(3)

if __name__ == '__main__':
    try:
        rospy.init_node('vpf_decode_node')
        nodename = rospy.get_name()
        rospy.loginfo("%s started" % nodename)
        Image_Receiver()
    except KeyboardInterrupt:
        pass
    except rospy.ROSInterruptException:
        pass