cmake_minimum_required(VERSION 3.0.2)
project(tio_ftp)

## Compile as C++11, supported in ROS Kinetic and newer
#add_compile_options(-std=c++11)
set(CMAKE_CXX_STANDARD 14)
add_definitions(-DLINUX)
## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  roslib
  message_generation
)

## System dependencies are found with CMake's conventions
find_package(Boost REQUIRED COMPONENTS system)
if(Boost_FOUND)
  MESSAGE( STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}.")
  include_directories(${Boost_INCLUDE_DIRS})
endif(Boost_FOUND)

find_package(CURL REQUIRED)
if(CURL_FOUND)
  MESSAGE( STATUS "CURL_INCLUDE_DIRS = ${CURL_INCLUDE_DIRS}.")
  MESSAGE( STATUS "CURL_VERSION_STRING = ${CURL_VERSION_STRING}.")
  include_directories(${CURL_INCLUDE_DIRS})
endif(CURL_FOUND)

add_message_files(
  DIRECTORY msg
  FILES
  status.msg
)

## Generate services in the 'srv' folder
add_service_files(
  DIRECTORY srv
  FILES
  upload.srv
)

generate_messages(
  DEPENDENCIES
  std_msgs
)

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
  CATKIN_DEPENDS roscpp std_msgs roslib
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

# Declare a C++ library
set(CONTROL_SOURCES
src/CurlHandle.cpp
src/FTPClient.cpp
src/ftp_control.cpp
)

add_executable(${PROJECT_NAME}_node src/ftp_node.cpp ${CONTROL_SOURCES})
target_link_libraries(${PROJECT_NAME}_node  glog gflags ${catkin_LIBRARIES} ${CURL_LIBRARIES})
add_dependencies(${PROJECT_NAME}_node ${PROJECT_NAME}_generate_messages_cpp)

#############
## Install ##
#############

install(DIRECTORY include/tio_control/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h")

install(TARGETS ${PROJECT_NAME}_node
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})