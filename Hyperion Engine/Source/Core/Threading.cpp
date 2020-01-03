/*==================================================================================================
	Hyperion Engine
	Source/Core/Threading.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/Threading.h"
#include "Hyperion/Core/Engine.h"

#include <chrono>


namespace Hyperion
{
	/*
		Static Definitions
	*/
	std::mutex TaskPool::m_TaskMutex;
	std::stack< std::unique_ptr< TaskInstanceBase > > TaskPool::m_TaskList;
	std::condition_variable TaskPool::m_TaskCV;
	bool TaskPool::m_TaskBool( false );

	/*----------------------------------------------------------------------------
		TickedThread::TickedThread
	----------------------------------------------------------------------------*/
	TickedThread::TickedThread( const TickedThreadParameters& params )
		: Thread( params.Identifier, params.AllowTasks ), m_Init( params.InitFunction ), m_Main( params.TickFunction ), m_Shutdown( params.ShutdownFunction ),
		m_Frequency( params.Frequency > 0.f ? ( 1.f / params.Frequency ) * 1000.f : 0.f ), m_Deviation( params.Deviation ), m_MinTasks( params.MinimumTasksPerTick ), m_MaxTasks( params.MaximumTasksPerTick )
	{
	}

	/*----------------------------------------------------------------------------
		TickedThread::Start
	----------------------------------------------------------------------------*/
	bool TickedThread::Start()
	{
		// Check state to see if we can start this thread
		if( m_State || m_Handle )
		{
			std::cout << "[ERROR] ThreadManager: Attempt to start thread '" << m_Identifier << "' while it was already running\n";
			return false;
		}
		else if( !m_Main )
		{
			std::cout << "[ERROR] ThreadManager: Attempt to start thread '" << m_Identifier << "' without a valid main function\n";
			return false;
		}

		// Create OS thread and set state
		m_State		= true;
		m_Handle	= std::make_unique< std::thread >( std::bind( &TickedThread::ThreadMain, this ) );

		return true;
	}

	/*----------------------------------------------------------------------------
		TickedThread::Stop
	----------------------------------------------------------------------------*/
	bool TickedThread::Stop()
	{
		// Once this state is set to false, the thread should stop at the start of the next iteration before calling the main func
		m_State = false;

		if( m_Handle )
		{
			// Check if the active thread is joinable, meaning we can properly shut it down
			if( m_Handle->joinable() )
			{
				// The join call is going to block this thread until the thread were shutting down completes
				m_Handle->join();
				m_Handle.reset();
			}
			else
			{
				std::cout << "[ERROR] ThreadManager: Failed to properly stop thread '" << m_Identifier << "' because it wasnt joinable!\n";
				return false;
			}
		}

		return true;
	}

	/*----------------------------------------------------------------------------
		TickedThread::RunNextTask
	----------------------------------------------------------------------------*/
	bool TickedThread::RunNextTask()
	{
		// There should already be a lock obtained before calling this function!
		std::unique_ptr< TaskInstanceBase > nextTask = nullptr;

		// Lock access to the task list when we go to pop the entry out
		{
			std::lock_guard< std::mutex > task_lock( m_TaskMutex );

			while( !nextTask && !m_TaskList.empty() )
			{
				nextTask = std::move( m_TaskList.top() );
				m_TaskList.pop();
			}
		}

		if( nextTask )
		{
			nextTask->Execute();
			return true;
		}
		else
		{
			return false;
		}
	}

	/*----------------------------------------------------------------------------
		TickedThread::ThreadMain
	----------------------------------------------------------------------------*/
	void TickedThread::ThreadMain()
	{
		// First, lets run our init function
		if( m_Init )
		{
			m_Init();
		}

		// Main thread loop begin
		while( m_Main && m_State )
		{
			// Store start of this tick, and calculate when the next tick should be
			auto tick_start		= std::chrono::high_resolution_clock::now();
			
			auto tick_delta		= std::chrono::duration_cast< std::chrono::nanoseconds >( m_Frequency );
			auto next_tick		= tick_start + tick_delta;

			// Run tick function
			m_Main();

			// Run the minimum number of tasks required, or until we run out of tasks
			uint32 taskCount = 0;
			while( taskCount < m_MinTasks && RunNextTask() )
			{
				taskCount++;
			}

			// Now, run more tasks until we either run out of time, hit the max number of tasks, or run out of tasks
			while( ( next_tick - std::chrono::high_resolution_clock::now() ) > m_Deviation && ( taskCount < m_MaxTasks || m_MaxTasks == 0 ) && RunNextTask() )
			{
				taskCount++;
			}

			// If we have leftover time, sleep until the next tick
			auto time_left = next_tick - std::chrono::high_resolution_clock::now();
			if( time_left > m_Deviation )
			{
				std::this_thread::sleep_until( next_tick - std::chrono::microseconds( 800 ) );
			}
		}

		// Ensure any pending tasks are ran.. and ensure no more tasks are able to be added
		m_State = false;

		{
			// Lock access to the task list, since m_State is already set to false, no other tasks should be added
			std::lock_guard< std::mutex > task_lock( m_TaskMutex );
			while( !m_TaskList.empty() )
			{
				RunNextTask();
			}
		}

		// Thread shutdown, run the shutdown func and update state
		if( m_Shutdown )
		{
			m_Shutdown();
		}
	}

	/*----------------------------------------------------------------------------
		TickedThread::InjectTask
	----------------------------------------------------------------------------*/
	bool TickedThread::InjectTask( std::unique_ptr< TaskInstanceBase >&& inTask )
	{
		// Ensure task is valid, thread is running, and we allow task injection
		if( inTask && m_State && m_AllowTasks )
		{
			std::lock_guard< std::mutex > task_lock( m_TaskMutex );
			m_TaskList.push( std::move( inTask ) );
			return true;
		}

		return false;
	}


	/*----------------------------------------------------------------------------
		CustomThread::CustomThread
	----------------------------------------------------------------------------*/
	CustomThread::CustomThread( const CustomThreadParameters& params )
		: Thread( params.Identifier, params.AllowTasks ), m_MainFunc( params.ThreadFunction )
	{
	}


	/*----------------------------------------------------------------------------
		CustomThread::Start
	----------------------------------------------------------------------------*/
	bool CustomThread::Start()
	{
		// Check if the thread is already running
		if( m_State || m_Handle )
		{
			std::cout << "[ERROR] ThreadManager: Attempt to start thread '" << m_Identifier << "' but its already running\n";
			return false;
		}
		else if( !m_MainFunc )
		{
			std::cout << "[ERROR] ThreadManager: Attempt to start thread '" << m_Identifier << "' but the thread function isnt bound\n";
			return false;
		}

		// Create the os thread and set state
		m_State = true;
		m_Handle = std::make_unique< std::thread >( std::bind( &CustomThread::ThreadMain, this ) );

		return true;
	}

	/*----------------------------------------------------------------------------
		CustomThread::Stop
	----------------------------------------------------------------------------*/
	bool CustomThread::Stop()
	{
		// The thread function is responsible for checking this atomic boolean for shutdown
		m_State = false;

		if( m_Handle )
		{
			// Check if the active thread is joinable, meaning we can properly shut it down
			if( m_Handle->joinable() )
			{
				// The join call is going to block this thread until the thread were shutting down completes
				m_Handle->join();
				m_Handle.reset();
			}
			else
			{
				std::cout << "[ERROR] ThreadManager: Failed to properly stop thread '" << m_Identifier << "' because it wasnt joinable!\n";
				return false;
			}
		}

		return true;
	}

	/*----------------------------------------------------------------------------
		CustomThread::InjectTask
	----------------------------------------------------------------------------*/
	bool CustomThread::InjectTask( std::unique_ptr< TaskInstanceBase >&& inTask )
	{
		if( inTask && m_State && m_AllowTasks )
		{
			std::lock_guard< std::mutex > task_lock( m_TaskMutex );
			m_TaskList.push( std::move( inTask ) );
			return true;
		}

		return false;
	}

	/*----------------------------------------------------------------------------
		CustomThread::PopTask
	----------------------------------------------------------------------------*/
	std::unique_ptr< TaskInstanceBase > CustomThread::PopTask()
	{
		std::unique_ptr< TaskInstanceBase > task = nullptr;

		// Obtain a lock on the task list so we can remove the next task from it
		{
			std::lock_guard< std::mutex > task_lock( m_TaskMutex );

			while( !task && !m_TaskList.empty() )
			{
				task = std::move( m_TaskList.top() );
				m_TaskList.pop();
			}
		}

		return task;
	}

	/*----------------------------------------------------------------------------
		CustomThread::ThreadMain
	----------------------------------------------------------------------------*/
	void CustomThread::ThreadMain()
	{
		if( m_MainFunc )
		{
			m_MainFunc( *this );
		}

		m_State = false;
	}

	/*----------------------------------------------------------------------------
		PoolWorkerThread::PoolWorkerThread
	----------------------------------------------------------------------------*/
	PoolWorkerThread::PoolWorkerThread()
		: m_State( false ), m_Handle( nullptr )
	{
	}

	/*----------------------------------------------------------------------------
		PoolWorkerThread::ThreadMain
	----------------------------------------------------------------------------*/
	void PoolWorkerThread::ThreadMain()
	{
		while( m_State )
		{
			// Attempt to pop a task from the list
			auto task = TaskPool::PopNextTask();
			if( task )
			{
				task->Execute();
			}
			else
			{
				// Since there were no tasks.. were going to wait until there is
				// If we dont get one within the next couple milliseconds, were going to stop waiting and iterate again
				TaskPool::WaitForNextTask();
			}
		}

		m_State = false;
	}

	/*----------------------------------------------------------------------------
		PoolWorkerThread::Start
	----------------------------------------------------------------------------*/
	bool PoolWorkerThread::Start()
	{
		// Check if the thread is already running
		if( m_State || m_Handle )
		{
			std::cout << "[ERROR] Threading: Attempt to start a pool worker thread thats already running!\n";
			return false;
		}

		m_State		= true;
		m_Handle	= std::make_unique< std::thread >( std::bind( &PoolWorkerThread::ThreadMain, this ) );

		return true;
	}

	/*----------------------------------------------------------------------------
		PoolWorkerThread::Stop
	----------------------------------------------------------------------------*/
	bool PoolWorkerThread::Stop()
	{
		m_State = false;

		if( m_Handle )
		{
			if( m_Handle->joinable() )
			{
				m_Handle->join();
				m_Handle.reset();
			}
			else
			{
				std::cout << "[ERROR] ThreadManager: Failed to stop worker thread.. wasnt joinable!\n";
				return false;
			}
		}

		return true;
	}

	/*----------------------------------------------------------------------------
		TaskPool::PopNextTask
	----------------------------------------------------------------------------*/
	std::unique_ptr< TaskInstanceBase > TaskPool::PopNextTask()
	{
		// First we need to gain a lock on the task pool members
		std::lock_guard< std::mutex > lock( m_TaskMutex );
		if( !m_TaskList.empty() )
		{
			auto output = std::move( m_TaskList.top() );
			m_TaskList.pop();

			// Pop any null tasks
			while( !output )
			{
				output = std::move( m_TaskList.top() );
				m_TaskList.pop();
			}

			return output;
		}

		return nullptr;
	}

	/*----------------------------------------------------------------------------
		TaskPool::WaitForNextTask
	----------------------------------------------------------------------------*/
	void TaskPool::WaitForNextTask()
	{
		// Wait for a task to be added, but time out after a couple milliseconds (incase of program shutdown)
		{
			std::unique_lock< std::mutex > lock( m_TaskMutex );
			m_TaskBool = false;
			m_TaskCV.wait_for( lock, std::chrono::milliseconds( 2 ), [ & ](){ return m_TaskBool; } );
		}
	}


	/*----------------------------------------------------------------------------
		ThreadManager::ThreadManager
	----------------------------------------------------------------------------*/
	ThreadManager::ThreadManager()
	{
		// We need to create the threads used for the task pool
		// This should give us the number of CPU threads minus one
		// If this function cant determine the number, we default to 3
		// TODO: Performance testing to determine the optimal number of threads for the pool
		auto tc = std::thread::hardware_concurrency();
		if( tc == 0 ) tc = 3; else tc--;

		std::cout << "Thread Manager: Initializing with " << tc << " worker threads in the pool\n";

		for( uint32 i = 0; i < tc; i++ )
		{
			// Create and start worker thread, then add to the list
			auto newWorker = std::make_shared< PoolWorkerThread >();
			newWorker->Start();

			m_WorkerPool.push_back( newWorker );
		}
	}

	/*----------------------------------------------------------------------------
		ThreadManager::CreateThread
	----------------------------------------------------------------------------*/
	std::shared_ptr< Thread > ThreadManager::CreateThread( const TickedThreadParameters& params )
	{
		// Validate the parameters
		if( params.Identifier.size() == 0 )
		{
			std::cout << "[ERROR] ThreadManager: Attempt to create a thread without a valid identifier\n";
			return nullptr;
		}
		else if( !params.TickFunction )
		{
			std::cout << "[ERROR] ThreadManager: Failed to create thread '" << params.Identifier << "' because the tick function was not specified\n";
			return nullptr;
		}
		else if( params.MinimumTasksPerTick > params.MaximumTasksPerTick && params.MaximumTasksPerTick != 0 )
		{
			std::cout << "[ERROR] ThreadManager: Failed to create thread '" << params.Identifier << "' because the minimum task count is greater than the maximum task count\n";
			return nullptr;
		}
		else if( GetThread( params.Identifier ) )
		{
			std::cout << "[ERROR] ThreadManager: Failed to create thread '" << params.Identifier << "' because a thread with that identifier already exists\n";
			return nullptr;
		}

		// Next, create the thread in-place
		auto& newThread = m_TickedThreads[ params.Identifier ] = std::shared_ptr< TickedThread >( new TickedThread( params ) );

		// And were going to run it automatically if desired
		if( params.StartAutomatically )
		{
			newThread->Start();
		}

		return newThread;
	}

	std::shared_ptr< Thread > ThreadManager::CreateThread( const CustomThreadParameters& params )
	{
		// Validate the parameters
		if( params.Identifier.size() == 0 )
		{
			std::cout << "[ERROR] ThreadManager: Attempt to create a thread without a valid identifier\n";
			return nullptr;
		}
		else if( !params.ThreadFunction )
		{
			std::cout << "[ERROR] ThreadManager: Failed to create thread '" << params.Identifier << "' because the tick function was not specified\n";
			return nullptr;
		}
		else if( GetThread( params.Identifier ) )
		{
			std::cout << "[ERROR] ThreadManager: Failed to create thread '" << params.Identifier << "' because a thread with that identifier already exists\n";
			return nullptr;
		}

		// Create thread in-place
		auto& newThread = m_CustomThreads[ params.Identifier ] = std::shared_ptr< CustomThread >( new CustomThread( params ) );

		if( params.StartAutomatically )
		{
			newThread->Start();
		}

		return newThread;
	}

	std::shared_ptr< Thread > ThreadManager::GetThread( const std::string& identifier )
	{
		if( identifier.size() == 0 )
			return nullptr;

		// Check both lists for the target 
		auto it = m_TickedThreads.find( identifier );
		if( it != m_TickedThreads.end() )
		{
			return it->second;
		}

		auto cit = m_CustomThreads.find( identifier );
		if( cit != m_CustomThreads.end() )
		{
			return cit->second;
		}

		return nullptr;
	}

	bool ThreadManager::DestroyThread( const std::string& identifier )
	{
		if( identifier.size() == 0 )
			return false;

		// Attempt to find an iterator to the thread
		auto it = m_TickedThreads.find( identifier );
		if( it != m_TickedThreads.end() )
		{
			if( it->second && it->second->IsRunning() )
			{
				it->second->Stop();
			}

			m_TickedThreads.erase( it );
			return true;
		}
		else
		{
			auto cit = m_CustomThreads.find( identifier );
			if( cit != m_CustomThreads.end() )
			{
				if( cit->second && cit->second->IsRunning() )
				{
					cit->second->Stop();
				}

				m_CustomThreads.erase( cit );
				return true;
			}
		}

		return false;
	}

	uint32 ThreadManager::GetThreadCount()
	{
		return static_cast< uint32 >( m_TickedThreads.size() + m_CustomThreads.size() );
	}


	void ThreadManager::StopThreads()
	{
		// Shutdown all open threads
		for( auto& pair : m_TickedThreads )
		{
			if( pair.second && pair.second->IsRunning() )
			{
				pair.second->Stop();
			}
		}

		for( auto& pair : m_CustomThreads )
		{
			if( pair.second && pair.second->IsRunning() )
			{
				pair.second->Stop();
			}
		}

		// Shutdown thread pool workers
		for( auto& worker : m_WorkerPool )
		{
			if( worker )
			{
				worker->Stop();
			}
		}

		// Clear the thread maps
		m_TickedThreads.clear();
		m_CustomThreads.clear();
		m_WorkerPool.clear();
	}

	void ThreadManager::Shutdown()
	{
		StopThreads();
	}

}