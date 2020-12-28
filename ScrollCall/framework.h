// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Cannot do WIN32_LEAN_AND_MEAN with GDIPLUS as it is too greedy!
// Workaround: #include <ole2.h>

// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <ole2.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
