// PE-runner force-include (-include pe_compat.h).
// Supplies the MFC DEBUG_NEW placement-new overloads that mingw lacks —
// BoB code compiled with NDEBUG undefined calls new(size, file, line).
#ifndef PE_COMPAT_H
#define PE_COMPAT_H
#include <new>
#include <cstdlib>
inline void* operator new(size_t n, const char*, int)      { return ::operator new(n); }
inline void  operator delete(void* p, const char*, int)    { ::operator delete(p); }
inline void* operator new[](size_t n, const char*, int)    { return ::operator new[](n); }
inline void  operator delete[](void* p, const char*, int)  { ::operator delete[](p); }
#endif
