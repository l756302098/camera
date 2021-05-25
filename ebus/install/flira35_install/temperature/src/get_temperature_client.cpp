#include "ros/ros.h"
#include "temperature/GetTemperature.h"
#include <cstdlib>

int main(int argc, char **argv)
{
    std::cout << "begin" << std::endl;
    ros::init(argc, argv, "get_temperature_client");
    ros::NodeHandle n;
    ros::ServiceClient client = n.serviceClient<temperature::GetTemperature>("get_temperature");
    temperature::GetTemperature srv;

    std::vector<unsigned short> spot_x;
    std::vector<unsigned short> spot_y;

    spot_x.push_back(1);
    spot_x.push_back(2);

    spot_y.push_back(4);
    spot_y.push_back(5);

    srv.request.spot_x = spot_x;
    srv.request.spot_y = spot_y;

    std::cout << "sizeof(req.spot_x) = " << srv.request.spot_x.size() << std::endl;
    if(client.call(srv))
    {
        ROS_INFO("temperature: %ld", (long int)srv.response.temperature_c);
    }
    else
    {
        ROS_ERROR("Faild to call service get_temperature");
        return 1;
    }
    return 0;
}
