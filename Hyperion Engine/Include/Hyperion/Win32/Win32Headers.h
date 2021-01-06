/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Win32/Win32Headers.h
	© 2020, Zachary Berry
==================================================================================================*/

#pragma once

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "../resource.h"