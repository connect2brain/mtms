//
// Created by mtms on 13.10.2022.
//

#include "optitrack_client.h"

#ifdef _WIN32
#include <conio.h>
#else

#include <unistd.h>
#include <termios.h>

#endif

#include <vector>

#include <NatNetTypes.h>
#include <NatNetCAPI.h>
#include <NatNetClient.h>


int OptitrackClient::connect_to_motive() {
  // Release previous server
  if (natnet_client) {
    natnet_client->Disconnect();
  }

  int ret_code = natnet_client->Connect(connect_parameters);
  if (ret_code != ErrorCode_OK) {
    RCLCPP_ERROR(rclcpp::get_logger("optitrack_bridge"), "Error connecting client to Motive. Error code: %d", ret_code);
  }
  return ret_code;
}

int OptitrackClient::disconnect_client() {
  if (natnet_client) {
    natnet_client->Disconnect();
    delete natnet_client;
    natnet_client = nullptr;
  }
  return ErrorCode_OK;
}

int OptitrackClient::create_client(NatNetFrameReceivedCallback data_received_callback) {
  natnet_client = new NatNetClient();
  natnet_client->SetFrameReceivedCallback(data_received_callback, natnet_client);

  return 0;
}

int OptitrackClient::discover_motive_servers() {
  RCLCPP_INFO(rclcpp::get_logger("optitrack_bridge"), "Looking for servers on the local network.");

  const unsigned discover_wait_time_ms = 5 * 1000;
  const int max_discovers = 1;
  sNatNetDiscoveredServer servers[max_discovers];
  int actual_number_of_discovers = max_discovers;

  NatNet_BroadcastServerDiscovery(
      servers,
      &actual_number_of_discovers,
      discover_wait_time_ms
  );

  if (actual_number_of_discovers > max_discovers) {
    RCLCPP_WARN(rclcpp::get_logger("optitrack_bridge"),
                "Discovered more motive servers than expected. Number of discovered servers: %d, expected: %d",
                actual_number_of_discovers, max_discovers);
  } else if (actual_number_of_discovers == 0) {
    RCLCPP_ERROR(rclcpp::get_logger("optitrack_bridge"), "No motive server found.");
    return 1;
  }

  const sNatNetDiscoveredServer &discovered_server = servers[0];

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
