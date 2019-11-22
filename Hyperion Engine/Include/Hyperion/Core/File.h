/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/File.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"


#if HYPERION_OS_WIN32
#include "Hyperion/Win32/Win32FileSystem.h"
namespace Hyperion
{
	typedef FileSystem_Impl< Win32FileSystem > IFileSystem;
}
#elif HYPERION_OS_MAC
#include "Hyperion/MacOS/MacOSFileSystem.h"
namespace Hyperion
{
	typedef FileSystem_Impl< MacOSFileSystem > IFileSystem;
}
#elif HYPERION_OS_LINUX
#include "Hyperion/Linux/LinuxFileSystem.h"
namespace Hyperion
{
	typedef FileSystem_Impl< LinuxFileSystem > IFileSystem;
}
#elif HYPERION_OS_ANDROID
#include "Hyperion/Android/AndroidFileSystem.h"
namespace Hyperion
{
	typedef FileSystem_Impl< AndroidFileSystem > IFileSystem;
}
#endif