/*==================================================================================================
	Hyperion Engine
	Source/Core/Threading.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/Threading.h"

#include <chrono>


namespace Hyperion
{

	/*----------------------------------------------------------------------------
		TickedThread::Start
	----------------------------------------------------------------------------*/
	bool TickedThread::Start()
	{
		// First, check if the thread is already running, and if a main func is set
		if( m_State || m_Thread )
		{
			std::cout << "[ERROR] TickedThread: Attempt to start thread that is still running!\n";
			return false;
		}
		else if( !m_MainFunc )
		{
			std::cout << "[ERROR] TickedThread: Attempt to start a thread that doesnt have its main function set!\n";
			return false;
		}

		// Set state and create thread
		m_State = true;
		m_Thread = std::make_unique< std::thread >( std::bind( &TickedThread::ThreadMain, this ) );

		return true;
	}

	/*----------------------------------------------------------------------------
		TickedThread::Stop
	----------------------------------------------------------------------------*/
	bool TickedThread::Stop()
	{
		// Once this state is set to false, the thread should stop at the start of the next iteration before calling the main func
		m_State = false;

		if( m_Thread )
		{
			// Check if the active thread is joinable, meaning we can properly shut it down
			if( m_Thread->joinable() )
			{
				// The join call is going to block this thread until the thread were shutting down completes
				m_Thread->join();
				m_Thread.reset();
			}
			else
			{
				std::cout << "[ERROR] TickedThread: Failed to properly stop thread '" << m_Name << "' because it wasnt joinable!\n";
				return false;
			}
		}

		return true;
	}

	/*----------------------------------------------------------------------------
		TickedThread::ThreadMain
	----------------------------------------------------------------------------*/
	void TickedThread::ThreadMain()
	{
		// Run the set init func
		if( m_InitFunc )
		{
			m_InitFunc();
		}

		// Continue to tick as long as state is good, and the main func is not null
		while( m_MainFunc && m_State )
		{
			// First, store the current time
			auto tick_start = std::chrono::high_resolution_clock::now();
			auto next_tick = tick_start + m_Frequency;

			// Run the main function
			m_MainFunc( *this );
			
			auto main_end = std::chrono::high_resolution_clock::now();
			auto next_tick_delta = next_tick - main_end;

			if( m_bAllowTaskInjection )
			{
				// TODO: Run injected tasks
				// TODO: If we have extra time, pause so we hit target frequency
			}
			else
			{
				// TODO: Pause current thread so we hit the targetted tick frequency
			}

		}

		// Run the shutdown function
		if( m_ShutdownFunc )
		{
			m_ShutdownFunc();
		}

		// Ensure state is set to false, signalling the thread is shutting down/shut down
		m_State = false;
	}


	/*----------------------------------------------------------------------------
		StandardThread::Start
	----------------------------------------------------------------------------*/
	bool StandardThread::Start()
	{
		return false;
	}

	/*----------------------------------------------------------------------------
		StandardThread::Stop
	----------------------------------------------------------------------------*/
	bool StandardThread::Stop()
	{
		return false;
	}

	/*----------------------------------------------------------------------------
		StandardThread::ThreadMain
	----------------------------------------------------------------------------*/
	void StandardThread::ThreadMain()
	{
		if( m_MainFunc )
		{
			m_MainFunc( *this );
		}

		m_State = false;
	}

	/*----------------------------------------------------------------------------
		TaskManager::ScheduleNextTask
	----------------------------------------------------------------------------*/
	void TaskManager::ScheduleNextTask()
	{
		// There should already be a lock on the task mutex before calling this
		// If there is still a 'NextTask' then dont do anything
		if( m_NextTask )
			return;

		// We have 3 task lists.. one for each priority level.. we need to determine how long its been since each task has been
		// added, apply a multiplier for each priority level and find the 'oldest' one and select it
		float verylowPriorityValue		= -1.f;
		float lowPriorityValue			= -1.f;
		float normPriorityValue			= -1.f;
		float highPriorityValue			= -1.f;
		float veryhighPriorityValue		= -1.f;

		// We want to ensure any empty tasks are cleared from our lists
		while( !m_VeryLowPriorityTasks.empty() && !m_VeryLowPriorityTasks.top() )
		{
			m_VeryLowPriorityTasks.pop();
		}

		while( !m_LowPriorityTasks.empty() && !m_LowPriorityTasks.top() )
		{
			m_LowPriorityTasks.pop();
		}

		while( !m_NormalPriorityTasks.empty() && !m_NormalPriorityTasks.top() )
		{
			m_NormalPriorityTasks.pop();
		}

		while( !m_HighPriorityTasks.empty() && !m_HighPriorityTasks.top() )
		{
			m_HighPriorityTasks.pop();
		}

		while( !m_VeryHighPriorityTasks.empty() && !m_VeryHighPriorityTasks.top() )
		{
			m_VeryHighPriorityTasks.pop();
		}

		// Calculate time since insertion for the oldest task from each priority to find the best task to run
		auto now = std::chrono::high_resolution_clock::now();
		bool bHasTask = false;

		if( !m_VeryLowPriorityTasks.empty() )
		{
			verylowPriorityValue = (float)( ( now - m_VeryLowPriorityTasks.top()->ScheduledTime ).count() ) * 0.2f;
			bHasTask = true;
		}

		if( !m_LowPriorityTasks.empty() )
		{
			lowPriorityValue = (float)( ( now - m_LowPriorityTasks.top()->ScheduledTime ).count() ) * 0.5f;
			bHasTask = true;
		}

		if( !m_NormalPriorityTasks.empty() )
		{
			normPriorityValue = (float)( ( now - m_NormalPriorityTasks.top()->ScheduledTime ).count() );
			bHasTask = true;
		}

		if( !m_HighPriorityTasks.empty() )
		{
			highPriorityValue = (float)( ( now - m_HighPriorityTasks.top()->ScheduledTime ).count() ) * 2.f;
			bHasTask = true;
		}

		if( !m_VeryHighPriorityTasks.empty() )
		{
			veryhighPriorityValue = (float)( ( now - m_VeryHighPriorityTasks.top()->ScheduledTime ).count() ) * 5.f;
			bHasTask = true;
		}

		// Now we need to find the highest value, and select that task
		if( bHasTask )
		{
			if( verylowPriorityValue > lowPriorityValue )
			{
				if( verylowPriorityValue > normPriorityValue )
				{
					if( verylowPriorityValue > highPriorityValue )
					{
						if( verylowPriorityValue > veryhighPriorityValue )
						{
							// VERY LOW
							m_NextTask = std::move( m_VeryLowPriorityTasks.top() );
							m_VeryLowPriorityTasks.pop();
						}
						else
						{
							// VERY HIGH
							m_NextTask = std::move( m_VeryHighPriorityTasks.top() );
							m_VeryHighPriorityTasks.pop();
						}
					}
					else // verylow > low && norm && high > verylow.. Therefore high > verylow, norm, high
					{
						if( highPriorityValue > veryhighPriorityValue )
						{
							// HIGH
							m_NextTask = std::move( m_HighPriorityTasks.top() );
							m_HighPriorityTasks.pop();
						}
						else
						{
							// VERY HIGH
							m_NextTask = std::move( m_VeryHighPriorityTasks.top() );
							m_VeryHighPriorityTasks.pop();
						}
					}
				}
				else // verylow > low && norm > verylow.. Therefore norm > verylow && low
				{
					if( normPriorityValue > highPriorityValue )
					{
						if( normPriorityValue > veryhighPriorityValue )
						{
							// NORM
							m_NextTask = std::move( m_NormalPriorityTasks.top() );
							m_NormalPriorityTasks.pop();
						}
						else
						{
							// VERY HIGH
							m_NextTask = std::move( m_VeryHighPriorityTasks.top() );
							m_VeryHighPriorityTasks.pop();
						}
					}
					else // norm > low, verylow & high > norm.. therefore high > verylow, low, norm
					{
						if( highPriorityValue > veryhighPriorityValue )
						{
							// HIGH
							m_NextTask = std::move( m_HighPriorityTasks.top() );
							m_HighPriorityTasks.pop();
						}
						else
						{
							// VERY HIGH
							m_NextTask = std::move( m_VeryHighPriorityTasks.top() );
							m_VeryHighPriorityTasks.pop();
						}
					}
				}
			}
			else // low > verylow
			{
				if( lowPriorityValue > normPriorityValue )
				{
					if( lowPriorityValue > highPriorityValue )
					{
						if( lowPriorityValue > veryhighPriorityValue )
						{
							// LOW
							m_NextTask = std::move( m_LowPriorityTasks.top() );
							m_LowPriorityTasks.pop();
						}
						else
						{
							// VERY HIGH
							m_NextTask = std::move( m_VeryHighPriorityTasks.top() );
							m_VeryHighPriorityTasks.pop();
						}
					}
					else // low > verylow, norm & high > low therefore high > low, verylow, norm
					{
						if( highPriorityValue > veryhighPriorityValue )
						{
							// HIGH
							m_NextTask = std::move( m_HighPriorityTasks.top() );
							m_HighPriorityTasks.pop();
						}
						else
						{
							// VERY HIGH
							m_NextTask = std::move( m_VeryHighPriorityTasks.top() );
							m_VeryHighPriorityTasks.pop();
						}
					}
				}
				else // low > verylow, norm > low THEREFORE norm > low, verylow
				{
					if( normPriorityValue > highPriorityValue )
					{
						if( normPriorityValue > veryhighPriorityValue )
						{
							// NORM
							m_NextTask = std::move( m_NormalPriorityTasks.top() );
							m_NormalPriorityTasks.pop();
						}
						else
						{
							// VERY HIGH
							m_NextTask = std::move( m_VeryHighPriorityTasks.top() );
							m_VeryHighPriorityTasks.pop();
						}
					}
					else // norm > low, verylow & high > norm THEREFORE high > norm, low, verylow
					{
						if( highPriorityValue > veryhighPriorityValue )
						{
							// HIGH
							m_NextTask = std::move( m_HighPriorityTasks.top() );
							m_HighPriorityTasks.pop();
						}
						else
						{
							// VERY HIGH
							m_NextTask = std::move( m_VeryHighPriorityTasks.top() );
							m_VeryHighPriorityTasks.pop();
						}
					}
				}
			}
		}

	}

	/*----------------------------------------------------------------------------
		TaskManager::PopNextTask
	----------------------------------------------------------------------------*/
	std::unique_ptr< TaskInstanceBase > TaskManager::PopNextTask()
	{
		// First we need to gain a lock on the task pool members
		std::lock_guard< std::mutex > lock( m_TaskMutex );

		// Next, check if there is a 'next task' scheduled
		if( m_NextTask )
		{
			// Move this task into a local variable so we can output it later
			auto output		= std::move( m_NextTask );
			m_NextTask		= nullptr;

			// We need to calculate the next task to run 
			ScheduleNextTask();

			return output;
		}
		else
		{
			return nullptr;
		}
	}

	/*----------------------------------------------------------------------------
		TaskManager::AddTask
	----------------------------------------------------------------------------*/
	void TaskManager::AddTask( std::unique_ptr< TaskInstanceBase >&& inTask, TaskPriority inPriority )
	{
		// Aquire a lock on the lists so we can perform the needed operations
		std::lock_guard< std::mutex > lock( m_TaskMutex );

		// Add the task to the proper list
		switch( inPriority )
		{
		case TaskPriority::VeryLow:
			m_VeryLowPriorityTasks.push( std::move( inTask ) );
			break;
		case TaskPriority::Low:
			m_LowPriorityTasks.push( std::move( inTask ) );
			break;
		case TaskPriority::Normal:
			m_NormalPriorityTasks.push( std::move( inTask ) );
			break;
		case TaskPriority::High:
			m_HighPriorityTasks.push( std::move( inTask ) );
			break;
		case TaskPriority::VeryHigh:
			m_VeryHighPriorityTasks.push( std::move( inTask ) );
			break;
		}

		// Recalculate the next task to execute
		ScheduleNextTask();
	}

}