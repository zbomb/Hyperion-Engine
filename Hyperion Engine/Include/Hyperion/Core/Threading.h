/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Threading.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

// Hyperion Includes
#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Memory.h"
#include "Hyperion/Core/Object.h"

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

	enum class TaskState
	{
		NotStarted,
		Running,
		Complete,
		Error
	};

	struct TaskWaitToken
	{
		std::mutex m;
		std::condition_variable cv;
		bool b;
	};

	struct TickedThreadParameters
	{
		std::string Identifier;
		std::function< void() > InitFunction;
		std::function< void() > TickFunction;
		std::function< void() > ShutdownFunction;
		float Frequency;
		float Deviation;
		bool AllowTasks;
		uint32 MinimumTasksPerTick;
		uint32 MaximumTasksPerTick;
		bool StartAutomatically;
	};


	struct CustomThreadParameters
	{
		std::string Identifier;
		std::function< void( class CustomThread& ) > ThreadFunction;
		bool AllowTasks;
		bool StartAutomatically;
	};


	struct TaskInstanceBase
	{
		virtual void Execute() = 0;
	};


	struct TaskHandleBase
	{

	protected:

		virtual bool AddWaitToken( const std::shared_ptr< TaskWaitToken >& ) const = 0;

	public:

		virtual void Wait() const = 0;
		virtual TaskState GetState() const = 0;
		virtual bool IsValid() const = 0;
		virtual bool IsComplete() const = 0;
		virtual bool HasResult() const = 0;

		friend class Task;
	};


	class Thread
	{

	protected:

		std::unique_ptr< std::thread > m_Handle;
		std::mutex m_TaskMutex;
		std::stack< std::unique_ptr< TaskInstanceBase > > m_TaskList;
		std::atomic< bool > m_State;
		std::string m_Identifier;
		bool m_AllowTasks;

		Thread( const std::string& inIdentifier, bool inAllowTasks )
			: m_Identifier( inIdentifier ), m_AllowTasks( inAllowTasks ), m_State( false )
		{}

		Thread() = delete;

	public:

		virtual bool Start() = 0;
		virtual bool Stop() = 0;
		virtual bool InjectTask( std::unique_ptr< TaskInstanceBase >&& ) = 0;

		std::string GetIdentifier() const { return m_Identifier; }
		bool IsRunning() const { return m_State; }
		bool CanInjectTask() const { return m_AllowTasks; }

		std::thread::id GetSystemIdentifier() const
		{
			if( m_Handle )
			{
				return m_Handle->get_id();
			}

			return std::thread::id();
		}
	};


	class TickedThread : public Thread
	{

	protected:

		std::function< void() > m_Init;
		std::function< void() > m_Main;
		std::function< void() > m_Shutdown;

		std::chrono::duration< float, std::milli > m_Frequency;
		std::chrono::duration< float, std::milli > m_Deviation;
		uint32 m_MinTasks;
		uint32 m_MaxTasks;

		TickedThread( const TickedThreadParameters& params );
		TickedThread() = delete;

		void ThreadMain();
		bool RunNextTask();

	public:

		bool Start() override;
		bool Stop() override;
		bool InjectTask( std::unique_ptr< TaskInstanceBase >&& in ) override;

		friend class ThreadManager;
	};


	class CustomThread : public Thread
	{

	protected:

		std::function< void( CustomThread& ) > m_MainFunc;

		CustomThread( const CustomThreadParameters& params );
		CustomThread() = delete;

		void ThreadMain();

	public:

		bool Start() override;
		bool Stop() override;
		bool InjectTask( std::unique_ptr< TaskInstanceBase >&& in ) override;

		std::unique_ptr< TaskInstanceBase > PopTask();

		friend class ThreadManager;
	};


	class PoolWorkerThread
	{

	protected:

		std::unique_ptr< std::thread > m_Handle;
		std::atomic< bool > m_State;

		void ThreadMain();

	public:

		PoolWorkerThread();
		PoolWorkerThread( const PoolWorkerThread& ) = delete;
		PoolWorkerThread& operator=( const PoolWorkerThread& ) = delete;

		bool Start();
		bool Stop();

	};


	template< typename T >
	struct TaskSharedState
	{
		T result;
		std::mutex mutex;
		std::vector< std::shared_ptr< TaskWaitToken > > wait_tokens;
		TaskState state;
	};

	template<>
	struct TaskSharedState< void >
	{
		std::mutex mutex;
		std::vector< std::shared_ptr< TaskWaitToken > > wait_tokens;
		TaskState state;
	};


	template< typename T >
	struct TaskInstance : public TaskInstanceBase
	{

		std::function< T() > func;
		std::shared_ptr< TaskSharedState< T > > state;

		TaskInstance() = delete;

		TaskInstance( const std::function< T() >& inFunc )
			: func( inFunc ), state( std::make_shared< TaskSharedState< T > >() )
		{
			state->state = TaskState::NotStarted;
		}

		void Execute() override
		{
			// First, lets check if were in a proper state to execute this task
			if( state )
			{
				// Lock access to the state object so we can run the task
				{
					std::lock_guard< boost::fibers::mutex > state_lock( state->mutex );
					if( state->state == TaskState::Error || state->state == TaskState::Complete )
					{
						// Already complete!
						return;
					}

					// Run the function.. if its not bound then set the result as error
					if( !func )
					{
						state->state = TaskState::Error;
					}
					else
					{
						state->result = func();
						state->state = TaskState::Complete;
					}

					// Now that the task is complete we can signal all wait tokens
					for( auto& token : state->wait_tokens )
					{
						if( token )
						{
							// Lock and signal
							{
								std::lock_guard< boost::fibers::mutex > token_lock( token->m );
								token->b = true;
							}

							token->cv.notify_all();
						}
					}

					// Clear tokens
					state->wait_tokens.clear();
				}
			}
		}

	};

	template<>
	struct TaskInstance< void > : public TaskInstanceBase
	{
		std::function< void() > func;
		std::shared_ptr< TaskSharedState< void > > state;

		TaskInstance() = delete;

		TaskInstance( const std::function< void() >& inFunc )
			: func( inFunc ), state( std::make_shared< TaskSharedState< void > >() )
		{
			state->state = TaskState::NotStarted;
		}

		void Execute() override
		{
			std::cout << "[DEBUG] Executing task instance...\n";

			// First, lets check if were in a proper state to execute this task
			if( state )
			{
				// Lock access to the state object so we can run the task
				{
					std::lock_guard< std::mutex > state_lock( state->mutex );
					if( state->state == TaskState::Error || state->state == TaskState::Complete )
					{
						// Already complete!
						return;
					}

					// Run the function.. if its not bound then set the result as error
					if( !func )
					{
						state->state = TaskState::Error;
					}
					else
					{
						func();
						state->state = TaskState::Complete;
					}

					// Now that the task is complete we can signal all wait tokens
					for( auto& token : state->wait_tokens )
					{
						if( token )
						{
							// Lock and signal
							{
								std::lock_guard< std::mutex > token_lock( token->m );
								token->b = true;
							}

							token->cv.notify_all();
						}
					}

					// Clear tokens
					state->wait_tokens.clear();
				}
			}
		}
	};

	template< typename T >
	struct TaskHandle
	{

	protected:

		std::shared_ptr< TaskSharedState< T > > state;

		bool AddWaitToken( const std::shared_ptr< TaskWaitToken >& inToken ) const override
		{
			if( state && inToken )
			{
				// Aquire lock on task state so we can insert the token
				std::lock_guard< std::mutex > state_lock( state->mutex );

				auto& task_state = state->state;
				if( task_state == TaskState::NotStarted || task_state == TaskState::Running )
				{
					state->wait_tokens.push_back( inToken );
					return true;
				}
			}

			// We return false if the task is comnplete, in an invalid state, or if the provided token was null
			// We only return true if the token was actually inserted
			return false;
		}

	public:

		TaskHandle()
			: TaskHandle( nullptr )
		{}

		TaskHandle( nullptr_t )
			: state( nullptr )
		{}

		TaskHandle( const TaskInstance< T >& parentInstance )
			: state( parentInstance.state )
		{
			if( !state )
			{
				std::cout << "[WARNING] TaskHandle: Attempt to create a task handle pointing to a null task instance!\n";
			}
		}

		void Wait() const override
		{
			if( state )
			{
				std::shared_ptr< TaskWaitToken > wait_token = nullptr;

				{
					// First, we need to aquire a lock on the state, so we can check if its finished
					std::lock_guard< std::mutex > state_lock( state->mutex );

					auto& task_state = state->state;
					if( task_state == TaskState::NotStarted || task_state == TaskState::Running )
					{
						// Create and insert a wait token
						wait_token = std::make_shared< TaskWaitToken >();
						state->wait_tokens.push_back( wait_token );
					}
				}

				// Now, we just need to perform a wait on the token we created
				if( wait_token )
				{
					std::unique_lock< std::mutex > token_lock( wait_token->m );
					wait_token->cv.wait( token_lock, [ wait_token ]{ return wait_token->b; } );
				}
			}
		}

		TaskState GetState() const override
		{
			TaskState output = TaskState::Error;

			if( state )
			{
				// Aquire lock on the task state so we can read it
				std::lock_guard< std::mutex > state_lock( state->mutex );
				output = state->state;
			}

			return output;
		}

		bool IsValid() const override
		{
			return state ? true : false;
		}

		Nullable< const T& > GetResult( bool bShouldWait = false )
		{
			if( state )
			{
				std::shared_ptr< TaskWaitToken > wait_token = nullptr;

				{
					// Aquire a lock on the task state so we can access its members
					std::lock_guard< std::mutex > state_lock( state->mutex );

					// Check if task is complete
					auto& task_state = state->state;

					if( task_state == TaskState::Complete )
					{
						return Nullable< T& >( std::ref( state->result ) );
					}
					else if( bShouldWait && task_state != TaskState::Error )
					{
						// Insert a wait token so we can know when the task is complete
						wait_token = std::make_shared< TaskWaitToken >();
						state->wait_tokens.push_back( wait_token );
					}
				}

				// If we inserted a wait token, then wait for the result to come back
				if( wait_token )
				{
					{
						std::unique_lock< std::mutex > wait_lock( wait_token->m );
						wait_token->cv.wait( wait_lock, []{ return wait_token->b; } );
					}

					// Now access the result from the shared state
					{
						std::lock_guard< std::mutex > state_lock( state->mutex );
						auto& task_state = state->state;

						if( task_state == TaskState::Complete )
						{
							return Nullable< T& >( std::ref( state->result ) );
						}
						else
						{
							return nullptr;
						}
					}
				}
			}

			// If we didnt want to wait for a result or the state was null (or result error), then return null
			return nullptr;
		}

		const T& GetResultRaw()
		{
			std::lock_guard< std::mutex > state_lock( state->mutex );
			return state->result;
		}

		bool IsComplete() const override
		{
			auto task_state = GetState();
			return task_state == TaskState::Complete || task_state == TaskState::Error;
		}

		bool HasResult() const override
		{
			return GetState() == TaskState::Complete;
		}
	};



	template<>
	struct TaskHandle< void > : public TaskHandleBase
	{
	protected:

		std::shared_ptr< TaskSharedState< void > > state;

		bool AddWaitToken( const std::shared_ptr< TaskWaitToken >& inToken ) const override
		{
			if( state && inToken )
			{
				// Aquire lock on task state so we can insert the token
				std::lock_guard< std::mutex > state_lock( state->mutex );

				auto& task_state = state->state;
				if( task_state == TaskState::NotStarted || task_state == TaskState::Running )
				{
					state->wait_tokens.push_back( inToken );
					return true;
				}
			}

			// We return false if the task is comnplete, in an invalid state, or if the provided token was null
			// We only return true if the token was actually inserted
			return false;
		}

	public:

		TaskHandle()
			: TaskHandle( nullptr )
		{}

		TaskHandle( nullptr_t )
			: state( nullptr )
		{}

		TaskHandle( const TaskInstance< void >& parentInstance )
			: state( parentInstance.state )
		{
			if( !state )
			{
				std::cout << "[WARNING] TaskHandle: Attempt to create a task handle pointing to a null task instance!\n";
			}
		}

		void Wait() const override
		{
			if( state )
			{
				std::shared_ptr< TaskWaitToken > wait_token = nullptr;

				{
					// First, we need to aquire a lock on the state, so we can check if its finished
					std::lock_guard< std::mutex > state_lock( state->mutex );

					auto& task_state = state->state;
					if( task_state == TaskState::NotStarted || task_state == TaskState::Running )
					{
						// Create and insert a wait token
						wait_token = std::make_shared< TaskWaitToken >();
						state->wait_tokens.push_back( wait_token );
					}
				}

				// Now, we just need to perform a wait on the token we created
				if( wait_token )
				{
					std::unique_lock< std::mutex > token_lock( wait_token->m );
					wait_token->cv.wait( token_lock, [ wait_token ]{ return wait_token->b; } );
				}
			}
		}

		TaskState GetState() const override
		{
			TaskState output = TaskState::Error;

			if( state )
			{
				// Aquire lock on the task state so we can read it
				std::lock_guard< std::mutex > state_lock( state->mutex );
				output = state->state;
			}

			return output;
		}

		bool IsValid() const override
		{
			return state ? true : false;
		}

		void GetResult( bool bShouldWait = false )
		{
			if( bShouldWait )
			{
				Wait();
			}

			return;
		}

		// In this specialization.. this doesnt do anything
		const void GetResultRaw()
		{
			return;
		}

		bool IsComplete() const override
		{
			auto task_state = GetState();
			return task_state == TaskState::Complete || task_state == TaskState::Error;
		}

		bool HasResult() const override
		{
			return GetState() == TaskState::Complete;
		}
	};




	class ThreadManager : public Object
	{

	protected:

		std::mutex m_TaskMutex;
		std::stack< std::unique_ptr< TaskInstanceBase > > m_TaskList;

		std::map< std::string, std::shared_ptr< TickedThread > > m_TickedThreads;
		std::map< std::string, std::shared_ptr< CustomThread > > m_CustomThreads;

		std::unique_ptr< TaskInstanceBase > PopNextTask();

	public:

		ThreadManager();

		template< typename T >
		TaskHandle< T > CreateTask( std::function< T( void ) > inFunc, std::string inTargetThread = "pool" )
		{
			// Validate input function
			if( !inFunc )
			{
				std::cout << "[ERROR] TaskManager: Failed to create task.. null function provided!\n";
				return TaskHandle< T >( nullptr );
			}

			// Create a new task instance
			auto inst = std::make_unique< TaskInstance< T > >( inFunc );

			// Then, create a new task handle pointing to this instance to return to the caller
			TaskHandle< T > handle( *inst );

			if( inTargetThread == THREAD_POOL )
			{
				// Insert into the queue
				m_TaskList.push( std::move( inst ) );
			}
			else
			{
				// Find the thread, and inject the task
				auto thread = GetThread( inTargetThread );
				if( !thread )
				{
					std::cout << "[ERROR] TaskManager: Failed to create task.. invalid target thread '" << inTargetThread << "'\n";
					return TaskHandle< T >( nullptr );
				}

				if( !thread->InjectTask( std::move( inst ) ) )
				{
					std::cout << "[ERROR] Taskmanager: Failed to create task.. thread '" << inTargetThread << "' did not accept the task\n";
					return TaskHandle< T >( nullptr );
				}
			}

			return handle;
		}
		
		std::shared_ptr< Thread > CreateThread( const TickedThreadParameters& );
		std::shared_ptr< Thread > CreateThread( const CustomThreadParameters& );
		std::shared_ptr< Thread > GetThread( const std::string& );
		bool DestroyThread( const std::string& );
		uint32 GetThreadCount();

		void Shutdown();
	};



	class Task
	{

	public:

		template< typename T >
		static TaskHandle< T > Create( std::function< T( void ) > inFunc, std::string inTargetThread = "pool" )
		{
			auto tm = Engine::GetInstance().GetThreadManager();
			if( tm )
			{
				return tm->CreateTask( inFunc, inTargetThread );
			}
			else
			{
				std::cout << "[ERROR] TaskLibrary: Failed to create new task because the thread manager was null!\n";
				return nullptr;
			}
		}

		static void WaitAll( std::initializer_list< TaskHandleBase > Tasks )
		{
			for( auto& t : Tasks )
			{
				if( !t.IsComplete() )
					t.Wait();
			}
		}

		static void WaitAny( std::initializer_list< TaskHandleBase > Tasks )
		{
			if( Tasks.size() > 0 )
			{
				// Create a wait token, then add it to each shared task states token list
				// This way, which ever task finishes first will trigger the token
				auto wait_token		= std::make_shared< TaskWaitToken >();
				bool bWait			= false;

				for( auto& task : Tasks )
				{
					if( task.AddWaitToken( wait_token ) )
					{
						bWait = true;
					}
				}

				// Check if we actually added any wait tokens before waiting
				if( bWait )
				{
					// Wait for a task to trigger our cv lock
					std::unique_lock< std::mutex > token_lock( wait_token->m );
					wait_token->cv.wait( token_lock, [ wait_token ]{ return wait_token->b; } );
				}
			}
		}

		



	};


}