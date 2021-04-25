#!/usr/bin/env python

import requests
from requests.auth import HTTPDigestAuth
import json
from PIL import Image
import os 
from io import BytesIO

url = 'http://192.168.1.66/ISAPI/Security/userCheck'
temp_url = 'http://192.168.1.66/ISAPI/Thermal/channels/2/thermometry/jpegPicWithAppendData?format=json'

#resp = requests.get(url,auth=HTTPDigestAuth('admin','abcd1234'))

resp = requests.get(temp_url,auth=HTTPDigestAuth('admin','abcd1234'))
print(resp.headers)
print("code:",resp.status_code)


folder_path = "/home/li"
resp.encoding = 'utf-8'
#print("text:",resp.text)
form_content = resp.content
form_list = form_content.split("--boundary")
print("len:",len(form_list))
for content in form_list:
    content_list = content.split("\r\n")
    print("size:",len(content_list))
    if "application/json" in content:
        print("contain application/json")
        content_len = len(content_list) 
        for index in range(content_len):
            line = content_list[index]
            print(line)
    if "image/pjpeg" in content:
        print("contain image/pjpeg")
        data_index = len(content_list)-2
        data_array = content_list[data_index]
        print("data_array len",len(data_array))
        #print("data_array:",data_array)
        image = Image.open(BytesIO(data_array))
        image.save(folder_path+"/aaa.jpg")
    if "application/octet-stream" in content:
        print("contain application/octet-stream")
        data_index = len(content_list)-2
        data_array = content_list[data_index]
        print("type:",type(data_array))
        b = BytesIO(data_array)
        print("b type:",type(b))
        #print("getvalue:",b.getvalue())
        print("str array len",len(data_array))
        #print("byte array len",data_array)
    # content_len = len(content_list) 
    # for index in range(content_len):
    #     line = content_list[index]
    #     if len(line) == 0:
    #         print("current len is 0")
    #         continue
    #     if len(line) < 100:
    #         print 'current:', line