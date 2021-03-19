/*==================================================================================================
	Hyperion Engine
	Source/Core/TaskManager.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/TaskManager.h"

#if HYPERION_OS_WIN32
#include "Hyperion/Win32/Win32Headers.h"
#endif


namespace Hyperion
{

	/*
	*	Constants
	*/
	constexpr uint32 TASK_MANAGER_MAX_SLEEP_DURATION		= 10;
	constexpr bool TASK_MANAGER_INFINITE_SLEEP_MODE			= true;

	/*
	*	Static Definitions
	*/
	ConcurrentQueue< std::unique_ptr< TaskManager::TaskEntry > > TaskManager::m_WorkQueue {};
	std::vector< std::unique_ptr< ThreadPoolWorker > > TaskManager::m_Workers {};
	std::atomic< uint32 > TaskManager::m_WorkCount( 0 );
	std::atomic< uint32 > TaskManager::m_IdleWorkers( 0 );

	#if HYPERION_OS_WIN32
	void* TaskManager::m_hWorkEvent( nullptr );
	void* TaskManager::m_hShutdownEvent( nullptr );
	#else
	std::mutex TaskManager::m_WorkMutex {};
	std::condition_variable TaskManager::m_WorkCV {};
	#endif


	bool PooledTaskHandleBase::IsComplete()
	{
		HYPERION_VERIFY( state != nullptr, "[TaskPook] Task state was null!" );
		return state->bComplete.load();
	}


	bool PooledTaskHandleBase::IsExecuting()
	{
		HYPERION_VERIFY( state != nullptr, "[TaskPool] Task state was null!" );
		return state->bExecuting.load();
	}


	void PooledTaskHandleBase::Wait()
	{
		HYPERION_VERIFY( state != nullptr, "[TaskPool] Task state was null!" );

		if( state->bComplete.load() == false )
		{
			#if HYPERION_OS_WIN32
			WaitForSingleObject( state->hCompleteEvent, INFINITE );
			#else
			{
				std::unique_lock< std::mutex > lck( state->m );
				state->cv.wait( lck, [] { return state->bComplete; } );
			}
			#endif
		}
	}


	std::any PooledTaskHandleBase::GetGenericResult()
	{
		HYPERION_VERIFY( state != nullptr, "[TaskPool] Task state was null!" );
		return state->val;
	}


	std::any PooledTaskHandleBase::WaitForGenericResult()
	{
		Wait();
		return state->val;
	}


	TaskState::TaskState()
		: bComplete( false ), bExecuting( false )
	{
		#if HYPERION_OS_WIN32
		hCompleteEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
		#endif
	}

	TaskState::~TaskState()
	{
		#if HYPERION_OS_WIN32
		if( hCompleteEvent != nullptr )
		{
			CloseHandle( hCompleteEvent );
		}
		#endif
	}

	/*--------------------------------------------------------------------------------------
		TaskManager
	--------------------------------------------------------------------------------------*/
	bool TaskManager::Initialize( uint32 inThreadCount )
	{
		HYPERION_VERIFY( inThreadCount > 1, "[TaskManager] Invalid thread count for task manager!" );


		// Depending on the platform, we might have to setup our syncronization primitive used to notify threads of new work
		#if HYPERION_OS_WIN32
		m_hWorkEvent		= CreateEvent( NULL, FALSE, FALSE, NULL );
		m_hShutdownEvent	= CreateEvent( NULL, TRUE, FALSE, NULL );
		#endif

		// Setup our threads
		m_Workers.resize( inThreadCount );

		for( auto it = m_Workers.begin(); it != m_Workers.end(); it++ )
		{
			*it = std::make_unique< ThreadPoolWorker >();
			bool bResult = ( *it )->Begin();
			HYPERION_VERIFY( bResult == true, "[TaskManager] Failed to create worker thread" );
		}


		return true;
	}


	void TaskManager::Shutdown()
	{
		// We need to trigger the shutdown event, so all threads break out of the idle phase 
		// Outside of windows, we would have to trigger the work CV, to unlock all threads
		#if HYPERION_OS_WIN32
		SetEvent( m_hShutdownEvent );
		#else
		m_WorkCV.notify_all();
		#endif

		for( auto it = m_Workers.begin(); it != m_Workers.end(); it++ )
		{
			auto bResult = ( *it )->End();
			HYPERION_VERIFY( bResult == true, "[TaskManager] Failed to shutdown worker thread" );
		}

		m_Workers.clear();

		#if HYPERION_OS_WIN32
		CloseHandle( m_hShutdownEvent );
		CloseHandle( m_hWorkEvent );
		#endif
	}


	std::shared_ptr<TaskState> TaskManager::AddTaskInternal( const std::function<std::any()>& inFunc )
	{
		HYPERION_VERIFY( inFunc, "[TaskManager] Function was not bound" );

		//auto beg = std::chrono::high_resolution_clock::now();
		// Lets create a task state for this task
		auto newState = std::make_shared< TaskState >();

		// Now, insert new work into the pool, attempt to perform a lock-free insert first
		auto newEntry	= std::make_unique< TaskEntry >();
		newEntry->func	= inFunc;
		newEntry->state = newState;

		m_WorkQueue.Push( std::move( newEntry ) );

		// Increment our work counter
		// We might not really need this counter, its not used for any logic
		m_WorkCount++;

		// Now, check for idle threads, if there are any, then we will release one of them to process this task
		if( m_IdleWorkers.load() > 0 )
		{
			#if HYPERION_OS_WIN32
			SetEvent( m_hWorkEvent );
			#else
			m_WorkCV.notify_one();
			#endif
		}

		return newState;
	}


	void TaskManager::AddUntrackedTask( const std::function< void() >& inFunc )
	{
		HYPERION_VERIFY( inFunc, "[TaskPool] Function was not bound" );

		// Create a work entry, without a state, wrap the function so it fits the proper format, Push the work to the queue, hopefully without locking
		auto newEntry	= std::make_unique< TaskEntry >();
		newEntry->func	= [ inFunc ] { inFunc(); return std::any(); };
		newEntry->state = std::shared_ptr< TaskState >( nullptr );

		m_WorkQueue.Push( std::move( newEntry ) );

		// Incremement work counter
		m_WorkCount++;

		// Check for idle threads, and release one if needed
		if( m_IdleWorkers.load() > 0 )
		{
			#if HYPERION_OS_WIN32
			SetEvent( m_hWorkEvent );
			#else
			m_WorkCV.notify_one();
			#endif
		}
	}


	ThreadPoolWorker::ThreadPoolWorker()
		: m_State( false ), m_Thread( nullptr )
	{

	}


	bool ThreadPoolWorker::Begin()
	{
		if( m_Thread != nullptr ) { return false; }

		m_State.store( true );
		m_Thread = std::make_unique< std::thread >( &ThreadPoolWorker::threadBody, this );

		return true;
	}


	bool ThreadPoolWorker::End()
	{
		if( m_Thread == nullptr || m_State.load() == false ) { return false; }

		m_State.store( false );
		if( m_Thread->joinable() )
		{
			m_Thread->join();
		}
		else
		{
			return false;
		}

		m_Thread.reset();
		return true;
	}


	void ThreadPoolWorker::threadBody()
	{
		// So, the idea is going to be, once the state is false, we exit as soon as we run out of tasks
		while( true )
		{
			// Pop tasks until there are none left
			std::pair< bool, std::unique_ptr< TaskManager::TaskEntry > > nextTask =
				TaskManager::m_WorkQueue.PopValue();

			//while( nextTask != nullptr && nextTask->first )
			while( nextTask.first && nextTask.second != nullptr )
			{
				// Decrease the atomic task counter
				TaskManager::m_WorkCount--;

				// Execute the task and get the result
				bool bHasState = nextTask.second->state ? true : false;
				if( bHasState ) { nextTask.second->state->bExecuting.store( true ); }

				if( bHasState )
				{
					nextTask.second->state->bComplete.store( true );
					nextTask.second->state->bExecuting.store( false );
					nextTask.second->state->val = nextTask.second->func();

					// Trigger event, so any threads waiting will continue
					#if HYPERION_OS_WIN32
					SetEvent( (HANDLE) nextTask.second->state->hCompleteEvent );
					#else
					nextTask->state->cv.notify_all();
					#endif
				}
				else
				{
					nextTask.second->func();
				}
				
				// Pop the next task
				nextTask = TaskManager::m_WorkQueue.PopValue();
			}

			// There are no more tasks, so we can exit if the thread needs to shut down
			// Otherwise, we will continue to spin lock, until we get another task or hit spin lock limit
			if( m_State.load() == false )
			{
				break;
			}
			

			// We cant spin lock anymore, so we need to enter an idle state, so other tasks can use this thread
			TaskManager::m_IdleWorkers++;
			#if HYPERION_OS_WIN32
			HANDLE list[] = { TaskManager::m_hShutdownEvent, TaskManager::m_hWorkEvent };
			WaitForMultipleObjects( 2, list, FALSE, TASK_MANAGER_INFINITE_SLEEP_MODE ? INFINITE : (DWORD) TASK_MANAGER_MAX_SLEEP_DURATION );
			#else
			{
				std::unique_lock< std::mutex > lck( TaskManager::m_WorkMutex );
				TaskManager::m_WorkCV.wait_for( lck, [] { return TaskManager::m_WorkCount.load() > 0; }, std::chrono::milliseconds( TASK_MANAGER_MAX_SLEEP_DURATION ) );
			}
			#endif
			TaskManager::m_IdleWorkers--;
		}

		// The thread is closing...
	}
}