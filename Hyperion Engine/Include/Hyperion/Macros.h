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
#elif
#define HYPERION_VERIFY( condition, message ) do { } while( false )
#endif

// Attributes
#define HYPERION_NODISCARD [[nodiscard]]
#define HYPERION_UNUSED [[maybe_unused]]