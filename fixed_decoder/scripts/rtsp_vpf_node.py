#!/usr/bin/env python
#coding:utf-8
import rospy
from PIL import Image as PIamge
from sensor_msgs.msg import Image
from cv_bridge import CvBridge, CvBridgeError
import numpy as np
import sys
sys.path.append('/home/li/nvidia/VideoProcessingFramework/install/bin')
print('Module Search Path:')
print(sys.path)

import PyNvCodec as nvc

encFile = "rtsp://admin:123qweasd@192.168.1.68:554/h264/ch1/main/av_stream"
decFile = open("output.nv12", "wb")

nvDec = nvc.PyNvDecoder(encFile, 0)

while True:
    rawFrame = nvDec.DecodeSingleFrame()
    # Decoder will return zero-size frame if input file is over;
    if not (rawFrame.size):
        break

    frameByteArray = bytearray(rawFrame)
    print("data len:",len(frameByteArray))
    decFile.write(frameByteArray)