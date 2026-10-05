// Dead-link stubs for missman/packages.cpp.
// packages.cpp has a file-scope `PackageList Todays_Packages` — its ctor/dtor
// sit in .init_array, so --gc-sections retains the whole member graph, and
// the functions inside it reference these globals/members. Every symbol here
// is reachable ONLY from functions the tests never call; bodies are trivial.
#include "harness.h"
#include "DOSDefs.h"
#include "myangles.h"
#include "mymath.h"
#include "worldinc.h"
#include "planetyp.h"
#include "fileman.h"     // FileNum for uiicons.h
#include "bfnumber.h"   // BFNUMBER_Included: infoitem.h gates info_grndgrp et al on it
#include "infoitem.h"
#include "nodebob.h"
#include "package.h"
#include "sky.h"

// Node_Data (a NodeData object) is blobbed in test_bob_nodedata_stub.cpp —
// its ctor/dtor have unlinked deps; only dead-path code references it.
PlaneInit* Plane_Type_Translate[8] = {};      // global pointer table

static char blob[512];
Target&   NodeData::GetTarget(UniqueID)      { return *reinterpret_cast<Target*>(blob); }
Squadron& NodeData::operator[](SquadNum)     { return *reinterpret_cast<Squadron*>(blob); }
int    info_grndgrp::GetCruiseAt(Coords3D&, ANGLES&) { return 0; }
FP     Atmosphere::GetWind(SLong, SLong&, SLong&, SLong&) { return 0; }
void   PackageList::MessageBoxFuelWarning(SquadNum) {}
PlaneTypeSelect Squadron::AcType()           { return (PlaneTypeSelect)0; }
UniqueID TargetFighterField::getsectoruid()  { return UID_NULL; }
TargetSectorField* TargetFighterField::getsectoraf() { return nullptr; }
void   AirStruc::GenWaypointOffsetSub(FormationTypeIndex, WaypointOffsetSub&, bool) {}
