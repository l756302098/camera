# fixed_hk_nosdk

云台控制节点，接收任务调度节点的控制请求，计算不同任务下需要云台旋转的角度，根据请求控制云

台转动，并实时发布一体化云台中相机的网络通信状态

## Depends  
pip install requests  

## Compile
cd ~/workspace && catkin_make

## Start Run

roslaunch fixed_hk_nosdk hk.launch  


## Params
- isapi_ptz.launch  
```json
<node pkg="fixed_hk_nosdk" type="isapi_ptz_node.py" name="isapi_ptz_node" respawn="false" output="screen">
      <param name="camera_id" type="int" value="1"/>
      <!-- 设备ip -->
      <param name="device_ip" value="192.168.1.66"/>
      <!-- 设备http port -->
      <param name="device_port" value="80"/>
      <!-- 设备的用户名,密码 -->
      <param name="device_username" value="admin"/>
      <param name="device_password" value="abcd1234"/>
</node>
```
- rtsp_temp66.launch  
```json
<node pkg="fixed_hk_nosdk" type="rtsp_node.py" name="rtsp_temp_node" respawn="false" output="screen">
      <param name="camera_id" type="int" value="1"/>
      <!-- 设备ip -->
      <param name="device_ip" value="192.168.1.66"/>
      <!-- 设备rtsp port -->
      <param name="device_port" value="554"/>
      <!-- 设备的用户名,密码 -->
      <param name="device_username" value="admin"/>
      <param name="device_password" value="abcd1234"/>
      <!-- 红外图像的宽高 -->
      <param name="image_width" type="int" value="384"/>
      <param name="image_height" type="int" value="288"/>
  </node>
```