/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-02 11:45:05
 * @LastEditors: li
 * @LastEditTime: 2021-05-06 15:34:49
 */
#ifndef __YD_COMMON_H__
#define __YD_COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <setjmp.h>
#include <errno.h>
#include <vector>
#include <iostream>

using namespace std;

#define MAX_DEGREE_DEC 3600
#define MAX_DEGREE_HEX 0x3600
#define ISAPI_OUT_LEN	3 * 1024 * 1024
#define ISAPI_STATUS_LEN  8*1024

//字符串分割函数  
void SplitString(const string& s, vector<string>& v, const string& c);
//字符串拼接函数
char* SplicingString(const char *a, const char *b);

unsigned short DECToHEX(int x);

unsigned short HEXToDEC(int x);

unsigned int N_CRC16(unsigned char *updata, unsigned int len);

#endif