/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-04-02 11:45:12
 * @LastEditors: li
 * @LastEditTime: 2021-04-12 16:55:18
 */
#include "fixed_hk_sdk/common.hpp"

using namespace std;

//字符串分割函数  
void SplitString(const string& s, vector<string>& v, const string& c)
{
    string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }

    if(pos1 != s.length())
    {
        v.push_back(s.substr(pos1));
    }
}

//字符串拼接函数
char* SplicingString(const char *a, const char *b)
{
    char* dest = new char[strlen(a)+strlen(b)+1];
    strcpy(dest,a);
    strcat(dest,b);
    return dest;
}

unsigned short DECToHEX(int x)
{
    if(x > MAX_DEGREE_DEC)
    {
        return 0;
    }
    return (x/1000)*4096 + ((x%1000)/100)*256 + ((x%100)/10)*16 + x%10;
}

unsigned short HEXToDEC(int x)
{
    if(x > MAX_DEGREE_HEX)
    {
        return 0;
    }
    return (x/4096*1000) + (x%4096/256*100) + (x%256/16*10) + x%16;
}

