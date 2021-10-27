<!--
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-05-12 16:14:16
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 18:18:01
-->
# tio_control
接收上位机或者网关层的消息,控制固定式建图     
#  话题
##  运行模式
Topic: /tio/control/task_status  
Type: tio_control/task  
```json
int32 device_id               #设备 id
int32 task_id                 #task id
int32 map_id                  #map id
string task_name              #任务名称/mapping coloring
int32 task_code               #错误代码/mapping coloring
float32 task_progress         #任务进度
int32 task_status             #0:正常结束\1:终止\2:暂停\3:正在执行                             #4:未执行\5:超期\6:预执行\7:超时
string message                #任务当前信息
```

#  服务
##  开始建图任务
Topic: /tio/control/mapping_start  
Type: tio_control/mapping_task  
```json
int32 device_id
int32 task_id
int32 map_id
int32 flag
---
bool success
string message
```
##  控制建图任务
Topic: /tio/control/mapping_control  
Type: tio_control/control_task  
```json
int32 device_id
int32 task_id
int32 map_id
int32 flag
---
bool success
string message
```
##  开始着色任务
Topic: /tio/control/coloring_start  
Type: tio_control/coloring_task  
```json
int32 device_id
int32 task_id
int32 map_id
int32 flag
---
bool success
string message
```
##  控制着色任务
Topic: /tio/control/coloring_control  
Type: tio_control/control_task  
```json
int32 device_id
int32 task_id
int32 map_id
int32 flag
---
bool success
string message
```
#  启动
##  启动建图
```json
#driver
roslaunch livox_ros_driver livox_lidar.launch
roslaunch fixed_decoder visible_65.launch
roslaunch  fixed_yd_motor motor2.launch
roslaunch mapping_service mapping_service.launch
roslaunch tio_control control.launch
```
##  启动着色
```json
#driver
roslaunch livox_ros_driver livox_lidar.launch
roslaunch fixed_hk_nosdk isapi_temp_68.launch
roslaunch  fixed_yd_motor motor2.launch
roslaunch tf_ircampose_service tf_ircampose_service.launch
roslaunch undistort_service undistort_service.launch
roslaunch tio_control control.launch
```