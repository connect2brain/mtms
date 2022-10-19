//
// Created by mtms on 13.10.2022.
//

#ifndef OPTITRACK_NATNET_CLIENT_OPTI_H
#define OPTITRACK_NATNET_CLIENT_OPTI_H

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
#include "Optitrack_NatNet.h"

class OptitrackClient {
public:

  int create_client(NatNetFrameReceivedCallback data_received_callback);

  int connect_client();

  int disconnect_client();

  void connect_to_motive();

  static void receive_rigidbody_data(sFrameOfMocapData *data, void *pUserData);

  int discover_motive_servers(int serverIndex);

  static optr myOp;

private:

  static const ConnectionType kDefaultConnectionType = ConnectionType_Multicast;

  NatNetClient *natnet_client = nullptr;

  static std::vector<sNatNetDiscoveredServer> discovered_servers;
  sNatNetClientConnectParams connect_parameters;
  char g_discoveredMulticastGroupAddr[kNatNetIpv4AddrStrLenMax] = NATNET_DEFAULT_MULTICAST_ADDRESS;
  int g_analogSamplesPerMocapFrame = 0;
  sServerDescription g_serverDescription;

};

#endif //OPTITRACK_NATNET_CLIENT_OPTI_H
