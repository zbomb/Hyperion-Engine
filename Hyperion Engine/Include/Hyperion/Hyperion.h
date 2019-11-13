/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Hyperion.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <stdint.h>
#include <iostream>

// Debug Defines
#define HYPERION_DEBUG
#define HYPERION_DEBUG_OBJECT

/*
	Engine Configuration
*/

// * Uncomment this to have the engine build in '2d' mode.. where the 2d renderer will be used
// and the 2d editor, along with the 2d runtime
#define HYPERION_CONFIG_2D



//////////////////////////////////////////////
/////// Do not edit below this line!!! ///////
//////////////////////////////////////////////

#ifndef HYPERION_CONFIG_2D
#define HYPERION_CONFIG_3D
#endif

// Operating System
#if _WIN32
#define HYPERION_OS_WIN32 true
#elif __ANDROID__
#define HYPERION_OS_ANDROID true
#elif __linux__
#define HYPERION_OS_LINUX true
#elif __APPLE__ && __MACH__
#define HYPERION_OS_MAC true
#else
static_assert( true, "[HYPERION] Couldnt detect the OS this is targetting!" );
#endif

typedef int8_t		int8;
typedef uint8_t		uint8;
typedef int16_t		int16;
typedef uint16_t	uint16;
typedef int32_t		int32;
typedef uint32_t	uint32;
typedef int64_t		int64;
typedef uint64_t	uint64;
typedef uint8_t		byte;
typedef uint32		Char;


// Assert Macro
#ifndef NDEBUG
	#define HYPERION_VERIFY( condition, message ) \
	do { \
		if( !( condition ) ) { \
			std::cerr << "Hyperion Verify Failed: \"" << #condition << "\" failed in \"" \
				<< __FILE__ << " line " << __LINE__ << ": " << message << std::endl; \
			std::terminate(); \
		} \
	} while( false )
#elif
#define HYPERION_VERIFY( condition, message ) do { } while( false )
#endif

// Attributes
#define HYPERION_NODISCARD [[nodiscard]]
#define HYPERION_UNUSED [[maybe_unused]]


#include "Hyperion/Constants.h"
#include "Hyperion/Core/Types/Angle.h"
#include "Hyperion/Core/Types/Color.h"
#include "Hyperion/Core/Types/Point.h"
#include "Hyperion/Core/Types/Vector.h"

#include <thread>



/*
	Global Namespace Functions
*/
namespace Hyperion
{
	static std::thread::id __gGameThreadId;
	static std::thread::id __gRenderThreadId;
	static std::thread::id __gRenderMarshalThreadId;;

	bool IsGameThread();
	bool IsRenderThread();
	bool IsRenderMarshalThread();

	// TODO: Print Function?

}
