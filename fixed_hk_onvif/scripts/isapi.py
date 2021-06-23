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

from requests.models import codes

class HK_Api(object):
    def __init__(self):
        print("init")
    '''
    Turn a simple dict of key/value pairs into XML
    '''
    def PTZData_to_xml(self, d):
        root = ElementTree.Element("PTZData")  #使用Element创建元素
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
    
    def ThermalStreamParam_to_xml(self):
        root = ElementTree.Element("ThermalStreamParam")  #使用Element创建元素
        root.set("xmlns","http://www.hikvision.com/ver20/XMLSchema")
        root.set("version","2.0")
        child = ElementTree.Element("videoCodingType")
        child.text = str("pixel-to-pixel_thermometry_data")
        #child.text = str("real-time_raw_data")
        #添加为elem的子节点
        root.append(child)
        return root

    def Position3D_to_xml(self,sx,sy,ex,ey):
        root = ElementTree.Element("Position3D")  #使用Element创建元素
        root.set("xmlns","http://www.hikvision.com/ver20/XMLSchema")
        root.set("version","2.0")
        s_elem = ElementTree.SubElement(root, 'StartPoint')
        s_params = {
            'positionX':sx,
            'positionY':sy
        }
        for key, val in s_params.items():
            #创建新的元素,确定元素的值
            child = ElementTree.Element(key)
            child.text = str(val)
            #添加为elem的子节点
            s_elem.append(child)
        e_elem = ElementTree.SubElement(root, 'EndPoint')
        s_params['positionX'] = ex 
        s_params['positionY'] = ey
        for key, val in s_params.items():
            #创建新的元素,确定元素的值
            child = ElementTree.Element(key)
            child.text = str(val)
            #添加为elem的子节点
            e_elem.append(child)
        return root

    def get_status(self,ip,channel=1,username="admin",psd="123qweasd"):
        elevation = -1
        azimuth = -1
        absoluteZoom = -1
        url = "http://"+ip+"/ISAPI/PTZCtrl/channels/"+str(channel)+"/status"
        response = ""
        try:
            res = requests.get(url,auth=HTTPDigestAuth(username,psd))
            response = res.text
        except Exception:
            print("request error")
            return elevation,azimuth,absoluteZoom 
        root = ElementTree.fromstring(response)
        #print("root tag:",root.tag)
        if root==None:
            return elevation,azimuth,absoluteZoom
        if root.tag.find('PTZStatus') == -1:
            print("not contain PTZStatus")
            return elevation,azimuth,absoluteZoom
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

    def put_status(self,ip,p,t,z,channel=1,username="admin",psd="123qweasd"):
        url = "http://"+ip+"/ISAPI/PTZCtrl/channels/"+str(channel)+"/absolute"
        params = {
            'elevation':t,
            'azimuth':p,
            'absoluteZoom':z
        }
        root = self.PTZData_to_xml(params)
        f = BytesIO()
        et = ElementTree.ElementTree(root)
        et.write(f, encoding='utf-8', xml_declaration=True)
        request_data = f.getvalue()
        print("request_data:",request_data)
        response = ""
        try:
            r = requests.put(url,auth=HTTPDigestAuth(username,psd),data=request_data)
            print(r.text)
            response = r.text
        except Exception:
            print("request error")
            return 0
        root = ElementTree.fromstring(response)
        #print("root tag:",root.tag)
        if root==None:
            return 0
        if root.tag.find('ResponseStatus') == -1:
            print("not contain ResponseStatus")
            return 0
        for child in root:
            if child == None or child.tag == None:
                continue
            if 'statusCod' in child.tag:
                code = int(child.text)
                print("code:",code)
                return code
        return 0

    def put_position(self,ip,sx,sy,ex,ey,channel=1,username="admin",psd="123qweasd"):
        url = "http://"+ip+"/ISAPI/PTZCtrl/channels/"+str(channel)+"/position3D"
        root = self.Position3D_to_xml(sx,sy,ex,ey)
        f = BytesIO()
        et = ElementTree.ElementTree(root)
        et.write(f, encoding='utf-8', xml_declaration=True)
        request_data = f.getvalue()
        response = ""
        try:
            r = requests.put(url,auth=HTTPDigestAuth(username,psd),data=request_data)
            response = r.text
        except Exception:
            print("request error")
            return 0
        root = ElementTree.fromstring(response)
        #print("root tag:",root.tag)
        if root==None:
            return 0
        if root.tag.find('ResponseStatus') == -1:
            print("not contain ResponseStatus")
            return 0
        for child in root:
            if child == None or child.tag == None:
                continue
            if 'statusCod' in child.tag:
                code = int(child.text)
                print("code:",code)
                return code
        return 0
    
    def get_streamParam(self,ip,channel=1,username="admin",psd="123qweasd"):
        url = "http://"+ip+"/ISAPI/Thermal/channels/"+str(channel)+"/streamParam"
        response = ""
        try:
            r = requests.get(url,auth=HTTPDigestAuth(username,psd))
            response = r.text
        except Exception:
            print("request error")
            return None,0
        root = ElementTree.fromstring(response)
        if root==None:
            return None,0
        if root.tag.find('ThermalStreamParam') == -1:
            print("not contain ThermalStreamParam")
            return None,0
        for child in root:
            #print(child.tag, child.attrib,child.text,child.tail)
            if child == None or child.tag == None:
                continue
            if 'videoCodingType' in child.tag:
                video_type = child.text
                print("video_type:",video_type)
                return video_type,1
        return None,0

    def put_streamParam(self,ip,channel=1,username="admin",psd="123qweasd"):
        url = "http://"+ip+"/ISAPI/Thermal/channels/"+str(channel)+"/streamParam"
        root = self.ThermalStreamParam_to_xml()
        f = BytesIO()
        et = ElementTree.ElementTree(root)
        et.write(f, encoding='utf-8', xml_declaration=True)
        request_data = f.getvalue()
        response = ""
        try:
            r = requests.put(url,auth=HTTPDigestAuth(username,psd),data=request_data)
            response = r.text
        except Exception:
            print("request error")
            return 0
        root = ElementTree.fromstring(response)
        if root==None:
            return 0
        if root.tag.find('ResponseStatus') == -1:
            print("not contain ResponseStatus")
            return 0
        for child in root:
            if child == None or child.tag == None:
                continue
            if 'statusCod' in child.tag:
                code = int(child.text)
                print("code:",code)
                return code
        return 0

# if __name__ == '__main__':
#     api = HK_Api()
    # p,t,z = api.get_status('192.168.1.65',1,'admin','abcd1234')
    # print(p,t,z)
    # code = api.put_status('192.168.1.65',0,0,0,1,'admin','abcd1234')
    # print(code)
    # code = api.put_position('192.168.1.66',1,1,10,10,1,'admin','abcd1234')
    # print(code)
    # result = api.get_streamParam('192.168.1.66',1,'admin','abcd1234')
    # print("result:",result)
    # code = api.put_streamParam('192.168.1.66',1,'admin','abcd1234')
    # print(code)