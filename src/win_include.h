#ifndef KADR_WIN_INCLUDE_H
#define KADR_WIN_INCLUDE_H

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOUSER
#define NOGDI
#include <windows.h>

#include <shellapi.h>

#endif

#endif  //KADR_WIN_INCLUDE_H
