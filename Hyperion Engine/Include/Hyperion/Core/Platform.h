/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Platform.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


#if HYPERION_OS_WIN32
#include "Hyperion/Win32/Win32Platform.h"
namespace Hyperion
{
	typedef PlatformImpl< Win32PlatformServices > Platform;
}
#elif HYPERION_OS_MAC
#include "Hyperion/MacOS/MacOSPlatform.h"
namespace Hyperion
{
	typedef PlatformImpl< MacOSPlatformServices > Platform;
}
#elif HYPERION_OS_LINUX
#include "Hyperion/Linux/LinuxPlatform.h"
namespace Hyperion
{
	typedef PlatformImpl< LinuxPlatformServices > Platform;
}
#elif HYPERION_OS_ANDROID
#include "Hyperion/Android/AndroidPlatform.h"
namespace Hyperion
{
	typedef PlatformImpl< AndroidPlatformServices > Platform;
}
#endif