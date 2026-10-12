// MSVC <new.h> shim — set_new_handler/_PNH decls -> standard <new>
#ifndef ROWAN_COMPAT_NEW_H
#define ROWAN_COMPAT_NEW_H
#include <new>
#ifndef _PNH_DEFINED
#define _PNH_DEFINED
typedef int (__cdecl* _PNH)(size_t);
#endif
#ifndef _set_new_handler
#define _set_new_handler(p) std::set_new_handler((void (*)())(p))
#endif
#endif
