cmake_minimum_required(VERSION 3.0.2)
project(fixed_yd_onvif)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  cv_bridge
  geometry_msgs
  roscpp
  rospy
  std_msgs
  fixed_msg
)
find_package(Eigen3 REQUIRED)

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
  LIBRARIES fixed_yd_onvif
  CATKIN_DEPENDS cv_bridge geometry_msgs roscpp rospy std_msgs fixed_msg
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
  ${EIGEN3_INCLUDE_DIR}
)

add_executable(ptz_node src/pt_node.cpp src/pt_control.cpp src/common.cpp)
target_link_libraries(ptz_node
  ${catkin_LIBRARIES}
)

add_executable(ptz_node2 src/pt_node2.cpp src/pt_control2.cpp src/common.cpp)
target_link_libraries(ptz_node2
  ${catkin_LIBRARIES}
)

add_executable(yd_visible_node src/visible_node.cpp src/visible_control.cpp src/common.cpp)
target_link_libraries(yd_visible_node
  ${catkin_LIBRARIES}
)