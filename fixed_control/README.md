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
##  红外数据
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
int16 device_id
int32 id
int32 action
int32 type
int32 value
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
