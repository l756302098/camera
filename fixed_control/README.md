<!--
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-05-12 16:14:16
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 18:18:01
-->
# fixed_control
接收上位机或者网关层的消息,控制固定式设备按任务运动      
#  话题
##  运行模式
Topic: /fixed/control/mode  
Type: fixed_msg/control_mode  
```json
int16 device_id
int32 mode
```
##  运行模式控制
Topic: /fixed/control/mode_control  
Type: fixed_msg/control_mode  
```json
int16 device_id
int32 mode
```
##  任务状态
Topic: /fixed/control/task_status 
Type: fixed_msg/task_status    
```json
int16 device_id
int32 task_id
uint8 task_status
```
##  云台变换(内部使用)
Topic: /fixed/platform/transfer  
Type: fixed_msg/platform_transfer  
```json
int32 flag
string data
```
##  开始着色任务
Topic: /infrared_survey_parm  
Type: fixed_msg::inspected_result    
```json
int32 camid
int32 picid
float32 x
float32 y
float32 z
uint8[] equipimage
uint8[] nameplates
string equipid
string result
bool success
```

##  开始抄表任务
Topic: /visible_survey_parm  
Type: fixed_msg::inspected_result  
```json
int32 camid
int32 picid
float32 x
float32 y
float32 z
uint8[] equipimage
uint8[] nameplates
string equipid
string result
bool success
```
##  检测完成
Topic: /ydmsg/platform/keep  
Type: std_msgs/String  
```json
string data
```
##  红外温度数据
Topic: /fixed/infrared/raw  
Type: sensor_msgs/Image  
```json
std_msgs/Header header
  uint32 seq
  time stamp
  string frame_id
uint32 height
uint32 width
string encoding
uint8 is_bigendian
uint32 step
uint8[] data
```
##  红外图像
Topic: /fixed/decoder/infrared  
Type: sensor_msgs/Image  
```json
std_msgs/Header header
  uint32 seq
  time stamp
  string frame_id
uint32 height
uint32 width
string encoding
uint8 is_bigendian
uint32 step
uint8[] data
```
##  可见光图像
Topic: /fixed/decoder/visible  
Type: sensor_msgs/Image  
```json
std_msgs/Header header
  uint32 seq
  time stamp
  string frame_id
uint32 height
uint32 width
string encoding
uint8 is_bigendian
uint32 step
uint8[] data
```
##  云台姿态
Topic: /fixed/platform/position  
Type:  nav_msgs/Odometry  
```json
std_msgs/Header header
  uint32 seq
  time stamp
  string frame_id
string child_frame_id
geometry_msgs/PoseWithCovariance pose
  geometry_msgs/Pose pose
    geometry_msgs/Point position
      float64 x
      float64 y
      float64 z
    geometry_msgs/Quaternion orientation
      float64 x
      float64 y
      float64 z
      float64 w
  float64[36] covariance
geometry_msgs/TwistWithCovariance twist
  geometry_msgs/Twist twist
    geometry_msgs/Vector3 linear
      float64 x
      float64 y
      float64 z
    geometry_msgs/Vector3 angular
      float64 x
      float64 y
      float64 z
  float64[36] covariance
```
##  云台到达指定位置
Topic: /fixed/platform/isreach  
Type: std_msgs/Int32  
```json
int32 data
```
1：转到指定位置

#  服务
##  云台控制命令
Topic: /fixed/platform/cmd  
Type: fixed_msg/cp_control  
```json
int16 device_id #设备id
int32 id        #云台id
int32 action    #设置1
int32 type      #水平0 垂直1 变倍2 水平相对转动3 垂直相对转动4
int32 value     #设置值(0-36000)/相对转动(value>0顺时针转动 value<0逆时针转动)
uint32[] allvalue
---
int32 result
```
##  任务下发
Topic: /fixed/control/task  
Type: fixed_msg/task    
```json
int16 device_id
string plan
---
bool status
```
example  
10003                           部件id  
5                               识别类型：5红外  
21.160200,6.331850,-134.537994  观测目标坐标  
10010001                        设备id  
1                               机器人id  
348                             任务id
```json
{
            "Align":"",
            "CameraPose":"10003:5/21.160200,6.331850,-134.537994/10010001/1/348",
            "Id":"0",
            "IsAnterograde":false,
            "TLine":"0.0,0.0,0.0/0.0,0.0,0.0",
            "TLoc":"0.0;0.0;0.0",
            "TLocType":"transfer",
            "TLocWidth":0,
            "TableInfo":"",
            "TurnAngle":"",
            "Watchs":[
                {
                    "Assemblys":[
                        {
                            "AssemblyId":10003,
                            "ReconParam":"10010001",
                            "ReconType":5
                        }
                    ],
                    "WatchId":8,
                    "WatchPos":"21.160200,6.331850,-134.537994",
                    "WatchType":"fixed"
                }
            ]
        }
```
##  任务控制
Topic: /fixed/control/task_control  
Type: fixed_msg/task_control    
```json
int16 device_id
int32 task_id
int16 flag
---
bool success
```
