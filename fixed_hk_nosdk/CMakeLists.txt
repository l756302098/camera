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
find_package(Eigen3 REQUIRED)

#set(_HTTPLIB_OPENSSL_MIN_VER "1.1.1")
#find_package(OpenSSL ${_HTTPLIB_OPENSSL_MIN_VER} COMPONENTS Crypto SSL REQUIRED)

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
  LIBRARIES fixed_hk_nosdk
  CATKIN_DEPENDS cv_bridge roscpp rospy std_msgs tf fixed_msg geometry_msgs
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

# install(PROGRAMS
#   scripts/*.py
#   DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
# )
install(DIRECTORY scripts
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

install(DIRECTORY launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)

add_custom_target(fixed_hk_nosdk_install ALL COMMAND "pip" "install" "--user" "-r" "${PROJECT_SOURCE_DIR}/requirements.txt")