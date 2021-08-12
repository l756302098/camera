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

def read_PixelToPixelParam():
    '''
    读取xml文件
    :return:
    '''
    f = open('PixelToPixelParam.xml',"r")
    body = f.read()
    print(body)
    return body

if __name__ == '__main__':
    content = read_PixelToPixelParam()
    print(content)
    # api = HK_Api()
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