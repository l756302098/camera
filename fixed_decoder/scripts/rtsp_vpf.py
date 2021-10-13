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

loop = False

def decode(gpuID, encFilePath):
    image_pub = rospy.Publisher('/image/test',Image,queue_size=1)
    nvDec = nvc.PyNvDecoder(encFilePath, gpuID, {'rtsp_transport': 'tcp', 'max_delay': '5000000', 'fmt': 'YUV420'})
    frameSize = nvDec.Framesize()
    w = nvDec.Width()
    h = frameSize/w
    print(frameSize,w,h)
    rawFrameNV12 = np.ndarray([0], dtype=np.uint8)
    cv_bridge = CvBridge()
    loop = True
    while not rospy.is_shutdown():
        if not loop:
            break
        success = nvDec.DecodeSingleFrame(rawFrameNV12)
        if not success:
            print('No more video frames.')
            continue
        print('success')
        yuv_img = rawFrameNV12.reshape(int(h), w)
        #print("image size:",type(yuv_img),len(yuv_img))
        img = cv2.cvtColor(yuv_img, cv2.COLOR_YUV2BGR_I420)
        #print("cv size:",type(img),img.size)
        # try:
        #     pass
        #     #ros_image = image_pub.publish(cv_bridge.cv2_to_imgmsg(img, "bgr8"))
        # except CvBridgeError as e:
        #     print(e)
        cv2.imshow('img', img)
        cv2.waitKey(1)

if __name__ == '__main__':
    try:
        rospy.init_node('vpf_decode_node')
        nodename = rospy.get_name()
        rospy.loginfo("%s started" % nodename)
        gpu_id = 0
        rtsp_url = 'rtsp://admin:123qweasd@192.168.1.68:554/h264/ch1/main/av_stream'
        #decode(gpu_id, "/home/li/Videos/e905f514-f377-411e-8e51-6b16a3c9_6750.mp4")
        decode(gpu_id, rtsp_url)
        loop = False
    except KeyboardInterrupt:
        loop = False
        pass