//
// Created by mtms on 13.10.2022.
//

#include "optitrack_client.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#   include <conio.h>
#else

#   include <unistd.h>
#   include <termios.h>

#endif

#include <vector>

#include <NatNetTypes.h>
#include <NatNetCAPI.h>
#include <NatNetClient.h>


// Establish a NatNet Client connection
int OptitrackClient::connect_client() {
  // Release previous server
  if (natnet_client) {
    natnet_client->Disconnect();
  }

  // Init Client and connect to NatNet server
  int retCode = natnet_client->Connect(connect_parameters);
  if (retCode != ErrorCode_OK) {
    RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "Unable to connect to server.  Error code: %d. Exiting.",
                retCode);
    return ErrorCode_Internal;
  } else {
    // connection succeeded
  }

  return ErrorCode_OK;
}

void OptitrackClient::connect_to_motive() {    // Connect to Motive
  int iResult;
  iResult = connect_client();
  if (iResult != ErrorCode_OK) {
    RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "Error initializing client. See log for details. Exiting.");

  } else {
    RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "Client initialized and ready.");
  }
}

int OptitrackClient::disconnect_client() { // Done - clean up.
  if (natnet_client) {
    natnet_client->Disconnect();
    delete natnet_client;
    natnet_client = nullptr;
  }
  return ErrorCode_OK;
}

int OptitrackClient::create_client(NatNetFrameReceivedCallback data_received_callback) {
  unsigned char ver[4];
  NatNet_GetVersion(ver);
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "NatNet Sample Client (NatNet ver. %d.%d.%d.%d)", ver[0], ver[1],
              ver[2], ver[3]);

  // create NatNet client
  natnet_client = new NatNetClient();
  natnet_client->SetFrameReceivedCallback(data_received_callback, natnet_client);

  return 0;
}

int OptitrackClient::discover_motive_servers(int serverIndex) {
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "Looking for servers on the local network.");

  //const unsigned discover_wait_time_ms = 5 * 1000; // Wait 5 seconds for responses.
  const int max_discovers = 1;
  sNatNetDiscoveredServer servers[max_discovers];
  int actual_number_of_discovers = max_discovers;
  NatNet_BroadcastServerDiscovery(servers, &actual_number_of_discovers);

  if (actual_number_of_discovers > max_discovers) {
    RCLCPP_WARN(rclcpp::get_logger("optitrack_bridge"),
                "Discovered more motive servers than expected. Number of discovered servers: %d, expected: %d",
                actual_number_of_discovers, max_discovers);
  } else if (actual_number_of_discovers == 0) {
    RCLCPP_ERROR(rclcpp::get_logger("optitrack_bridge"), "No motive server found.");
    return 1;
  }

  const sNatNetDiscoveredServer &discovered_server = servers[serverIndex - 1];

  if (discovered_server.serverDescription.bConnectionInfoValid) {
    // Build the connection parameters.

    snprintf(
        g_discoveredMulticastGroupAddr, sizeof g_discoveredMulticastGroupAddr,
        "%" PRIu8 ".%" PRIu8".%" PRIu8".%" PRIu8"",
        discovered_server.serverDescription.ConnectionMulticastAddress[0],
        discovered_server.serverDescription.ConnectionMulticastAddress[1],
        discovered_server.serverDescription.ConnectionMulticastAddress[2],
        discovered_server.serverDescription.ConnectionMulticastAddress[3]
    );

    connect_parameters.connectionType = discovered_server.serverDescription.ConnectionMulticast
                                        ? ConnectionType_Multicast
                                        : ConnectionType_Unicast;
    connect_parameters.serverCommandPort = discovered_server.serverCommandPort;
    connect_parameters.serverDataPort = discovered_server.serverDescription.ConnectionDataPort;
    connect_parameters.serverAddress = discovered_server.serverAddress;
    connect_parameters.localAddress = discovered_server.localAddress;
    connect_parameters.multicastAddress = g_discoveredMulticastGroupAddr;

    RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "Discovered address: %s", g_discoveredMulticastGroupAddr);
  }
  return 0;
}
