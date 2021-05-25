#include "ros/ros.h"
#include "temperature/GetTemperature.h"

bool get_temperature(temperature::GetTemperature::Request &req,
         temperature::GetTemperature::Response &res)
{
    //std::cout << "sizeof(req.spot_x)/sizeof(int) = " << sizeof(req.spot_x)/sizeof(short) << std::endl;
    std::cout << "sizeof(req.spot_x) = " << sizeof(req.spot_x) << std::endl;
    std::cout << "sizeof(int) = " << sizeof(req.spot_x[0]) << std::endl;
	for(int i=0; i<req.spot_x.size(); i++)
	{
		res.temperature_c = req.spot_x[i] + req.spot_y[i];
        std::cout << "res.temperature_c = " << res.temperature_c << std::endl;
	}
    ROS_INFO("sending back response: [%ld]", (long int)res.temperature_c);
    return true;
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "get_temperature_server");
    ros::NodeHandle n;

    ros::ServiceServer service = n.advertiseService("get_temperature", get_temperature);
    ROS_INFO("Ready to get points.");
    ros::spin();

    return 0;
}
