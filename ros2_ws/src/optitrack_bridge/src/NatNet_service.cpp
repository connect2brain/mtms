//
// Created by mtms on 14.10.2022.
//
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "neuronavigation_interfaces/srv/NatNet.hpp"
#include "neuronavigation_interfaces/msg/NatNet.hpp"
#include <chrono>
#include <cstdlib>
#include <memory>

#include "Client_Opti.h>

using namespace std::chrono_literals
void add(const std::shared_ptr<neuronavigation_interfaces::srv::NatNet::Response> response)
{
    myClient.Get_datastream();
    auto message = response.coordinates;
    message.PositionToolTipX1 = myClient.myOp.PositionToolTipX1;
    message.PositionToolTipY1 = myClient.myOp.PositionToolTipY1;
    message.PositionToolTipZ1 = myClient.myOp.PositionToolTipZ1;
    message.qxToolTip = myClient.myOp.qxToolTip;
    message.qyToolTip = myClient.myOp.qyToolTip;
    message.qzToolTip = myClient.myOp.qzToolTip;
    message.qwToolTip = myClient.myOp.qwToolTip;

    message.PositionHeadX1 = myClient.myOp.PositionHeadX1;
    message.PositionHeadY1 = myClient.myOp.PositionHeadY1;
    message.PositionHeadZ1 = myClient.myOp.PositionHeadZ1;
    message.qxHead = myClient.myOp.qxHead;
    message.qyHead = myClient.myOp.qyHead;
    message.qzHead = myClient.myOp.qzHead;
    message.qwHead = myClient.myOp.qwHead;

    message.PositionCoilX1 = myClient.myOp.PositionCoilX1;
    message.PositionCoilY1 = myClient.myOp.PositionCoilY1;
    message.PositionCoilZ1 = myClient.myOp.PositionCoilZ1;
    message.qxCoilTip = myClient.myOp.qxCoilTip;
    message.qyCoilTip = myClient.myOp.qyCoilTip;
    message.qzCoilTip = myClient.myOp.qzCoilTip;
    message.qwCoilTip = myClient.myOp.qwCoilTip;
}

int main()
{
    ClientOpti myClient;
    rclcpp::init(argc, argv);

    myClient.Connect_to_server(argc, serverIndex);
    myClient.ConnectToMotive();
    // Ready to receive marker stream!
    printf("\nClient is connected to server and listening for data...\n");
    bool bExit = false;
    //myClient.Get_datastream();
    sleep(5);

    rclcpp::Service<neuronavigation_interfaces::srv::NatNet>SharedPtr service =
            node ->create_service<neuronavigation_interfaces::srv::NatNet>("NatNet",&add);
    RCLPCPP_INFO(rccpp::get_logger("NatNet"), "NatNet server ready.");

    rclcpp::spin(node);
    rclcpp::shutdown();

}