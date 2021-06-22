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
    f = open('position.xml',"r")
    body = f.read()
    print(body)
    return body

#2.发送接口请求
from os import nice
from io import BytesIO
import requests
from requests.auth import HTTPDigestAuth
from xml.etree import ElementTree

class HK_Api(object):
    def __init__(self):
        print("init")
    '''
    Turn a simple dict of key/value pairs into XML
    '''
    def dict_to_xml(self,tag, d):
        root = ElementTree.Element(tag)  #使用Element创建元素
        root.set("xmlns","http://www.hikvision.com/ver20/XMLSchema")
        root.set("version","2.0")
        elem = ElementTree.SubElement(root, 'AbsoluteHigh')
        #循环key与val
        for key, val in d.items():
            #创建新的元素,确定元素的值
            child = ElementTree.Element(key)
            child.text = str(val)
            #添加为elem的子节点
            elem.append(child)
        return root
    
    def dict_to_xml2(self,tag):
        root = ElementTree.Element(tag)  #使用Element创建元素
        root.set("xmlns","http://www.hikvision.com/ver20/XMLSchema")
        root.set("version","2.0")
        child = ElementTree.Element("videoCodingType")
        child.text = str("pixel-to-pixel_thermometry_data")
        #child.text = str("real-time_raw_data")
        #添加为elem的子节点
        root.append(child)
        return root

    def get_status(self,ip,channel,username="admin",psd="123qweasd"):
        elevation = -1
        azimuth = -1
        absoluteZoom = -1
        url = "http://"+ip+"/ISAPI/PTZCtrl/channels/"+str(channel)+"/status"
        print("url :",url)
        res = requests.get(url,auth=HTTPDigestAuth(username,psd))
        print(res.text)
        root = ElementTree.fromstring(res.text)
        #print("root tag:",root.tag)
        if root==None:
            return elevation,azimuth,absoluteZoom
        if root.tag.find('PTZStatus') == -1:
            print("not contain PTZStatus")
        for child in root:
            #print(child.tag, child.attrib,child.text,child.tail)
            if child == None or child.tag == None:
                continue
            if  child.tag.find('AbsoluteHigh') == -1:
                continue
            for grandson in child:
                #print("grandson ",grandson.tag, grandson.attrib,grandson.text,grandson.tail)
                if 'elevation' in grandson.tag:
                    elevation = int(grandson.text)
                if 'azimuth' in grandson.tag:
                    azimuth = int(grandson.text)
                if 'absoluteZoom' in grandson.tag:
                    absoluteZoom = int(grandson.text)
        print([elevation,azimuth,absoluteZoom])
        return elevation,azimuth,absoluteZoom

    def put_status(self,ip,channel,p,t,z,username="admin",psd="123qweasd"):
        url = "http://"+ip+"/ISAPI/PTZCtrl/channels/"+str(channel)+"/absolute"
        params = {
            'elevation':t,
            'azimuth':p,
            'absoluteZoom':z
        }
        root = self.dict_to_xml('PTZData', params)
        f = BytesIO()
        et = ElementTree.ElementTree(root)
        et.write(f, encoding='utf-8', xml_declaration=True)
        request_data = f.getvalue()
        print("request_data:",request_data)
        #ElementTree.dump(root)
        #print("tostring:",ElementTree.tostring(root,encoding="utf-8",method="xml"))
        r = requests.put(url,auth=HTTPDigestAuth(username,psd),data=request_data)
        print(r.text)

    def put_position(self,ip,channel,username="admin",psd="123qweasd"):
        url = "http://"+ip+"/ISAPI/PTZCtrl/channels/"+str(channel)+"/position3D"
        #ElementTree.dump(root)
        #print("tostring:",ElementTree.tostring(root,encoding="utf-8",method="xml"))
        r = requests.put(url,auth=HTTPDigestAuth(username,psd),data=read_xml())
        print(r.text)
    
    def put_streamParam(self,ip,channel,username="admin",psd="123qweasd"):
        url = "http://"+ip+"/ISAPI/Thermal/channels/"+str(channel)+"/streamParam"
        root = self.dict_to_xml2('ThermalStreamParam')
        f = BytesIO()
        et = ElementTree.ElementTree(root)
        et.write(f, encoding='utf-8', xml_declaration=True)
        request_data = f.getvalue()
        print("request_data:",request_data)
        #ElementTree.dump(root)
        #print("tostring:",ElementTree.tostring(root,encoding="utf-8",method="xml"))
        r = requests.put(url,auth=HTTPDigestAuth(username,psd),data=request_data)
        print(r.text)

# if __name__ == '__main__':
#     api = HK_Api()
#     api.get_status('192.168.1.66',1,'admin','abcd1234')
#     api.put_status(0,0,0)
#     api.put_position()
#     api.put_streamParam()