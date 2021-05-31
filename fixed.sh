#!/bin/sh

WORKSPACE_PATH=`echo /home/yd/catkin_ws`
LOCAL_IP=192.168.1.115
MASTER_IP=192.168.1.115
echo "[YDRobot] Configuring dependencies......"
#fresh env
source /opt/ros/melodic/setup.bash
source $WORKSPACE_PATH/devel/setup.bash
export ROS_IP=$LOCAL_IP
export ROS_HOSTNAME=$LOCAL_IP
export ROS_MASTER_URI=http://$MASTER_IP:11311
export GENICAM_ROOT_V3_0=/opt/pleora/ebus_sdk/Ubuntu-x86_64/lib/genicam

echo "[YDRobot] Starting the main system nodes......"
#rosnode 
num=$(ps -aux | grep rosmaster | wc -l)
if test $[num] -gt 1
then
	echo "rosmaster num $num"
	echo "rosmaster running ..."
else
	roscore &
	sleep 5
fi
#rosbridge
roslaunch rosbridge_server rosbridge_websocket.launch &
#flir
roslaunch flir_gige node.launch &
roslaunch flir_gige thermal_proc.launch &
#control
roslaunch fixed_control control.launch &
#visible control
roslaunch fixed_visible client.launch
#infrared control
roslaunch fixed_infrared client.launch
#motor
roslaunch fixed_yd_onvif motor.launch &
#decoder
roslaunch fixed_decoder 131.launch &
#lidar
roslaunch livox_ros_driver livox_lidar.launch
echo "[YDRobot] System is running now......"

#cleanup ros log
rm -rf /home/yd/.ros/log
echo "[YDRobot] Stopping the system and all nodes......"
rosnode kill -a
