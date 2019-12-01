/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Threading.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Memory.h"

#include <memory>
#include <thread>
#include <atomic>
#include <functional>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stack>
#include <map>


namespace Hyperion
{

	class Thread
	{

	protected:

		std::unique_ptr< std::thread > m_Thread;
		std::string m_Name;
		std::atomic< bool > m_State;
		bool m_bAllowTaskInjection;

	public:

		bool GetState() const { return m_State; }
		bool IsRunning() const { return IsValid() && GetState(); }
		std::string GetName() const { return m_Name; }
		bool IsValid() const { return m_Thread ? true : false; }
		std::thread::id GetSystemId() const { return m_Thread ? m_Thread->get_id() : std::thread::id(); }
		bool CanInjectTasks() const { return m_bAllowTaskInjection; }

		virtual bool Start() = 0;
		virtual bool Stop() = 0;

	};

	class TickedThread : public Thread
	{

	protected:

		std::function< void() > m_InitFunc;
		std::function< void() > m_ShutdownFunc;
		std::function< void( TickedThread& ) > m_MainFunc;

		void ThreadMain();

		std::chrono::time_point< std::chrono::high_resolution_clock > m_LastTick;
		std::chrono::milliseconds m_Frequency; // How often should we try and tick? In milliseconds
		std::chrono::milliseconds m_TickDeviation; // If the tick function runs.. and were 'X' ms away from the next tick, just start the next tick. This is checked after the main tick function runs, and after each async task is ran
		
		uint32 m_MaxAsyncTasksPerTick; // 0 = unlimited (until we hit next targetted tick, or all queued tasks if m_Frequency is also 0)

	public:

		bool Start() override;
		bool Stop() override;

	};

	class StandardThread : public Thread
	{

	protected:

		std::function< void( StandardThread& ) > m_MainFunc;

		void ThreadMain();

	public:

		bool Start() override;
		bool Stop() override;

	};

	enum class TaskState
	{
		NotStarted,
		Running,
		Complete,
		Error
	};

	template< typename T >
	struct TaskSharedState
	{

		T Result;
		std::atomic< TaskState > State;
		std::mutex Mutex;
		std::condition_variable CV;

	};

	enum class TaskPriority
	{
		VeryLow,
		Low,
		Normal,
		High,
		VeryHigh
	};

	struct TaskInstanceBase
	{
		
		virtual void Execute() = 0;
		std::chrono::time_point< std::chrono::high_resolution_clock > ScheduledTime;

	};

	template< typename T >
	struct TaskInstance : public TaskInstanceBase
	{

		std::function< T() > Func;
		std::shared_ptr< TaskSharedState< T > > SharedState;
		TaskPriority Priority;

		TaskInstance() = delete;

		TaskInstance( const std::function< T() >& inFunc, TaskPriority inPriority )
			: Func( inFunc ), Priority( inPriority ), SharedState( std::make_shared< TaskSharedState< T > >() )
		{
			SharedState->State = TaskState::NotStarted;
		}

		void Execute() override
		{
			if( Func && SharedState && SharedState->State == TaskState::NotStarted )
			{
				// Run the task and get the result.. store in the handle
				SharedState->State		= TaskState::Running;
				SharedState->Result		= Func();
				SharedState->State		= TaskState::Complete;

				// Notify cv incase anyone is waiting on this task
				SharedState->CV.notify_all();
			}
			else
			{
				// Set state to indicate error and notify cv incase anyone is waiting
				SharedState->State = TaskState::Error;
				SharedState->CV.notify_all();
			}
		}

	};

	struct TaskHandleBase
	{

		virtual void Wait() const = 0;
		virtual TaskState GetState() const = 0;
		virtual bool IsValid() const = 0;

		bool IsComplete() const
		{
			auto state = GetState();
			return IsValid() && ( state == TaskState::Complete || state == TaskState::Error );
		}
	};

	template< typename T >
	struct TaskHandle
	{

	protected:

		std::shared_ptr< TaskSharedState< T > > SharedState;

	public:

		TaskHandle()
			: TaskHandle( nullptr )
		{}

		TaskHandle( nullptr_t )
			: SharedState( nullptr )
		{}

		TaskHandle( const TaskInstance< T >& parentInstance )
			: SharedState( parentInstance->SharedState )
		{
			if( !SharedState )
			{
				std::cout << "[WARNING] TaskHandle: Attempt to create a task handle pointing to a null task instance!\n";
			}
		}

		void Wait() const override
		{
			if( SharedState )
			{
				std::unique_lock< std::mutex > lock( SharedState->Mutex );
				while( SharedState->State != TaskState::Complete )
				{
					SharedState->CV.wait( lock );
				}
			}
		}

		TaskState GetState() const override
		{
			if( SharedState )
			{
				return SharedState->State;
			}

			return TaskState::Error;
		}

		bool IsValid() const override
		{
			return SharedState ? true : false;
		}

		Nullable< T& > GetResult( bool bShouldWait = false )
		{
			// If this handle isnt valid, return null
			if( !SharedState )
			{
				return nullptr;
			}

			auto state = SharedState->State;
			if( state == TaskState::Complete )
			{
				return SharedState->Result;
			}
			else if( state == TaskState::Error )
			{
				return nullptr;
			}
			else
			{
				// Task isnt complete..
				if( bShouldWait )
				{
					// Wait for the task to complete.. and then return the result
					Wait();

					auto new_state = SharedState->State;
					if( new_state == TaskState::Complete )
					{
						return SharedState->Result;
					}
					else
					{
						// If for some reason the state isnt complete (i.e. error) then return null
						return nullptr;
					}
				}
				else
				{
					// If the user doesnt want to wait.. then lets just return null
					return nullptr;
				}
			}
		}

		T& GetResultRaw()
		{
			return SharedState->Result;
		}

		friend class TaskManager;
	};

	class ThreadManager
	{

	};


	class TaskManager
	{

	private:

		static void ScheduleNextTask();

	protected:

		static std::mutex m_TaskMutex;

		static std::stack< std::unique_ptr< TaskInstanceBase > > m_VeryLowPriorityTasks;
		static std::stack< std::unique_ptr< TaskInstanceBase > > m_LowPriorityTasks;
		static std::stack< std::unique_ptr< TaskInstanceBase > > m_NormalPriorityTasks;
		static std::stack< std::unique_ptr< TaskInstanceBase > > m_HighPriorityTasks;
		static std::stack< std::unique_ptr< TaskInstanceBase > > m_VeryHighPriorityTasks;

		static std::unique_ptr< TaskInstanceBase > m_NextTask;

		static std::unique_ptr< TaskInstanceBase > PopNextTask();
		static void AddTask( std::unique_ptr< TaskInstanceBase >&& inTask, TaskPriority inPriority );

	public:

		template< typename T >
		static TaskHandle< T > CreateTask( std::function< T( void ) > inFunc, TaskPriority inPriority = TaskPriority::Normal, std::string inTargetThread = "pool" )
		{
			// Validate input function
			if( !inFunc )
			{
				std::cout << "[ERROR] TaskManager: Failed to create task.. null function provided!\n";
				return TaskHandle< T >( nullptr );
			}

			// Create a new task instance
			auto inst = std::make_unique< TaskInstance< T > >( inFunc, inPriority );

			// Then, create a new task handle pointing to this instance to return to the caller
			TaskHandle< T > handle( inst );

			// Task can be inserted into the queue right away
			AddTask( std::move( inst, inPriority ) );

			return handle;
		}

		static void WaitAll( std::initializer_list< TaskHandleBase > Tasks )
		{
			for( auto& t : Tasks )
			{
				if( t.IsValid() && !t.IsComplete() )
				{
					t.Wait();
				}
			}
		}

		static void WaitAny( std::initializer_list< TaskHandleBase > Tasks )
		{
			// How do we do this?
			// Well... if any are already complete then we can skip the whole thing
			for( auto& t : Tasks )
			{
				if( t.IsComplete() )
					return;
			}

			// How can we just wait for a single one of these to complete?
			// It probably requires digging more into the stl threading library, and maybe accessing the underlying cv or mutex
			// because maybe we can perform a wait based on an entire set of mutex's
			// TODO TODO TODO TODO TODO TODO TODO
		}

		



	};


}