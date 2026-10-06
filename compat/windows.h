// Native-build <windows.h> shim for BoB leaf code.
// Forwards to migalley's proven Win32 type layer (WIN32_COMPAT.H) so BoB
// headers see the same DWORD/HWND/WINAPI environment as the MiG port.
// Not a complete Win32 — extend only as leaf code requires.
#ifndef ROWAN_COMPAT_WINDOWS_H
#define ROWAN_COMPAT_WINDOWS_H

// C++-style COM IUnknown for headers that call ->Release() (BoB uses the
// member-call form; migalley's C lpVtbl stub is suppressed via its own guard).
#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__
struct IUnknown {
    virtual long   QueryInterface(const void* riid, void** ppv) = 0;
    virtual unsigned long AddRef()  = 0;
    virtual unsigned long Release() = 0;
};
typedef IUnknown* LPUNKNOWN;
#endif

#include "WIN32_COMPAT.H"

// MSVC spellings that BoB headers use; harmless no-ops/aliases under GCC.
#ifndef __declspec
#define __declspec(x)
#endif
#ifndef __int16
#define __int16 short
#define __int32 int
#endif
typedef struct HMONITOR__* HMONITOR;
typedef void* HINSTANCE;   // matches dinput_stub.h (identical re-typedef is legal)

#ifndef DEFINE_GUID
#define DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
    static const GUID name = { l, w1, w2, { b1,b2,b3,b4,b5,b6,b7,b8 } }
#endif



// --- Win32 sync/input: pthread-backed CRITICAL_SECTION (process-local mutex) ---
// Guarded: identical block also lives in migalley/H/WIN32_COMPAT.H — first
// include wins (identical definitions either way).
#ifndef ROWAN_CS_IMPL
#define ROWAN_CS_IMPL
#include <pthread.h>
struct CRITICAL_SECTION { pthread_mutex_t m; };
typedef CRITICAL_SECTION* LPCRITICAL_SECTION;
inline void InitializeCriticalSection(LPCRITICAL_SECTION c) {
    pthread_mutexattr_t a; pthread_mutexattr_init(&a);
    pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);  // Win32 CS is recursive
    pthread_mutex_init(&c->m, &a); pthread_mutexattr_destroy(&a);
}
inline void DeleteCriticalSection(LPCRITICAL_SECTION c) { pthread_mutex_destroy(&c->m); }
inline void EnterCriticalSection(LPCRITICAL_SECTION c)  { pthread_mutex_lock(&c->m); }
inline void LeaveCriticalSection(LPCRITICAL_SECTION c)  { pthread_mutex_unlock(&c->m); }
#endif /* ROWAN_CS_IMPL */

// --- Bitmap compression constants (BITMAPINFOHEADER.biCompression) ---
#ifndef BI_RGB
#define BI_RGB       0
#define BI_RLE8      1
#define BI_RLE4      2
#define BI_BITFIELDS 3
#endif

// --- Win32 console API: BoB's monotxt.cpp debug console. The original makefile
// notes the Windows build contains MONOTXT "without debugs so calls succeed but
// do nothing" — mirror that with no-op implementations under POSIX.
#ifndef STD_OUTPUT_HANDLE
#define STD_OUTPUT_HANDLE ((unsigned long)-11)
#define STD_ERROR_HANDLE  ((unsigned long)-12)
struct COORD { short X, Y; };
struct CONSOLE_SCREEN_BUFFER_INFO {
    COORD dwSize, dwCursorPosition;
    unsigned short wAttributes;
    struct { short Left, Top, Right, Bottom; } srWindow;
    COORD dwMaximumWindowSize;
};
inline void* GetStdHandle(unsigned long) { return (void*)0; }
inline int GetConsoleScreenBufferInfo(void*, CONSOLE_SCREEN_BUFFER_INFO* i) {
    i->dwSize.X = 80; i->dwSize.Y = 25; i->dwCursorPosition.X = 0; i->dwCursorPosition.Y = 0;
    return 1;
}
inline int FillConsoleOutputCharacter(void*, char, unsigned long, COORD, DWORD* w) {
    if (w) *w = 0; return 1;
}
inline int SetConsoleCursorPosition(void*, COORD) { return 1; }
inline int WriteConsoleOutputCharacter(void*, const char*, unsigned long n, COORD, unsigned long* w) {
    if (w) *w = n; return 1;
}
#endif

// --- MFC debug-new plumbing: BoB's DEBUG_NEW expands new to
// new(file,line); provide the overloads so leaf TUs compile ---
#ifdef __cplusplus
inline void* operator new(size_t n, const char* file, int line) {
    (void)file; (void)line; return ::operator new(n);
}
inline void* operator new[](size_t n, const char* file, int line) {
    (void)file; (void)line; return ::operator new[](n);
}
#endif

#endif // ROWAN_COMPAT_WINDOWS_H
