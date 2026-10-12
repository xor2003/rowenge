// MSVC <process.h> shim — process/thread CRT entry points.
#ifndef ROWAN_COMPAT_PROCESS_H
#define ROWAN_COMPAT_PROCESS_H
#include <unistd.h>
#define _getpid getpid
// _beginthread/_endthread arrive with a threading owner (no live user yet).
#endif
