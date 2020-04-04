/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Macros.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <iostream> // for std::cerr


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
#else
#define HYPERION_VERIFY( condition, message ) do { } while( false )
#endif

// Not Implemented Macro
#ifndef NDEBUG
#define HYPERION_NOT_IMPLEMENTED( message ) \
	do { \
		std::cerr << "Hyperion Error: Attempt to run code path that is not yet implemented (" << \
		message << ") in '" << __FILE__ << "' at line " << __LINE__ << std::endl; \
		std::terminate(); \
	} while( false )
#else
#define HYPERION_NOT_IMPLEMENTED( message ) do { } while( false )
#endif

// Attributes
#define HYPERION_NODISCARD [[nodiscard]]
#define HYPERION_UNUSED [[maybe_unused]]