#ifndef KADR_WIN_INCLUDE_H
#define KADR_WIN_INCLUDE_H

#if defined(_WIN32)

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4005)  // do not complain about WIN32_LEAN_AND_MEAN redefinitions
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOUSER
#define NOGDI
#include <windows.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <shellapi.h>

#endif

#endif  //KADR_WIN_INCLUDE_H
