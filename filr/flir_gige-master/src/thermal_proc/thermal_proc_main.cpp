#include "thermal_proc/thermal_proc_node.h"

int main(int argc, char **argv) {
  ros::init(argc, argv, "thermal_proc");
  ros::NodeHandle nh;
  ros::NodeHandle pnh("~");

  try {
    flir_gige::ThermalProcNode thermal_proc_node(nh, pnh);
    ros::ServiceServer service = nh.advertiseService(
    	"get_temperature", &flir_gige::ThermalProcNode::GetTemperature, &thermal_proc_node);
      std::cout << "service = "  << service << std::endl;
    ros::spin();
  }
  catch (const std::exception &e) {
    ROS_ERROR("%s: %s", nh.getNamespace().c_str(), e.what());
  }
}
