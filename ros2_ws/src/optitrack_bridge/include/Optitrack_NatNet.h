//
// Created by mtms on 11.10.2022.
//

#ifndef OPTITRACK_NATNET_OPTITRACK_NATNET_H
#define OPTITRACK_NATNET_OPTITRACK_NATNET_H

#include <NatNetTypes.h>
#include <NatNetCAPI.h>
#include <NatNetClient.h>
class optr
{
public:
    double PositionToolTipX1, PositionToolTipY1, PositionToolTipZ1;
    double YawToolTip, PitchToolTip, RollToolTip;
    double PositionHeadX1, PositionHeadY1, PositionHeadZ1;
    double YawHead, PitchHead, RollHead;
    double PositionCoilX1, PositionCoilY1, PositionCoilZ1;
    double YawCoil, PitchCoil, RollCoil;
    double qxToolTip, qyToolTip, qzToolTip, qwToolTip;
    double qxHead, qyHead, qzHead, qwHead;
    double qxCoil, qyCoil, qzCoil, qwCoil;
    int probeID, coilID, HeadID;
};
#endif //OPTITRACK_NATNET_OPTITRACK_NATNET_H
