/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/TaskManager.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

// Hyperion Includes
#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Object.h"
#include "Hyperion/Core/Types/ConcurrentQueue.h"

#include <any>


namespace Hyperion
{

	template< typename _Ret >
	class ITask
	{

	public:

		virtual ~ITask() {}
		virtual _Ret Execute() = 0;
	};

	class ThreadPoolWorker
	{

	protected:

		std::unique_ptr< std::thread > m_Thread;
		std::atomic< bool > m_State;

		void threadBody();

	public:

		ThreadPoolWorker();
		ThreadPoolWorker( const ThreadPoolWorker& ) = delete;
		ThreadPoolWorker& operator=( const ThreadPoolWorker& ) = delete;

		bool Begin();
		bool End();

		std::thread::id GetSystemThreadId() const { return m_Thread ? m_Thread->get_id() : std::thread::id(); }

	};


	struct TaskState
	{
		#if HYPERION_OS_WIN32
		void* hCompleteEvent;
		#else
		std::mutex m;
		std::condition_variable cv;
		#endif

		std::atomic< bool > bComplete;
		std::atomic< bool > bExecuting;
		std::any val;

		TaskState();
		~TaskState();

	};


	struct PooledTaskHandleBase
	{

	protected:

		std::shared_ptr< TaskState > state;

	public:

		bool IsComplete();
		bool IsExecuting();
		void Wait();
		std::any GetGenericResult();
		std::any WaitForGenericResult();

		friend class TaskManager;

	};


	template< typename _Ret >
	struct PooledTaskHandle : public PooledTaskHandleBase
	{

	public:

		PooledTaskHandle() = delete;
		PooledTaskHandle( const std::shared_ptr< TaskState >& inState )
		{
			state = inState;
		}

		_Ret GetResult()
		{
			return std::any_cast< _Ret >( state->val );
		}


		_Ret WaitForResult()
		{
			Wait();
			return GetResult();
		}

	};

	template<>
	struct PooledTaskHandle< void > : public PooledTaskHandleBase
	{

	public:

		PooledTaskHandle() = delete;
		PooledTaskHandle( const std::shared_ptr< TaskState >& inState )
		{
			state = inState;
		}

	};


	class TaskManager
	{

	private:

		struct TaskEntry
		{
			std::function< std::any() > func;
			std::shared_ptr< TaskState > state;
		};

		static ConcurrentQueue< std::unique_ptr< TaskEntry > > m_WorkQueue;
		static std::vector< std::unique_ptr< ThreadPoolWorker > > m_Workers;
		static std::atomic< uint32 > m_WorkCount;
		static std::atomic< uint32 > m_IdleWorkers;

		#if HYPERION_OS_WIN32

		static void* m_hWorkEvent;
		static void* m_hShutdownEvent;

		#else

		static std::mutex m_WorkMutex;
		static std::condition_variable m_WorkCV;

		#endif

		static std::shared_ptr< TaskState > AddTaskInternal( const std::function< std::any() >& inFunc );

	public:

		static bool Initialize( uint32 inThreadCount );
		static void Shutdown();

		
		template< typename _Func >
		static PooledTaskHandle< typename std::invoke_result_t< _Func > > AddTask( _Func&& f )
		{
			using _Ret = typename std::invoke_result_t< _Func >;

			// Create a task instance
			auto taskState = AddTaskInternal( [ f ] { return std::any( f() ); } );
			return PooledTaskHandle< _Ret >( taskState );
		}


		template< typename _Ret >
		static PooledTaskHandle< _Ret > AddTask( const std::function< _Ret() >& inFunc )
		{
			// Ensure the function is valid
			if( !inFunc )
			{
				Console::WriteLine( "[ERROR] TaskManager: Failed to add task, function wasnt bound!" );

				// Create a 'dummy' task state
				auto state = std::make_shared< TaskState >();
				state->bComplete = true;
				state->bExecuting = false;

				return PooledTaskHandle< _Ret >( state );
			}

			// Create a task instance
			auto taskState = AddTaskInternal( [ inFunc ] { return std::any( inFunc() ); } );
			return PooledTaskHandle< _Ret >( taskState );
		}
		
		template< typename _Ret >
		static PooledTaskHandle< _Ret > AddTask( const HypPtr< ITask< _Ret > >& inTask )
		{
			// We need to hold a strong ref to this object until the task is finished completing
			// This way, the object wont go out of scope and be invalidated when the function is called
			if( !inTask.IsValid() )
			{
				Console::WriteLine( "[ERROR] TaskManager: Failed to add task, target object was invalid" );

				// Return a 'dummy' task state
				auto state = std::make_shared< TaskState >();
				state->bComplete = true;
				state->bExecuting = false;

				return PooledTaskHandle< _Ret >( state );
			}

			// We will include a copy of the pointer in the task payload
			auto taskState = AddTaskInternal(
				[inTask]
				{
					if( !inTask.IsValid() )
					{
						Console::WriteLine( "[ERROR] TaskManager: Failed to execute task, the task object was invalid" );
						return std::any();
					}
					else
					{
						return std::any( inTask->Execute() );
					}
				} 
			);

			return PooledTaskHandle< _Ret >( taskState );
		}

		static void AddUntrackedTask( const std::function< void() >& inFunc );
		
		template< typename _Ret >
		static void AddUntrackedTask( const HypPtr< ITask< _Ret > >& inTask )
		{
			if( !inTask.IsValid() )
			{
				Console::WriteLine( "[ERROR] TaskManager: Failed to add task, target object was invalid" );
				return;
			}

			AddUntrackedTask(
				[inTask]
				{
					if( !inTask.IsValid() )
					{
						Console::WriteLine( "[ERROR] TaskManager: Failed to execute task, the task object was invalid" );
					}
					else
					{
						inTask->Execute();
					}
				} 
			);
		}

		friend class ThreadPoolWorker;
	};

}