# tio_mqtt_bridge

mqtt_bridge provides a functionality to bridge between ROS and MQTT in bidirectional.


## Principle

`mqtt_bridge` uses ROS message as its protocol. Messages from ROS are serialized by json (or messagepack) for MQTT, and messages from MQTT are deserialized for ROS topic. So MQTT messages should be ROS message compatible. (We use `rosbridge_library.internal.message_conversion` for message conversion.)

This limitation can be overcome by defining custom bridge class, though.


## Demo

### Prerequisites

```
$ sudo apt install python3-pip
$ sudo apt install ros-noetic-rosbridge-library
$ sudo apt install mosquitto mosquitto-clients
```

### Install python modules

```bash
$ pip3 install -r requirements.txt
```

### launch node

``` bash
$ roslaunch mqtt_bridge demo.launch
```

Publish to `/ping`,

```
$ rostopic pub /ping std_msgs/Bool "data: true"
```

and see response to `/pong`.

```
$ rostopic echo /pong
data: True
---
```

## Usage

parameter file (config.yaml):

``` yaml
mqtt:
  client_name: tio_bridge_115       #client name
  client:
    protocol: 4                     # MQTTv311
  connection:
    host: 192.168.1.35              #mqtt host
    port: 1883                      #mqtt port
    keepalive: 60                   #heart beat
  private_path: device/001
  account:
    username: ydrobot               #client username
    password: 123qweasd             #client password
bridge:
    # ping pong
  - factory: mqtt_bridge.bridge:RosToMqttBridge
    msg_type: std_msgs.msg:Bool
    topic_from: /ping
    topic_to: ping
  - factory: mqtt_bridge.bridge:MqttToRosBridge
    msg_type: std_msgs.msg:Bool
    topic_from: ping
    topic_to: /pong
```

you can use any msg types like `std_msgs.msg:Bool`.

launch file:

``` xml
<launch>
  <arg name="use_tls" default="false" />
  <node name="mqtt_bridge" pkg="tio_mqtt_bridge" type="mqtt_bridge_node.py" output="screen">
    <rosparam command="load" file="$(find tio_mqtt_bridge)/config/tio_params.yaml" />
    <rosparam if="$(arg use_tls)" command="load" ns="mqtt" file="$(find tio_mqtt_bridge)/config/tls_params.yaml" />
  </node>
</launch>
```

## Configuration

### mqtt

Parameters under `mqtt` section are used for creating paho's `mqtt.Client` and its configuration.

#### subsections

* `client`: used for `mqtt.Client` constructor
* `tls`: used for tls configuration
* `account`: used for username and password configuration
* `message`: used for MQTT message configuration
* `userdata`: used for MQTT userdata configuration
* `will`: used for MQTT's will configuration

See `mqtt_bridge.mqtt_client` for detail.

### serializer and deserializer

`mqtt_bridge` uses `msgpack` as a serializer by default. But you can also configure other serializers. For example, if you want to use json for serialization, add following configuration.

``` yaml
serializer: json:dumps
deserializer: json:loads
```

### bridges

You can list ROS <--> MQTT tranfer specifications in following format.

``` yaml
bridge:
  # ping pong
  - factory: mqtt_bridge.bridge:RosToMqttBridge
    msg_type: std_msgs.msg:Bool
    topic_from: /ping
    topic_to: ping
  - factory: mqtt_bridge.bridge:MqttToRosBridge
    msg_type: std_msgs.msg:Bool
    topic_from: ping
    topic_to: /pong
```

* `factory`: bridge class for transfering message from ROS to MQTT, and vise versa.
* `msg_type`: ROS Message type transfering through the bridge.
* `topic_from`: topic incoming from (ROS or MQTT)
* `topic_to`: topic outgoing to (ROS or MQTT)

Also, you can create custom bridge class by inheriting `mqtt_brige.bridge.Bridge`.


## License

This software is released under the MIT License, see LICENSE.txt.
