/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Managers/ThreadManager.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Threading.h"


namespace Hyperion
{

	class ThreadManager
	{

	private:

		static std::map< std::string, std::shared_ptr< TickedThread > > m_TickedThreads;
		static std::map< std::string, std::shared_ptr< CustomThread > > m_CustomThreads;
		static std::vector< std::shared_ptr< PoolWorkerThread > > m_TaskPoolThreads;

		static bool m_bRunning;

	public:

		ThreadManager() = delete;

		static bool Start( uint32 inFlags = 0 );
		static bool Stop();

		template< typename T >
		static TaskHandle< T > CreateTask( std::function< T( void ) > inFunc, std::string inTargetThread = THREAD_POOL )
		{
			// Validate input function
			if( !inFunc )
			{
				Console::WriteLine( "[ERROR] TaskManager: Failed to create task.. null function provided!" );
				return TaskHandle< T >( nullptr );
			}

			// Create a new task instance
			auto inst = std::make_unique< TaskInstance< T > >( inFunc );

			// Then, create a new task handle pointing to this instance to return to the caller
			TaskHandle< T > handle( *inst );

			if( inTargetThread == THREAD_POOL )
			{
				// Aquire lock on the task list
				{
					std::lock_guard< std::mutex > lock( TaskPool::m_TaskMutex );

					// Insert into the queue and notify any threads waiting
					TaskPool::m_TaskList.push( std::move( inst ) );
					TaskPool::m_TaskBool = true;
				}

				// The boolean will be reset by whichever worker thread wakes up from this call
				TaskPool::m_TaskCV.notify_one();
			}
			else
			{
				// Find the thread, and inject the task
				auto thread = GetThread( inTargetThread );
				if( !thread )
				{
					Console::WriteLine( "[ERROR] TaskManager: Failed to create task.. invalid target thread '", inTargetThread, "'" );
					return TaskHandle< T >( nullptr );
				}

				if( !thread->InjectTask( std::move( inst ) ) )
				{
					Console::WriteLine( "[ERROR] Taskmanager: Failed to create task.. thread '", inTargetThread, "' did not accept the task" );
					return TaskHandle< T >( nullptr );
				}
			}

			return handle;
		}


		static std::shared_ptr< Thread > CreateThread( const TickedThreadParameters& );
		static std::shared_ptr< Thread > CreateThread( const CustomThreadParameters& );
		static std::shared_ptr< Thread > GetThread( const std::string& );
		static bool DestroyThread( const std::string& );
		static uint32 GetThreadCount();

		static bool IsWorkerThread( const std::thread::id& inIdentifier );

	};

}