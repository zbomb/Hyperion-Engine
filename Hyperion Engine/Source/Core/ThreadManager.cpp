/*==================================================================================================
	Hyperion Engine
	Source/Managers/ThreadManager.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/ThreadManager.h"

#if HYPERION_OS_WIN32
#include "Hyperion/Win32/Win32Headers.h"
#include <timeapi.h>
#include <mmsystem.h>
#endif


namespace Hyperion
{

	/*
		Static Definitions
	*/
	std::map< std::string, HypPtr< Thread > > ThreadManager::m_Threads {};
	std::mutex ThreadManager::m_ThreadMutex {};
	std::atomic< bool > ThreadManager::m_bIsShutdown( false );
	std::atomic< uint32 > ThreadManager::m_ThreadCount( 0 );
	bool ThreadManager::m_bClosed( false );
	bool ThreadManager::m_bInit( false );
	float ThreadManager::m_MinResolution( 1.f );

	/*
	*	Ticked thread body
	*/
	void _tickedThreadBody( Thread& inThread )
	{
		auto& params = inThread.GetParameters();

		if( params.initFunc )
		{
			params.initFunc();
		}

		bool bInfRate = params.ticksPerSecond == 0.0;
		if( bInfRate )
		{
			// Infinite rate ticked thread, most simple of all types of ticked threads
			while( inThread.m_State.load() )
			{
				params.mainFunc( inThread.m_State );
			}
		}
		else if( params.tickType == ThreadTickBehavior::ClockSynced )
		{
			// This tick method creates a clock, and syncs the tick calls to the clock signal
			// For Windows, were going to use multi-media timers, although this might change in the future, it seems to be the most
			// accurate way to get a high-resolution timer even though the API is considered obsolete
			
			#if HYPERION_OS_WIN32

			// We need to create a sync event, then, the timer is going to trigger this event
			// At the end of each tick, we reset the event and then wait for it to be triggered again
			HANDLE hTick = CreateEvent( NULL, TRUE, FALSE, NULL );
			UINT tickDelay = (UINT) round( 1000.0 / params.ticksPerSecond );

			auto winTimerId = timeSetEvent( tickDelay, 0, (TIMECALLBACK*) hTick, NULL, TIME_PERIODIC | TIME_CALLBACK_EVENT_SET );
			if( winTimerId == NULL )
			{
				Console::WriteLine( "[ERROR] ThreadManager: Failed to create a clock-synced ticked thread, the os-based timer couldnt be created!" );
				if( params.shutdownFunc )
				{
					params.shutdownFunc();
				}

				return;
			}

			// Main tick-loop
			while( inThread.m_State.load() )
			{
				// Reset and wait for the next clock signal
				bool bClockSyncSuccess = false;

				if( ResetEvent( hTick ) )
				{
					// We also want to ensure were checking for the thread shutdown, so this thread doesnt hang
					DWORD waitResult;
					bool bShouldExit = false;

					while( ( waitResult = WaitForSingleObject( hTick, 500 ) ) == WAIT_TIMEOUT )
					{
						if( !inThread.m_State.load() )
						{
							bShouldExit = true;
							break;
						}
					}

					if( bShouldExit ) 
					{
						break;
					}

					if( waitResult == WAIT_OBJECT_0 )
					{
						bClockSyncSuccess = true;
					}
				}

				if( !bClockSyncSuccess )
				{
					Console::WriteLine( "[ERROR] Thread: Ticked thread '", inThread.GetIdentifier(), "' failed to sync with clock!" );
					break;
				}

				// Execute tick function
				params.mainFunc( inThread.m_State );
			}

			// Before exiting the tick cycle, kill the timer
			timeKillEvent( winTimerId );

			#else
			HYPERION_NOT_IMPLEMENTED( "[Thread] Non win-32 clock-synced ticked thread loop" );
			#endif

		}
		else
		{
			// This method is going to basically time the start of the next tick based on the start of the previous tick
			
			#if HYPERION_OS_WIN32

			// To do this one, we will have to use one-time clocks each tick, and the clock will set an event for us to check at the end of the tick cycle
			HANDLE hTick	= CreateEvent( NULL, TRUE, FALSE, NULL );
			UINT tickDelay	= (UINT) round( 1000.0 / params.ticksPerSecond );

			while( inThread.m_State.load() )
			{
				// Set our clock to time when we can start the next tick
				auto winTimerId = timeSetEvent( tickDelay, 0, (TIMECALLBACK*) hTick, NULL, TIME_ONESHOT | TIME_CALLBACK_EVENT_SET );
				if( winTimerId == NULL )
				{
					Console::WriteLine( "[ERROR} Thread: Ticked thread '", params.identifier, "' failed to create tick timer!" );
					break;
				}

				// Execute tick function
				params.mainFunc( inThread.m_State );

				// Wait for the clock signal before next tick
				DWORD waitResult;
				bool bShouldExit = false;

				while( ( waitResult = WaitForSingleObject( hTick, 500 ) ) == WAIT_TIMEOUT )
				{
					if( !inThread.m_State.load() )
					{
						bShouldExit = true;
						break;
					}
				}

				if( bShouldExit )
				{
					break;
				}

				if( waitResult != WAIT_OBJECT_0 || !ResetEvent( hTick ) )
				{
					Console::WriteLine( "[ERROR] Thread: Ticked thread '", params.identifier, "' failed to sync with clock (rate-limited thread)" );
					break;
				}
			}

			#else
			HYPERION_NOT_IMPLEMENTED( "[Thread] Non win-32 rate-limited ticked thread loop" );
			#endif
		}

		if( params.shutdownFunc )
		{
			params.shutdownFunc();
		}
	}

	/*
	*	Non-ticked thread body 
	*/
	void _normalThreadBody( Thread& inThread )
	{
		auto& params = inThread.GetParameters();
		
		if( params.initFunc )
		{
			params.initFunc();
		}

		params.mainFunc( inThread.m_State );

		if( params.shutdownFunc )
		{
			params.shutdownFunc();
		}
	}

	/*--------------------------------------------------------------------------------------
		Thread
	--------------------------------------------------------------------------------------*/
	bool Thread::Start()
	{
		HYPERION_VERIFY( m_Params.mainFunc, "[ThreadManager] Thread main function was not bound!" );

		// Check if the thread is already running
		if( m_State.load() || m_Handle != nullptr )
		{
			Console::WriteLine( "[ERROR] ThreadManager: Attempt to start running a thread, that is already running!" );
			return false;
		}
		
		// Create the thread using the proper thread body
		m_State.store( true );
		m_Handle = std::make_unique< std::thread >( std::bind( m_Params.bIsTicked ? &_tickedThreadBody : &_normalThreadBody, *this ) );
		
		return true;
	}


	bool Thread::Stop()
	{
		// Set the state to false, so the thread knows it should stop
		m_State.store( false );

		if( m_Handle != nullptr )
		{
			if( m_Handle->joinable() )
			{
				m_Handle->join();
				m_Handle.reset();
			}
			else
			{
				Console::WriteLine( "[ERROR] ThreadManager: Failed to stop thread, it wasnt 'joinable'!" );
				return false;
			}
			
		}

		return true;
	}

	/*--------------------------------------------------------------------------------------
		Thread Manager
	--------------------------------------------------------------------------------------*/
	bool ThreadManager::Initialize()
	{
		// Ensure we arent already running
		if( m_bInit )
		{
			Console::WriteLine( "[ERROR] ThreadManager: Attempt to start the thread manager when its already running!" );
			return false;
		}

		m_bInit = true;

		// Depending on the system, we need to determine the accuracy of the timer system
	#if HYPERION_OS_WIN32

		TIMECAPS timerPerf {};

		if( timeGetDevCaps( &timerPerf, sizeof( timerPerf ) ) != MMSYSERR_NOERROR )
		{
			Console::WriteLine( "[ERROR] ThreadManager: Failed to query system timer performance! Ticked threads might not run at the desired frequency" );

			// Fall back on defaults
			m_MinResolution	= 1.f;
		}
		else
		{
			m_MinResolution = (float) timerPerf.wPeriodMin;
		}

		timeBeginPeriod( m_MinResolution );

		Console::WriteLine( "[Startup] ThreadManager: Initialized using Win32 with a timer resolution of ", m_MinResolution, "ms" );
	#else
		
		m_MinResolution = 1.f;

	#endif

		return true;
	}


	bool ThreadManager::Shutdown()
	{
		// Use atomics to ensure this function wasnt already called, and it only gets ran once
		bool bShutdown = m_bIsShutdown.exchange( true );
		if( bShutdown )
		{
			Console::WriteLine( "[ERROR] ThreadManager: Attempt to shutdown, but shutdown was already called!" );
			return false;
		}

		{
			std::lock_guard< std::mutex > lck( m_ThreadMutex );
			m_bClosed = true;

			// Loop through any active threads and shut them down
			for( auto it = m_Threads.begin(); it != m_Threads.end(); it++ )
			{
				if( it->second )
				{
					it->second->Stop();
				}
			}

			m_Threads.clear();
			m_ThreadCount.store( 0 );
		}

		#if HYPERION_OS_WIN32
		timeEndPeriod( (UINT) m_MinResolution );
		#endif

		Console::WriteLine( "[Shutdown] ThreadManager shutdown complete!" );
		return true;
	}


	HypPtr< Thread > ThreadManager::CreateThread( ThreadParameters& inParams )
	{
		// Validate the parameters
		if( inParams.identifier.empty() || !inParams.mainFunc )
		{
			Console::WriteLine( "[ERROR] ThreadManager: Failed to create thread.. parameters were invalid (either identifier or main func)" );
			return nullptr;
		}

		// Check if the tick frequency exceeds the limits of the current OS/hardware
		if( inParams.bIsTicked && inParams.ticksPerSecond > 0 )
		{
			float tickLengthMs = 1000.f / inParams.ticksPerSecond;
			if( tickLengthMs < m_MinResolution )
			{
				inParams.ticksPerSecond = 1000.0 / (double) m_MinResolution;
				Console::WriteLine( "[Warning] ThreadManager: Attempt to create a thread that requires higher tick precision than the system can garuntee, clamping tick rate to ", inParams.ticksPerSecond, "hz" );
			}
		}

		// Aquire a lock on the thread mutex
		HypPtr< Thread > newThread;

		{
			std::lock_guard< std::mutex > lck( m_ThreadMutex );

			// Ensure were not shutdown while holding the lock, ensures threads arent able to be added after shutdown
			if( m_bClosed )
			{
				Console::WriteLine( "[ERROR] ThreadManager: Failed to create thread.. the thread list was shutdown!" );
				return false;
			}

			// Check if a thread with this name already exists
			auto entry = m_Threads.find( inParams.identifier );
			if( entry != m_Threads.end() )
			{
				Console::WriteLine( "[ERROR] ThreadManager: Failed to create thread.. a thread with this name already exists '", inParams.identifier, "'" );
				return false;
			}

			newThread = m_Threads[ inParams.identifier ] = CreateObject< Thread >( inParams );
			HYPERION_VERIFY( newThread != nullptr, "[ThreadManager] Thread couldnt be created?" );

			m_ThreadCount++;
		}

		if( inParams.bAutoStart && !newThread->Start() )
		{
			Console::WriteLine( "[ThreadManager] Failed to create thread.. it couldnt be started" );
			return false;
		}

		return newThread;
	}


	HypPtr< Thread > ThreadManager::GetThread( const std::string& identifier )
	{
		if( identifier.size() == 0 )
			return nullptr;

		// Acquire a lock, and query the list for the identifier
		{
			std::lock_guard< std::mutex > lck( m_ThreadMutex );

			auto entry = m_Threads.find( identifier );
			if( entry == m_Threads.end() )
			{
				return nullptr;
			}

			return entry->second;
		}
	}


	bool ThreadManager::DestroyThread( const std::string& identifier )
	{
		if( identifier.empty() ) { return false; }

		// We need a lock on the thread list
		{
			std::lock_guard< std::mutex > lck( m_ThreadMutex );

			auto entry = m_Threads.find( identifier );
			if( entry == m_Threads.end() || !entry->second.IsValid() )
			{
				return false;
			}

			if( entry->second->IsRunning() )
			{
				entry->second->Stop();
			}

			m_Threads.erase( entry );
			m_ThreadCount--;

			return true;
		}
	}


	uint32 ThreadManager::GetThreadCount()
	{
		return m_ThreadCount.load();
	}

}

HYPERION_REGISTER_ABSTRACT_OBJECT_TYPE( Thread, Object );