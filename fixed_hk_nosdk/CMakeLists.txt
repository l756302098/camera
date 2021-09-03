cmake_minimum_required(VERSION 3.0.2)
project(fixed_hk_nosdk)

## Compile as C++11, supported in ROS Kinetic and newer
# add_compile_options(-std=c++11)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  cv_bridge
  geometry_msgs
  roscpp
  rospy
  std_msgs
  tf
  fixed_msg
)
catkin_python_setup()
catkin_package()

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

install(DIRECTORY scripts
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

install(DIRECTORY launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

add_custom_target(fixed_hk_nosdk_install ALL COMMAND "pip" "install" "--user" "-r" "${PROJECT_SOURCE_DIR}/requirements.txt")