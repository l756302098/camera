#!/usr/bin/env python
#coding=utf-8
import csv
import cv2  
from PIL import Image  
import numpy as np
#import cv
#1.先把xml入参用open函数读出来变为字符串
def read_csv():
    '''
    读取xml文件
    :return:
    '''
    with open('/home/li/HotPicData/20210817_155031_1629186631/data.csv', 'r') as f:
        reader = csv.reader(f)
        # print(type(reader))
        index_r = 0
        index_c = 0
        for row in reader:
            for cols in row:
                print(cols)
            # print(row)

def load_csv():
    dlist = np.loadtxt(open("/home/li/data.csv","rb"),delimiter=",",skiprows=0)
    print(dlist)
    print(dlist.shape)
    img = Image.fromarray(dlist)
    img.show()  
    # img = cv2.cvtColor(np.asarray(img),cv2.COLOR_GRAY2BGR)  
    # cv2.imshow("OpenCV",img)  
    # cv2.waitKey()  
    # nlist = dlist.reshape((384,288))
    # print()


if __name__ == '__main__':
    # read_csv()
    # show_image()
    load_csv()