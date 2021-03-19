/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Hyperion.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include <iostream>

/*
*	Pre-processor Defines
*/

// Debugging
// The game can override these
#ifndef HYPERION_DEBUG
#ifndef HYPERION_RELEASE
#ifdef _DEBUG
#define HYPERION_DEBUG true
#else
#define HYPERION_RELEASE
#endif
#endif
#endif

#ifdef HYPERION_DEBUG
#ifdef HYPERION_RELEASE
static_assert( true, "[Preprocessor] Cannot use both DEBUG and RELEASE modes. Check project settings!" );
#endif
#endif

// Application Name & Data Folder
// The game can override these as well
#ifndef HYPERION_APP_NAME
#define HYPERION_APP_NAME "Hyperion Engine"
#endif

#ifndef HYPERION_DOC_FOLDER
#define HYPERION_DOC_FOLDER "hyperion/"
#endif

// Operating system
// Check if the source application has overriden our auto-OS selection
#ifndef HYPERION_OS_WIN32
#define HYPERION_OS_WIN32 false
#endif

#ifndef HYPERION_OS_LINUX
#define HYPERION_OS_LINUX false
#endif

// TODO: Add more OS support!

// If the OS wasnt override, then auto-select one based on any system specific preprocessor values
#if !HYPERION_OS_WIN32 && !HYPERION_OS_LINUX
#if _WIN32
#undef HYPERION_OS_WIN32
#define HYPERION_OS_WIN32 true
#elif __linux__
#undef HYPERION_OS_LINUX
#define HYPERION_OS_LINUX true
#else
static_assert( true, "Couldnt detect what operating system this build is supposed to be targetting!" );
#endif
#endif

// Determine what renderers will work with the current OS
#if HYPERION_OS_WIN32
#define HYPERION_SUPPORT_DX11 true
#define HYPERION_SUPPORT_OGL true
#elif HYPERION_OS_LINUX
#define HYPERION_SUPPORT_DX11 false
#define HYPERION_SUPPORT_OGL true
#endif

// Global Includes
#include "Hyperion/Constants.h"
#include "Hyperion/Ints.h"
#include "Hyperion/Macros.h"
#include "Hyperion/Console/Console.h"
#include "Hyperion/Core/Types/Color.h"
#include "Hyperion/Core/Types/Point.h"
#include "Hyperion/Library/Geometry.h"

#include <sstream>
#include <thread>
#include <string>

/*
*	Global Namespace Functions & Structures
*/
namespace Hyperion
{

	struct RawImageData
	{
		uint32 Width;
		uint32 Height;
		std::vector< byte > Data;
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

	constexpr bool IsDirectX11Supported()
	{
		#if HYPERION_SUPPORT_DX11
		return true;
		#else 
		return false;
		#endif
	}

	constexpr bool IsOpenGLSupported()
	{
		#if HYPERION_SUPPORT_OGL
		return true;
		#else
		return false;
		#endif
	}

}
