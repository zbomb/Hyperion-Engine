/*==================================================================================================
	Hyperion Engine
	Tests/AssetTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <iostream>

#include "Hyperion/Core/AssetManager.h"
#include "Hyperion/Assets/TestAsset.h"


namespace Hyperion
{
namespace Tests
{

	void myTask()
	{
		std::cout << "------> Running task A (1s)\n";
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
		std::cout << "------> Task A Complete!\n";
	}

	void myOtherTask()
	{
		std::cout << "-----> Running other task (300ms)\n";
		std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
		std::cout << "-----> Other task complete!\n";
	}

	void RunAssetTests()
	{
		std::cout << "--------------------------------------------------------------------------------------------------\n---> Running asset tests!\n\n";

		auto asset = AssetManager::LoadSync< TestAsset >( "test_asset.htxt" );
		if( asset.IsValid() )
		{
			std::cout << "---> Test asset valid!\n";
			std::cout << "-----> String Value: " << asset->m_Data << "\n";
		}
		else
		{
			std::cout << "----> Test asset invalid!\n";
		}


		std::cout << "\n\n--> Running some threading tests....\n";
		std::cout << "----> Creating two tasks....\n";

		auto t		= Task::Create< void >( myTask );
		auto t2		= Task::Create< void >( myOtherTask );

		std::cout << "------> Waiting for both...\n";
		Task::WaitAll( { t, t2 } );

		std::cout << "------> Both tasks complete!\n";

		std::cout << "---> Asset tests complete!\n";
		std::cout << "--------------------------------------------------------------------------------------------------\n\n";
	}
}
}