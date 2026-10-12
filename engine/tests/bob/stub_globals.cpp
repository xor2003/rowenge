// Leaf-test stub globals for the BoB files/ link surface.
// Provides the game-global objects (_Error, Math_Lib, Save_Data, Image_Map,
// SHAPESTUFF) and the member functions fileman/winfile call on them, so the
// real fileman.o/winfile.o link without dragging in the whole game.
// NOT linked into any production target — test scaffold only (T103).
#include <stdio.h>
#include <string.h>
#include <direct.h>
#include "mymath.h"
#include "fileman.h"
#include "worldinc.h"
#include "imagemap.h"
#include "Shapes.h"
#include "savegame.h"
#include "mytime.h"

Error		_Error;
// Math_Lib is defined by math/math.cpp (bob_math), File_Man by
// files/fileman.cpp — only globals with no live definition are stubbed here.
SaveData	Save_Data;
ImageMap	Image_Map;
shapestuff	SHAPESTUFF;

// ctors/dtors whose real bodies live in unlinked TUs (imagemap.cpp/shapes.cpp)
ImageMap::ImageMap() {}
ImageMap::~ImageMap() {}
shapestuff::shapestuff() {}
shapestuff::~shapestuff() {}

// Error: keep Say/ExitMode quiet in tests; SayAndQuit must not kill the
// harness, so leaf tests stay away from paths that emit errors (checked by
// the deliberate-failure proofs instead).
char	Error::exit_string[2048];
Error::Error() { logfile = NULL; }
Error::~Error() {}
Error& Error::ExitMode() { return *this; }
Error& Error::Say(const char*, ...) { return *this; }
Error& Error::SayAndQuit(const char*, ...) { return *this; }
void Error::Quit() {}
// ReallyEmitSysErr is a macro for Say (myerror.h), not a real function.

void ImageMap::UnLoadImageMapPtrs() {}
void shapestuff::UnLoadRequiredShapes() {}

// T104: bfields static-init references the frame-time singleton. The real
// definition lives in unlinked game code; TimerCode is a trivial value type.
TimerCode Timer_Code;

// T105: dinput_stub.h's inline IDI_* vtable thunks emit GUID/RowanDI
// references in every TU that includes it (fileman/winfile pull it via the
// windows chain). The real definitions live in migalley Hardware/dinput_stub.cpp
// which is not linked here; these stubs only need to resolve, never run.
#include "dinput_stub.h"
const GUID IID_IDirectInputDeviceA = {
    0x5944e680, 0xc92e, 0x11cf,
    {0xbf, 0x8b, 0x00, 0xaa, 0x00, 0x6c, 0xe2, 0x14}
};
const GUID IID_IDirectInputDevice2A = {
    0x5944e682, 0xc92e, 0x11cf,
    {0xbf, 0x8b, 0x00, 0xaa, 0x00, 0x6c, 0xe2, 0x14}
};
const GUID IID_IDirectInputA = { 0x89521360, 0xaa8a, 0x11cf, { 0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 } };
HRESULT IDirectInputA::EnumDevices(DWORD, LPDIENUMDEVICESCALLBACK, LPVOID, DWORD) { return DI_OK; }
void RowanDISetKeyNotify(HANDLE) {}
void RowanDIKeyWatchEnsure() {}
DWORD RowanDIKeyDrain(DIDEVICEOBJECTDATA*, DWORD, bool) { return 0; }
