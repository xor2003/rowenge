// T107 family 3: shape layouts — the ShapeDesc/shape header structs are the
// on-disk binary format contracts: shapes.cpp walks raw file buffers as
// ShapeDesc records by sizeof (GetShapePtr/shapeptr+=sizeof(ShapeDesc)), so
// layout stability under #pragma pack(1) IS the contract. These asserts pin
// the pack(1) i386 layout — byte positions the binary loader depends on.
#include "../harness.h"
#include "DOSDefs.h"
#include "myangles.h"
#include "mymath.h"
#include "worldinc.h"          // ScaleType (used by shapes.h)
#include "3ddefs.h"
#include "shapes.h"
#include "animdata.h"
#include <cstddef>
#include <cstring>

REGISTER_TEST(test_bob_shape_layout_offsets) {
    // pack(1) layout — Type is 3 bytes (UWord bitfield unit + UByte unit)
    CHECK_EQ((int)offsetof(ShapeDesc, Type),             0);
    CHECK_EQ((int)sizeof(ShapeType),                     3);
    CHECK_EQ((int)offsetof(ShapeDesc, Size),             3);
    CHECK_EQ((int)offsetof(ShapeDesc, LauncherDataSize), 5);
    CHECK_EQ((int)offsetof(ShapeDesc, CollisionOffset),  7);
    CHECK_EQ((int)offsetof(ShapeDesc, LightDataOffset),  9);
    CHECK_EQ((int)offsetof(ShapeDesc, AnimDataSize),    11);
    CHECK_EQ((int)offsetof(ShapeDesc, sx),              13);
    CHECK_EQ((int)offsetof(ShapeDesc, liveshpref),      19);
    CHECK_EQ((int)offsetof(ShapeDesc, deadshpref),      21);
    CHECK_EQ((int)offsetof(ShapeDesc, damagedshpref),   23);
    CHECK_EQ((int)offsetof(ShapeDesc, dyingshpref),     25);
    CHECK_EQ((int)offsetof(ShapeDesc, repairshpref),    27);
    // Vulnerability bitfield group sits in the UWord at offset 29
    CHECK_EQ((int)offsetof(ShapeDesc, DamageOffset),    31);
    CHECK_EQ((int)offsetof(ShapeDesc, RealVulnerability),33);
    CHECK_EQ((int)offsetof(ShapeDesc, EyeDeltaX),       35);
    CHECK_EQ((int)offsetof(ShapeDesc, MaxDrawRange),    37);
    CHECK_EQ((int)offsetof(ShapeDesc, InteractiveOffset),43);
    // total record size = the on-disk stride
    CHECK_EQ((int)sizeof(ShapeDesc),                    45);
}

REGISTER_TEST(test_bob_shape_bitfields) {
    // ShapeType packs as: bytes 0-1 UWord unit (shapenumber:12, VapourType:4),
    // byte 2 UByte unit (5 flags bits0-4, scaletype:2 bits5-6, hasWire bit7)
    ShapeType t;
    memset(&t, 0, sizeof t);
    t.shapenumber = 0x0FFF;
    CHECK_EQ(((UByte*)&t)[0], 0xFF);
    CHECK_EQ(((UByte*)&t)[1] & 0x0F, 0x0F);
    memset(&t, 0, sizeof t);
    t.scaletype = 3;
    CHECK_EQ(((UByte*)&t)[2] & 0x60, 0x60);
    t.hasWire = 1;
    CHECK_EQ(((UByte*)&t)[2] & 0x80, 0x80);
    // scaletype round-trip through the ScaleType enum values
    t.scaletype = SHP_GRP;
    CHECK_EQ(t.scaletype, SHP_GRP);

    // MinAnimData: nolaunchers:3 + 5 flags = 8 bits in one UByte
    CHECK_EQ((int)sizeof(MinAnimData), 1);
    MinAnimData mad;
    memset(&mad, 0, sizeof mad);
    mad.nolaunchers = 5;
    mad.IsInvisible = 1;
    CHECK_EQ(mad.nolaunchers, 5);          // 3-bit field holds 0..7
    mad.nolaunchers = 8;                   // truncates to 3 bits -> 0
    CHECK_EQ(mad.nolaunchers, 0);
}
