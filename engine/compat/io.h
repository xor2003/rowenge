// MSVC <io.h> shim — find* family + low-level io; migalley's io_compat
// supplies the proven implementations.
#ifndef ROWAN_COMPAT_IO_H
#define ROWAN_COMPAT_IO_H
#include "io_compat.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#define _open   open
#define _close  close
#define _read   read
#define _write  write
#define _lseek  lseek
#define _tell   tell
#define _sopen  open
#define _commit fsync
#endif
