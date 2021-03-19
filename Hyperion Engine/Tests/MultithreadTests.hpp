/*==================================================================================================
	Hyperion Engine
	Tests/MultithreadTests.hpp
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/TaskManager.h"


namespace Hyperion
{
	namespace Tests
	{
		void RunMultithreadTests()
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
			Console::WriteLine( "[Test] Starting multi-threading test...." );
			Console::WriteLine( "--->Creating a tracked task, and measuring time to spin up, and time to spin down...\n" );

			/*
			double avgSpinUp = 0.0;
			double avgSpinDown = 0.0;

			for( int i = 0; i < 1000; i++ )
			{
				auto bef = std::chrono::high_resolution_clock::now();
				auto tt = [ bef, i ]
				{
					return std::make_tuple( bef, std::chrono::high_resolution_clock::now(), i );
				};

				auto hnd = TaskManager::AddTask< std::tuple< std::chrono::high_resolution_clock::time_point, std::chrono::high_resolution_clock::time_point, int > >( tt );

				//hnd.Wait();
				//auto res = hnd.GetResult();
				auto res = hnd.WaitForResult();
				
				// Now, lets calculate how long it took for the task to spin up, and how long it took for the task to spin down
				std::chrono::duration< double, std::micro > spinDownTime = std::chrono::high_resolution_clock::now() - std::get< 1 >( res );
				std::chrono::duration< double, std::micro > spinUpTime = std::get< 1 >( res ) - std::get< 0 >( res );

				Console::WriteLine( "-----> Spin Up: ", spinUpTime.count(), "us \t Spin Down: ", spinDownTime.count(), "us \t Identifier: ", std::get< 2 >( res ) );

				avgSpinUp		+= spinUpTime.count();
				avgSpinDown		+= spinDownTime.count();

				std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
			}

			// Calculate our average times
			avgSpinUp /= 1000.0;
			avgSpinDown /= 1000.0;

			Console::WriteLine( "\n\n ----> Average Spin Up: ", avgSpinUp, "us \t Average Spin Down: ", avgSpinDown, "us" );
			*/

			// Were going to do a multi-producer/consumer test

			double avgSpinUpGroup = 0.0;
			double avgSpinDownGroup = 0.0;
			std::atomic< uint64 > avgSpinUpTask = 0.0;
			std::atomic< uint64 > avgSpinDownTask = 0.0;

			for( int i = 0; i < 10; i++ )
			{
				auto befi = std::chrono::high_resolution_clock::now();
				auto fi = [befi, &avgSpinUpTask, &avgSpinDownTask]
				{
					auto starti = std::chrono::high_resolution_clock::now();
					for( int j = 0; j < 50; j++ )
					{
						auto befj = std::chrono::high_resolution_clock::now();
						auto fj = [befj]
						{
							return std::make_pair( befj, std::chrono::high_resolution_clock::now() );
						};

						auto tj = TaskManager::AddTask( fj );
						auto resj = tj.WaitForResult();

						std::chrono::duration< double, std::micro > suj = resj.second - resj.first;
						std::chrono::duration< double, std::micro > sdj = std::chrono::high_resolution_clock::now() - resj.second;

						avgSpinUpTask += (uint64)suj.count();
						avgSpinDownTask += (uint64)sdj.count();

						Console::WriteLine( "--------> TaskGroup task.. Spin up time: ", suj.count(), "us \t Spin down time: ", sdj.count(), "us" );
					}

					std::chrono::duration< double, std::micro > sui = starti - befi;
					return std::make_pair( sui, std::chrono::high_resolution_clock::now() );
				};

				auto ti = TaskManager::AddTask( fi );
				auto resi = ti.WaitForResult();

				std::chrono::duration< double, std::micro > sdi = std::chrono::high_resolution_clock::now() - resi.second;
				Console::WriteLine( "----> TaskGroup complete.. Spin up time: ", resi.first.count(), "us \t Spin down time: ", sdi.count(), "us" );

				avgSpinUpGroup += resi.first.count();
				avgSpinDownGroup += sdi.count();
			}

			avgSpinUpGroup /= 10.0;
			avgSpinDownGroup /= 10.0;
			auto finalAvgTaskSpinDown = avgSpinDownTask.load() / 500;
			auto finalAvgTaskSpinUp = avgSpinUpTask.load() / 500;

			Console::WriteLine( "\n--> Average group spin up: ", avgSpinUpGroup, "us \t Average group spin down: ", avgSpinDownGroup, "us" );
			Console::WriteLine( "--> Average task spin up: ", finalAvgTaskSpinUp, "us \t Average task spin down: ", finalAvgTaskSpinDown, "us" );
		}
	}
}