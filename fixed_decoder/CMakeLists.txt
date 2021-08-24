cmake_minimum_required(VERSION 3.0.2)
project(fixed_decoder)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

option(ARM_BUILD "option for arm build" OFF)
if(ARM_BUILD)
  Message("ARM BUILD")
  set(AVCODEC_LIBRARY /usr/lib/aarch64-linux-gnu/libavcodec.so)
  set(AVDEVICE_LIBRARY /usr/lib/aarch64-linux-gnu/libavdevice.so)
  set(AVUTIL_LIBRARY /usr/lib/aarch64-linux-gnu/libavutil.so)
  set(AVFORMAT_LIBRARY /usr/lib/aarch64-linux-gnu/libavformat.so)
  set(SWRESAMPLE_LIBRARY /usr/lib/aarch64-linux-gnu/libswresample.so)
  set(SWSCALE_LIBRARY /usr/lib/aarch64-linux-gnu/libswscale.so)
else()
  Message("X86 BUILD")
  set(AVCODEC_LIBRARY /usr/lib/x86_64-linux-gnu/libavcodec.so)
  set(AVDEVICE_LIBRARY /usr/lib/x86_64-linux-gnu/libavdevice.so)
  set(AVUTIL_LIBRARY /usr/lib/x86_64-linux-gnu/libavutil.so)
  set(AVFORMAT_LIBRARY /usr/lib/x86_64-linux-gnu/libavformat.so)
  set(SWRESAMPLE_LIBRARY /usr/lib/x86_64-linux-gnu/libswresample.so)
  set(SWSCALE_LIBRARY /usr/lib/x86_64-linux-gnu/libswscale.so)
endif()
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
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(ffmpeg REQUIRED IMPORTED_TARGET libavcodec libavformat libavutil libswscale)

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
if(ARM_BUILD)
/usr/local/ffmpeg/include/libavcodec
/usr/local/ffmpeg/include/libavformat
/usr/local/ffmpeg/include/libswscale
else()
  /usr/include/x86_64-linux-gnu/libavcodec
  /usr/include/x86_64-linux-gnu/libavformat
  /usr/include/x86_64-linux-gnu/libswscale
endif()
)

add_executable(fixed_decoder_node src/rtsp_capture.cpp src/base64.cpp)
target_link_libraries(fixed_decoder_node 
${catkin_LIBRARIES}  
${AVCODEC_LIBRARY}
${AVDEVICE_LIBRARY}
${AVUTIL_LIBRARY}
${AVFORMAT_LIBRARY}
${SWRESAMPLE_LIBRARY}
${SWSCALE_LIBRARY})

add_executable(decoder_h264_node src/decoder_h264_node.cpp src/H264Decoder.cpp)
target_link_libraries(decoder_h264_node ${catkin_LIBRARIES}  ${OpenCV_LIBS} 
${catkin_LIBRARIES}  
${AVCODEC_LIBRARY}
${AVDEVICE_LIBRARY}
${AVUTIL_LIBRARY}
${AVFORMAT_LIBRARY}
${SWRESAMPLE_LIBRARY}
${SWSCALE_LIBRARY})