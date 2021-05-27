#!/usr/bin/env python
#coding=utf-8
'''
python处理xml格式入参的接口
'''

#1.先把xml入参用open函数读出来变为字符串
def read_xml():
    '''
    读取xml文件
    :return:
    '''
    f = open('status.xml',"r")
    body = f.read()
    print(body)
    return body

#2.发送接口请求
import requests
from requests.auth import HTTPDigestAuth

class Api_Test(object):

    def get_status(self):
        url = "http://192.168.1.131/ISAPI/PTZCtrl/channels/1/status"
        res = requests.get(url,auth=HTTPDigestAuth('admin','123qweasd'))
        print(res.text)

    def put_status(self):
        url = "http://192.168.1.131/ISAPI/PTZCtrl/channels/1/status"
        r = requests.put(url,auth=HTTPDigestAuth('admin','123qweasd'),data=read_xml())
        print(r.text)

if __name__ == '__main__':
    api = Api_Test()
    api.get_status()
    api.put_status()