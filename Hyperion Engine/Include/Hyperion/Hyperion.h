/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Hyperion.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <iostream>


// Debug Defines
#define HYPERION_DEBUG
#define HYPERION_DEBUG_OBJECT
#define HYPERION_DEBUG_RENDERER

// External Defines


/*
	Engine Configuration
*/

// * Uncomment this to have the engine build in '2d' mode.. where the 2d renderer will be used
// and the 2d editor, along with the 2d runtime
//#define HYPERION_CONFIG_2D

#define HYPERION_DOC_FOLDER "hyperion/"



//////////////////////////////////////////////
/////// Do not edit below this line!!! ///////
//////////////////////////////////////////////

#ifndef HYPERION_CONFIG_2D
#define HYPERION_CONFIG_3D
#endif

// Operating System
#if _WIN32
#define HYPERION_OS_WIN32 true
#define WIN32_LEAN_AND_MEAN
#elif __ANDROID__
#define HYPERION_OS_ANDROID true
#elif __linux__
#define HYPERION_OS_LINUX true
#elif __APPLE__ && __MACH__
#define HYPERION_OS_MAC true
#else
static_assert( true, "[HYPERION] Couldnt detect the OS this is targetting!" );
#endif

// Renderer Support
#ifdef HYPERION_OS_WIN32
#define HYPERION_SUPPORT_DIRECTX
#endif


#include "Hyperion/Constants.h"
#include "Hyperion/Ints.h"
#include "Hyperion/Macros.h"
#include "Hyperion/Console/Console.h"
#include "Hyperion/Core/Types/Angle.h"
#include "Hyperion/Core/Types/Color.h"
#include "Hyperion/Core/Types/Point.h"
#include "Hyperion/Core/Types/Vector.h"

#include <sstream>
#include <thread>
#include <string>



/*
	Global Namespace Functions
*/
namespace Hyperion
{

	bool IsGameThread();
	bool IsRenderThread();
	bool IsWorkerThread();

	enum class ComparisonType
	{
		LESS_THAN,
		LESS_THAN_OR_EQUAL,
		EQUAL,
		GREATER_THAN,
		GREATER_THAN_OR_EQUAL
	};

	template< typename _F, typename _S >
	bool EvaluateComparison( const _F& first, const _S& second, ComparisonType compare )
	{
		switch( compare )
		{
		case ComparisonType::LESS_THAN:
			return first < second;
		case ComparisonType::LESS_THAN_OR_EQUAL:
			return first <= second;
		case ComparisonType::EQUAL:
			return first == second;
		case ComparisonType::GREATER_THAN:
			return first > second;
		case ComparisonType::GREATER_THAN_OR_EQUAL:
			return first >= second;
		default:
			return false;
		}
	}
}
