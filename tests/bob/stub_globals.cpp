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
