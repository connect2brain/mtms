//
// Created by mtms on 12.10.2022.
//

#ifndef OPTITRACK_NATNET_DEFINITIONS_H
#define OPTITRACK_NATNET_DEFINITIONS_H
#include <vector>

#include <NatNetTypes.h>
#include <NatNetCAPI.h>
#include <NatNetClient.h>
#include <Optitrack_NatNet.h>
static const ConnectionType kDefaultConnectionType = ConnectionType_Multicast;

NatNetClient* g_pClient = NULL;

std::vector< sNatNetDiscoveredServer > g_discoveredServers;
sNatNetClientConnectParams g_connectParams;
char g_discoveredMulticastGroupAddr[kNatNetIpv4AddrStrLenMax] = NATNET_DEFAULT_MULTICAST_ADDRESS;
int g_analogSamplesPerMocapFrame = 0;
sServerDescription g_serverDescription;
#endif //OPTITRACK_NATNET_DEFINITIONS_H
