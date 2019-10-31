/*==================================================================================================
	Hyperion Engine
	Source/Core/Thread.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/Thread.h"
#include <iostream>
#include <chrono>


namespace Hyperion
{


	Thread::Thread()
	{
		m_Status		= Thread::Status::NotStarted;
		m_bKillThread	= false;
	}

	Thread::~Thread()
	{
		
	}

	void Thread::Initialize()
	{

	}

	void Thread::Shutdown()
	{
		// If the thread is running, were going to perform a blocking stop of the thread
		Stop();
	}

	void Thread::RunThread()
	{
		// Perform init function
		if( m_InitFunction )
		{
			m_InitFunction();
		}

		// Start running tick function
		while( m_TickFunction && !m_bKillThread )
		{
			std::chrono::time_point< std::chrono::high_resolution_clock > tickStart = std::chrono::high_resolution_clock::now();
			m_TickFunction();
			std::chrono::time_point< std::chrono::high_resolution_clock > tickEnd = std::chrono::high_resolution_clock::now();

			// Calculate how many microseconds it took to run the tick function
			auto tickTime = std::chrono::duration_cast< std::chrono::microseconds >( tickEnd - tickStart ).count();

			// Check for abort
			if( m_bKillThread )
				break;

			// Check for and run any async operations
			static const int MaxAsyncOperations = 5;
			int OperationCount = 0;

			std::lock_guard< std::mutex > Lock( m_AsyncQueueMutex );
			while( !m_AsyncQueue.empty() && OperationCount < MaxAsyncOperations )
			{
				// Get next async operation
				auto& nextFunc = m_AsyncQueue.front();

				// Run function
				if( nextFunc )
					nextFunc();

				m_AsyncQueue.pop();
				OperationCount++;
			}
		}

		if( m_ShutdownFunction )
		{
			m_ShutdownFunction();
		}
	}

	bool Thread::Start()
	{
		if( m_Status == Thread::Status::Running )
		{
			std::cout << "[ERROR] Thread: Cannot run thread, its already running!\n";
			return false;
		}
		else if( !m_TickFunction )
		{
			std::cout << "[ERROR] Thread: Cannot run thread, tick function isnt set!\n";
			return false;
		}
		else if( m_Thread )
		{
			// Destroy old thread?
			m_Thread.reset();
		}

		// Set members
		m_Status		= Thread::Status::Running;
		m_bKillThread	= false;

		// Create the thread and run
		m_Thread = std::make_unique< std::thread >( std::bind( &Thread::RunThread, this ) );

		return true;
	}

	bool Thread::Stop()
	{
		// Well, we dont really care what the status is, we care about what the actual underlying thread is doing
		if( m_Thread )
		{
			// Check if the thread is joinable
			if( m_Thread->joinable() )
			{
				// Set the atomic shutdown flag, join thread
				// Maybe in the future, a detach is better? Either way we just want to make sure it shuts down
				m_bKillThread = true;
				m_Thread->join();
			}
			else
			{
				std::cout << "[ERROR] Thread: Failed to stop thread.. thread wasnt joinable!\n";
				return false;
			}
		}

		m_Status = Thread::Status::Stopped;
		m_Thread.reset();

		return true;
	}

	void Thread::RunTask( std::function< void() > inFunc )
	{
		// Ensure the thread is actually running first
		if( !m_Thread || m_Status != Thread::Status::Running || m_bKillThread )
		{
			std::cout << "[ERROR] Thread: Attempt to run task on thread that isnt running!\n";
			return;
		}

		// ENsure we were passed a valid function
		if( !inFunc )
		{
			std::cout << "[ERROR] Thread: Attempt to run empty function on thread!\n";
			return;
		}

		// Add function to async task queue
		std::lock_guard< std::mutex > Lock( m_AsyncQueueMutex );
		m_AsyncQueue.push( inFunc );

	}

	std::thread::id Thread::GetSystemIdentifier()
	{
		if( m_Thread )
		{
			return m_Thread->get_id();
		}
		else
		{
			return std::thread::id();
		}
	}

}