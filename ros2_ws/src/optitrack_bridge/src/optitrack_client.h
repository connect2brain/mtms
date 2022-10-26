//
// Created by mtms on 13.10.2022.
//

#ifndef OPTITRACK_NATNET_CLIENT_OPTI_H
#define OPTITRACK_NATNET_CLIENT_OPTI_H

#include <cinttypes>

#include "rclcpp/rclcpp.hpp"

#ifdef _WIN32
#include <conio.h>
#else

#include <unistd.h>
#include <termios.h>

#endif

#include <vector>

#include "NatNetTypes.h"
#include "NatNetCAPI.h"
#include "NatNetClient.h"

class OptitrackClient {
public:

  int create_client(NatNetFrameReceivedCallback data_received_callback);

  int disconnect_client();

  int connect_to_motive();

  int discover_motive_servers();

private:

  NatNetClient *natnet_client = nullptr;

  static std::vector<sNatNetDiscoveredServer> discovered_servers;
  sNatNetClientConnectParams connect_parameters;
  char g_discoveredMulticastGroupAddr[kNatNetIpv4AddrStrLenMax] = NATNET_DEFAULT_MULTICAST_ADDRESS;

};

#endif //OPTITRACK_NATNET_CLIENT_OPTI_H
