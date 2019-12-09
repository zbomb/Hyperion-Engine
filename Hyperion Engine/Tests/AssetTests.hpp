/*==================================================================================================
	Hyperion Engine
	Tests/AssetTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <iostream>

#include "Hyperion/Core/AssetManager.h"
#include "Hyperion/Assets/TestAsset.h"
#include "Hyperion/Core/Async.h"


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

	int addTask( int a, int b )
	{
		std::cout << "-----> Add function running....\n";
		std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
		std::cout << "------> Add function finishing!\n";
		return a + b;
	}

	void RunAssetTests()
	{
		std::cout << "--------------------------------------------------------------------------------------------------\n---> Running asset tests!\n\n";

		/*

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
		Task::WaitAll( t, t2 );

		std::cout << "------> Both tasks complete!\n";

		std::cout << "\n--> Creating two more tasks....\n";

		auto t3 = Task::Create< void >( myTask );
		auto t4 = Task::Create< void >( myOtherTask );

		std::cout << "-----> Waiting for either..\n";
		Task::WaitAny( t3, t4 );
		std::cout << "----------> Complete! One of the tasks finished!\n";

		std::cout << "\n";

		std::cout << "\n\n---> Testing task result system...\n";
		auto add_task = Task::Create< int >( std::bind( addTask, 10, 5 ) );
		std::cout << "-------> Add task created!\n";

		auto res = add_task.GetResult();
		if( !res )
		{
			std::cout << "-----> Immediate result is null!\n";
		}
		else
		{
			std::cout << "------> Immediate result is: " << res << "\n";
		}

		std::cout << "\n";

		auto wait_res = add_task.GetResult( true );
		std::cout << "------> Got waited result!\n";
		if( wait_res )
		{
			int ires = wait_res.Get();
			std::cout << "-------> Wait Result: " << ires << "\n";
		}
		else
		{
			std::cout << "-------> Wait Result Null!\n";
		}

		*/

		// Lets test async asset loading...
		std::cout << "--> Testing async asset loading...\n";
		auto asset_task = AssetManager::LoadAsync< TestAsset >( "test_asset.htxt" );
		auto other_task = AssetManager::LoadAsync< TestAsset >( "test_asset.htxt" );

		std::cout << "-----> Waiting for asset...\n";
		asset_task.Wait();
		std::cout << "-----> Asset loaded!\n";
		
		auto asset = asset_task.GetResultRaw();
		if( asset )
		{
			std::cout << "----------> Asset Loaded: " << asset->m_Data << "\n";
		}
		else
		{
			std::cout << "----------> Asset wasnt loaded!\n";
		}

		std::cout << "-----> Checking other asset...\n";
		other_task.Wait();

		auto other_asset = other_task.GetResultRaw();
		if( other_asset )
		{
			std::cout << "-----------> Other Asset Loaded: " << other_asset->m_Data << "\n";
		}
		else
		{
			std::cout << "----------> Other asset failed to load!\n";
		}

		std::cout << "\n";

		std::cout << "---> Asset tests complete!\n";
		std::cout << "--------------------------------------------------------------------------------------------------\n\n";
	}
}
}