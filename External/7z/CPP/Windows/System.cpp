// Windows/System.cpp

#include "StdAfx.h"

#include "../Common/Defs.h"

#include "System.h"

namespace NWindows {
namespace NSystem {

UInt32 GetNumberOfProcessors()
{
#ifdef _WINDOWS
  SYSTEM_INFO systemInfo;
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
  GetNativeSystemInfo(&systemInfo);
#else
  GetSystemInfo(&systemInfo);
#endif
  return (UInt32)systemInfo.dwNumberOfProcessors;
#else
  return 1;
#endif
}

#ifdef _WINDOWS

#ifndef UNDER_CE

#if !defined(_WIN64) && defined(__GNUC__)

typedef struct _MY_MEMORYSTATUSEX {
  DWORD dwLength;
  DWORD dwMemoryLoad;
  DWORDLONG ullTotalPhys;
  DWORDLONG ullAvailPhys;
  DWORDLONG ullTotalPageFile;
  DWORDLONG ullAvailPageFile;
  DWORDLONG ullTotalVirtual;
  DWORDLONG ullAvailVirtual;
  DWORDLONG ullAvailExtendedVirtual;
} MY_MEMORYSTATUSEX, *MY_LPMEMORYSTATUSEX;

#else

#define MY_MEMORYSTATUSEX MEMORYSTATUSEX
#define MY_LPMEMORYSTATUSEX LPMEMORYSTATUSEX

#endif

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
#else
typedef BOOL (WINAPI *GlobalMemoryStatusExP)(MY_LPMEMORYSTATUSEX lpBuffer);
#endif

#endif

UInt64 GetRamSize()
{
#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
  return 0;
#else
  #ifndef UNDER_CE
  MY_MEMORYSTATUSEX stat;
  stat.dwLength = sizeof(stat);
  #endif
  #ifdef _WIN64
  if (!::GlobalMemoryStatusEx(&stat))
    return 0;
  return MyMin(stat.ullTotalVirtual, stat.ullTotalPhys);
  #else
  #ifndef UNDER_CE
  GlobalMemoryStatusExP globalMemoryStatusEx = (GlobalMemoryStatusExP)
      ::GetProcAddress(::GetModuleHandle(TEXT("kernel32.dll")), "GlobalMemoryStatusEx");
  if (globalMemoryStatusEx != 0 && globalMemoryStatusEx(&stat))
    return MyMin(stat.ullTotalVirtual, stat.ullTotalPhys);
  #endif
  {
    MEMORYSTATUS stat;
    stat.dwLength = sizeof(stat);
    ::GlobalMemoryStatus(&stat);
    return MyMin(stat.dwTotalVirtual, stat.dwTotalPhys);
  }
  #endif
#endif
}

#endif

}}
