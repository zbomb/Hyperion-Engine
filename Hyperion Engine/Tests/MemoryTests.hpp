/*==================================================================================================
	Hyperion Engine
	Tests/MemoryTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

// Engine Includes
#include "Hyperion/Core/Library/Memory.hpp"

// STL Includes
#include <iostream>
#include <chrono>
#include <string>


namespace Hyperion
{
namespace Tests
{
	void RunMemoryTests()
	{
		std::cout << "\n---------------------------------------------------------------------------------------------\n[TEST] Running flyweight test...\n";

		{

			// Create flyweight cached string
			std::cout << "Creating first string.......\n";
			Flyweight< std::string > testStr( "This is a test!" );

			// Check if valid
			if( testStr )
			{
				std::cout << "The value is valid!\n";
			}
			else
			{
				std::cout << "The value is invalid!\n";
			}

			// Print this value out
			std::cout << "The test value is...\n\t";
			std::cout << *testStr;
			std::cout << "\n";

			// Create a second value of the same type
			std::cout << "Creating second string.....\n";
			Flyweight< std::string > secondStr( "This is a test!" );

			// Check if valid
			if( secondStr )
			{
				std::cout << "Second string valid!\n";
			}
			else
			{
				std::cout << "Second string invalid!\n";
			}

			// Print value out
			std::cout << "Second string value is...\n\t";
			std::cout << *secondStr;
			std::cout << "\n";

			// Create a bunch of values of the same value, but different type
			std::cout << "\nRunning int test....\n";
			std::cout << "Creating 20 uints with a value of 0xFF00FF00\n";
			uint32 TargetValue = 0xFF00FF00;
			std::vector< Flyweight< uint32 > > m_Values;

			for( int i = 0; i < 20; i++ )
			{
				std::cout << "-> Creating value number " << i << "\n";
				m_Values.push_back( Flyweight< uint32 >( TargetValue ) );
			}

			std::cout << "All uint values created!\n\n";


			// Print out cache debug info
#ifdef HYPERION_FLYWEIGHT_DEBUG
			std::cout << "\n========================= UINT32 CACHE =========================\n";
			_Impl_DefaultFlyweightCache< uint32 >::DebugCache();
			std::cout << "\n========================= STRING CACHE =========================\n";
			_Impl_DefaultFlyweightCache< std::string >::DebugCache();
#endif
			std::cout << "\n---------> Freeing all flyweights...\n";
		}

		// Now, all the flyweights we created go out of scope and everything should be freed

		// Print out cache debug info
#ifdef HYPERION_FLYWEIGHT_DEBUG
		std::cout << "\n\n\n========================= UINT32 CACHE =========================\n";
		_Impl_DefaultFlyweightCache< uint32 >::DebugCache();
		std::cout << "\n========================= STRING CACHE =========================\n";
		_Impl_DefaultFlyweightCache< std::string >::DebugCache();
#endif

		std::cout << std::endl;
		std::cout << "----> Memory Test Complete!\n";
		std::cout << "---------------------------------------------------------------------------------------------\n";
	}
}
}