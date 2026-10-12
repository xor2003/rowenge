// <mmsystem.h> shim — multimedia API surface grows on demand (T007: audio
// goes through the Miles AIL adapter, not raw mmsystem).
#ifndef ROWAN_COMPAT_MMSYSTEM_H
#define ROWAN_COMPAT_MMSYSTEM_H
#include "windows.h"
// timeGetTime etc. live here on Windows; migalley's impl is proven.
#include "timeapi.h"
#endif
