cmake_minimum_required(VERSION 3.0.2)
project(fixed_decoder)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  image_transport
  roscpp
  rospy
  sensor_msgs
  std_msgs
  cv_bridge
)

find_package( OpenCV REQUIRED )
find_package(PkgConfig REQUIRED)
pkg_check_modules(ffmpeg REQUIRED IMPORTED_TARGET libavcodec libavformat libavutil libswscale)

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if your package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
   INCLUDE_DIRS include
   LIBRARIES fixed_decoder
   CATKIN_DEPENDS image_transport roscpp rospy sensor_msgs std_msgs cv_bridge
#  DEPENDS system_lib
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_executable(fixed_decoder_node src/rtsp_capture.cpp src/base64.cpp)
target_link_libraries(fixed_decoder_node ${catkin_LIBRARIES}  libavcodec.so libavformat.so libavutil.so libswscale.so)

add_executable(decoder_h264_node src/decoder_h264_node.cpp src/H264Decoder.cpp)
target_link_libraries(decoder_h264_node ${catkin_LIBRARIES}  ${OpenCV_LIBS} libavcodec.so libavformat.so libavutil.so libswscale.so)