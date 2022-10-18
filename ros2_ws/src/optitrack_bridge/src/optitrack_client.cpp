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
#include "Optitrack_NatNet.h"
#include "Definitions.h"

optr OptitrackClient::myOp;

char OptitrackClient::getch() {
  char buf = 0;
  termios old = {0};

  fflush(stdout);

  if (tcgetattr(0, &old) < 0)
    perror("tcsetattr()");

  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;

  if (tcsetattr(0, TCSANOW, &old) < 0)
    perror("tcsetattr ICANON");

  if (read(0, &buf, 1) < 0)
    perror("read()");

  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;

  if (tcsetattr(0, TCSADRAIN, &old) < 0)
    perror("tcsetattr ~ICANON");

  //printf( "%c\n", buf );

  return buf;
}

void OptitrackClient::receive_rigidbody_data(sFrameOfMocapData *data, void *pUserData) {
  // Rigid Bodies
  int i = 0;
  printf("Rigid Bodies [Count=%d]\n", data->nRigidBodies);
  for (i = 0; i < data->nRigidBodies; i++) {
    if (data->RigidBodies[i].ID == 999) {
      myOp.PositionToolTipX1 = data->RigidBodies[i].x;
      myOp.PositionToolTipY1 = data->RigidBodies[i].y;
      myOp.PositionToolTipZ1 = data->RigidBodies[i].z;
      myOp.qxToolTip = data->RigidBodies[i].qx;
      myOp.qyToolTip = data->RigidBodies[i].qy;
      myOp.qzToolTip = data->RigidBodies[i].qz;
      myOp.qwToolTip = data->RigidBodies[i].qw;
      printf("\tRigid Bodies \n");
      printf("\tMeasurement Probe\t%3.2f\n", myOp.PositionToolTipX1);
      printf("\tMeasurement Probe\t%3.2f\n", myOp.PositionToolTipY1);
      printf("\tMeasurement Probe\t%3.2f\n", myOp.PositionToolTipZ1);
      printf("\tMeasurement Probe\t%3.2f\n", myOp.qxToolTip);
      printf("\tMeasurement Probe\t%3.2f\n", myOp.qyToolTip);
      printf("\tMeasurement Probe\t%3.2f\n", myOp.qzToolTip);
      printf("\tMeasurement Probe\t%3.2f\n", myOp.qwToolTip);
    } else if (data->RigidBodies[i].ID == 1003) {
      myOp.PositionHeadX1 = data->RigidBodies[i].x;
      myOp.PositionHeadY1 = data->RigidBodies[i].y;
      myOp.PositionHeadZ1 = data->RigidBodies[i].z;
      myOp.qxHead = data->RigidBodies[i].qx;
      myOp.qyHead = data->RigidBodies[i].qy;
      myOp.qzHead = data->RigidBodies[i].qz;
      myOp.qwHead = data->RigidBodies[i].qw;

      printf("\tRigid Bodies \n");
      printf("\tHead\t%3.2f\n", myOp.PositionHeadX1);
      printf("\tHead\t%3.2f\n", myOp.PositionHeadY1);
      printf("\tHead\t%3.2f\n", myOp.PositionHeadZ1);
      printf("\tMeasurement Probe\t%3.2f\n", myOp.qxHead);
      printf("\tMeasurement Probe\t%3.2f\n", myOp.qyHead);
      printf("\tMeasurement Probe\t%3.2f\n", myOp.qzHead);
      printf("\tMeasurement Probe\t%3.2f\n", myOp.qwHead);
    } else if (data->RigidBodies[i].ID == 1002) {
      myOp.PositionCoilX1 = data->RigidBodies[i].x;
      myOp.PositionCoilY1 = data->RigidBodies[i].y;
      myOp.PositionCoilZ1 = data->RigidBodies[i].z;
      myOp.qxCoil = data->RigidBodies[i].qx;
      myOp.qyCoil = data->RigidBodies[i].qy;
      myOp.qzCoil = data->RigidBodies[i].qz;
      myOp.qwCoil = data->RigidBodies[i].qw;

      printf("\tRigid Bodies \n");
      printf("\tCoil\t%3.2f\n", myOp.PositionCoilX1);
      printf("\tCoil\t%3.2f\n", myOp.PositionCoilY1);
      printf("\tCoil\t%3.2f\n", myOp.PositionCoilZ1);
      printf("\tMeasurement Probe\t%3.2f\n", myOp.qxCoil);
      printf("\tMeasurement Probe\t%3.2f\n", myOp.qyCoil);
      printf("\tMeasurement Probe\t%3.2f\n", myOp.qzCoil);
      printf("\tMeasurement Probe\t%3.2f\n", myOp.qwCoil);
    }

  }
}

// Establish a NatNet Client connection
int OptitrackClient::connect_client() {
  // Release previous server
  g_pClient->Disconnect();

  // Init Client and connect to NatNet server
  int retCode = g_pClient->Connect(g_connectParams);
  if (retCode != ErrorCode_OK) {
    printf("Unable to connect to server.  Error code: %d. Exiting.\n", retCode);
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
    printf("Error initializing client. See log for details. Exiting.\n");

  } else {
    printf("Client initialized and ready.\n");
  }
}

int OptitrackClient::disconnect_client() { // Done - clean up.
  if (g_pClient) {
    g_pClient->Disconnect();
    delete g_pClient;
    g_pClient = NULL;
  }
  return ErrorCode_OK;
}

void OptitrackClient::connect_to_server(int argc, int serverIndex) {
  unsigned char ver[4];
  NatNet_GetVersion(ver);
  printf("NatNet Sample Client (NatNet ver. %d.%d.%d.%d)\n", ver[0], ver[1], ver[2], ver[3]);

  // Install logging callback
  //NatNet_SetLogCallback( MessageHandler );

  // create NatNet client
  g_pClient = new NatNetClient();
  g_pClient->SetFrameReceivedCallback(receive_rigidbody_data, g_pClient);
  // set the frame callback handler
  //g_pClient->SetFrameReceivedCallback( DataHandler, g_pClient );	// this function will receive data from the server
  // If no arguments were specified on the command line,
  // attempt to discover servers on the local network.
  if (argc == 1) {
    // An example of synchronous server discovery.
#if 0
    const unsigned int kDiscoveryWaitTimeMillisec = 5 * 1000; // Wait 5 seconds for responses.
    const int kMaxDescriptions = 10; // Get info for, at most, the first 10 servers to respond.
    sNatNetDiscoveredServer servers[kMaxDescriptions];
    int actualNumDescriptions = kMaxDescriptions;
    NatNet_BroadcastServerDiscovery( servers, &actualNumDescriptions );

    if ( actualNumDescriptions < kMaxDescriptions )
    {
        // If this happens, more servers responded than the array was able to store.
    }
#endif

    // Do asynchronous server discovery.
    printf("Looking for servers on the local network.\n");
    printf("Press the number key that corresponds to any discovered server to connect to that server.\n");
    printf("Press Q at any time to quit.\n\n");

    NatNetDiscoveryHandle discovery;
    NatNet_CreateAsyncServerDiscovery(&discovery, server_discovered_callback);
    sleep(5);

    const sNatNetDiscoveredServer &discoveredServer = g_discoveredServers[serverIndex - 1];

    if (discoveredServer.serverDescription.bConnectionInfoValid) {
      // Build the connection parameters.

      snprintf(

          g_discoveredMulticastGroupAddr, sizeof g_discoveredMulticastGroupAddr,
          "%" PRIu8 ".%" PRIu8".%" PRIu8".%" PRIu8"",
          discoveredServer.serverDescription.ConnectionMulticastAddress[0],
          discoveredServer.serverDescription.ConnectionMulticastAddress[1],
          discoveredServer.serverDescription.ConnectionMulticastAddress[2],
          discoveredServer.serverDescription.ConnectionMulticastAddress[3]
      );

      g_connectParams.connectionType = discoveredServer.serverDescription.ConnectionMulticast ? ConnectionType_Multicast
                                                                                              : ConnectionType_Unicast;
      g_connectParams.serverCommandPort = discoveredServer.serverCommandPort;
      g_connectParams.serverDataPort = discoveredServer.serverDescription.ConnectionDataPort;
      g_connectParams.serverAddress = discoveredServer.serverAddress;
      g_connectParams.localAddress = discoveredServer.localAddress;
      g_connectParams.multicastAddress = g_discoveredMulticastGroupAddr;
    }

    NatNet_FreeAsyncServerDiscovery(discovery);
  }

}

void NATNET_CALLCONV
OptitrackClient::server_discovered_callback(const sNatNetDiscoveredServer *pDiscoveredServer, void *pUserContext) {
  char serverHotkey = '.';
  if (g_discoveredServers.size() < 9) {
    serverHotkey = static_cast<char>('1' + g_discoveredServers.size());
  }

  printf("[%c] %s %d.%d at %s ",
         serverHotkey,
         pDiscoveredServer->serverDescription.szHostApp,
         pDiscoveredServer->serverDescription.HostAppVersion[0],
         pDiscoveredServer->serverDescription.HostAppVersion[1],
         pDiscoveredServer->serverAddress);

  if (pDiscoveredServer->serverDescription.bConnectionInfoValid) {
    printf("(%s)\n", pDiscoveredServer->serverDescription.ConnectionMulticast ? "multicast" : "unicast");
  } else {
    printf("(WARNING: Legacy server, could not autodetect settings. Auto-connect may not work reliably.)\n");
  }

  g_discoveredServers.push_back(*pDiscoveredServer);
}

void OptitrackClient::get_datastream() {
  g_pClient->SetFrameReceivedCallback(receive_rigidbody_data, g_pClient);

}