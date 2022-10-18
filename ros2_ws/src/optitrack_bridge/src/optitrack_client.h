//
// Created by mtms on 13.10.2022.
//

#ifndef OPTITRACK_NATNET_CLIENT_OPTI_H
#define OPTITRACK_NATNET_CLIENT_OPTI_H

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

#include "NatNetTypes.h"
#include "NatNetCAPI.h"
#include "NatNetClient.h"
#include "Optitrack_NatNet.h"

class OptitrackClient {
public:
  static void NATNET_CALLCONV
  server_discovered_callback(const sNatNetDiscoveredServer *pDiscoveredServer, void *pUserContext);

  int connect_client();

  int disconnect_client();

  void connect_to_motive();

  static void receive_rigidbody_data(sFrameOfMocapData *data, void *pUserData);

  void connect_to_server(int argc, int serverIndex);

  void get_datastream();

  char getch();

  static optr myOp;

private:

};

#endif //OPTITRACK_NATNET_CLIENT_OPTI_H
