// MSVC <direct.h> shim — directory CRT functions BoB's winfile.cpp uses.
// Maps to POSIX equivalents; drive-letter APIs stubbed to C: semantics
// (BoB resolves real paths through roots.dir + the case resolver, not drives).
#ifndef ROWAN_COMPAT_DIRECT_H
#define ROWAN_COMPAT_DIRECT_H
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#ifndef _mkdir
static inline int _mkdir(const char* p) { return mkdir(p, 0755); }
#define mkdir _mkdir
#endif
#define _rmdir  rmdir
#define _chdir  chdir
#define _getcwd getcwd

// Drive-letter APIs: single "current drive" model — 3 = C:
static inline int _getdrive(void) { return 3; }
static inline unsigned _getdrives(void) { return (1u << 2); } // only C:
static inline int _chdrive(int d) { return d == 3 ? 0 : -1; }

// _MAX_DRIVE/_MAX_PATH already in limits via WIN32_COMPAT; guard anyway
#ifndef _MAX_DRIVE
#define _MAX_DRIVE 3
#endif
// _findfirst/_findnext/_findclose + fopen_ci: the proven migalley
// implementation (io_compat.cpp) — FNM_CASEFOLD matching and component-wise
// case-insensitive path resolution, the T103 resolver.
#include "io_compat.h"
#endif // ROWAN_COMPAT_DIRECT_H
