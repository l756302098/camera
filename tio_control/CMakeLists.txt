cmake_minimum_required(VERSION 3.0.2)
project(tio_control)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  roslib
  message_generation
  mapping_msg
  fixed_msg
)

## System dependencies are found with CMake's conventions
find_package(Boost REQUIRED COMPONENTS system)
if(Boost_FOUND)
  MESSAGE( STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}.")
  include_directories(${Boost_INCLUDE_DIRS})
endif(Boost_FOUND)

add_message_files(
  DIRECTORY msg
  FILES
  task_status.msg
)

## Generate services in the 'srv' folder
add_service_files(
  DIRECTORY srv
  FILES
  mapping_task.srv
  coloring_task.srv
  control_task.srv
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
src/action_idle.cpp
src/action_coloring.cpp
src/action_mapping.cpp
src/task_control.cpp
)

add_executable(${PROJECT_NAME}_node src/tio_control_node.cpp ${CONTROL_SOURCES})
target_link_libraries(${PROJECT_NAME}_node  glog gflags ${catkin_LIBRARIES})
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
  RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})