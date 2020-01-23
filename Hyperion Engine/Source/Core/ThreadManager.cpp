/*==================================================================================================
	Hyperion Engine
	Source/Managers/ThreadManager.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/ThreadManager.h"


namespace Hyperion
{

	/*
		Static Definitions
	*/
	std::map< std::string, std::shared_ptr< TickedThread > > ThreadManager::m_TickedThreads;
	std::map< std::string, std::shared_ptr< CustomThread > > ThreadManager::m_CustomThreads;
	std::vector< std::shared_ptr< PoolWorkerThread > > ThreadManager::m_TaskPoolThreads;
	bool ThreadManager::m_bRunning( false );


	bool ThreadManager::Start( uint32 inFlags /* = 0 */ )
	{
		// Check if were already running
		if( m_bRunning )
		{
			// TODO: Use console
			Console::Write( "[ERROR] ThreadSystem: Attempt to start when already running!\n" );
			return false;
		}

		m_bRunning = true;

		// Create our worker pool
		auto tc = std::thread::hardware_concurrency();

		if( tc <= 1 ) tc = 3; else tc--;
		for( uint32 i = 0; i < tc; i++ )
		{
			auto newWorker = std::make_shared< PoolWorkerThread >();
			newWorker->Start();

			m_TaskPoolThreads.push_back( newWorker );
		}

		Console::Write( "[STATUS] ThreadSystem: Initialized successfully! Running ", tc, " worker threads.\n" );

		return true;
	}


	bool ThreadManager::Stop()
	{
		// Ensure were running
		if( !m_bRunning && m_TaskPoolThreads.size() == 0 &&
			m_CustomThreads.size() == 0 && m_TickedThreads.size() == 0 )
		{
			Console::Write( "[ERROR] ThreadSystem: Attempt to shutdown, but this system wasnt running!\n" );
			return false;
		}

		m_bRunning = false;

		// Shut down all active threads
		Console::Write( "[STATUS] ThreadSystem: Shutting down engine threads...\n" );

		for( auto& th : m_CustomThreads )
		{
			if( th.second && th.second->IsRunning() ) th.second->Stop();
		}

		m_CustomThreads.clear();

		for( auto& th : m_TickedThreads )
		{
			if( th.second && th.second->IsRunning() ) th.second->Stop();
		}

		m_TickedThreads.clear();

		Console::Write( "[STATUS] ThreadSystem: Shutting down task pool threads...\n" );

		for( auto& th : m_TaskPoolThreads )
		{
			if( th ) th->Stop();
		}

		m_TaskPoolThreads.clear();

		Console::Write( "[STATUS] ThreadSystem: Shutdown successful!\n" );

		return true;
	}


	std::shared_ptr< Thread > ThreadManager::CreateThread( const TickedThreadParameters& params )
	{
		// Validate the parameters
		if( params.Identifier.size() == 0 )
		{
			Console::Write( "[ERROR] ThreadSystem: Attempt to create a thread without a valid identifier\n" );
			return nullptr;
		}
		else if( !params.TickFunction )
		{
			Console::Write( "[ERROR] ThreadSystem: Failed to create thread '", params.Identifier, "' because the tick function was not specified\n" );
			return nullptr;
		}
		else if( params.MinimumTasksPerTick > params.MaximumTasksPerTick&& params.MaximumTasksPerTick != 0 )
		{
			Console::Write( "[ERROR] ThreadSystem: Failed to create thread '", params.Identifier, "' because the minimum task count is greater than the maximum task count\n" );
			return nullptr;
		}
		else if( GetThread( params.Identifier ) )
		{
			Console::Write( "[ERROR] ThreadSystem: Failed to create thread '", params.Identifier, "' because a thread with that identifier already exists\n" );
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
			Console::Write( "[ERROR] ThreadManager: Attempt to create a thread without a valid identifier\n" );
			return nullptr;
		}
		else if( !params.ThreadFunction )
		{
			Console::Write( "[ERROR] ThreadManager: Failed to create thread '", params.Identifier, "' because the tick function was not specified\n" );
			return nullptr;
		}
		else if( GetThread( params.Identifier ) )
		{
			Console::Write( "[ERROR] ThreadManager: Failed to create thread '", params.Identifier, "' because a thread with that identifier already exists\n" );
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


	bool ThreadManager::IsWorkerThread( const std::thread::id& inIdentifier )
	{
		// If we were passed a default thread id, then return false
		if( inIdentifier == std::thread::id() )
			return false;

		for( auto It = m_TaskPoolThreads.begin(); It != m_TaskPoolThreads.end(); It++ )
		{
			if( *It && ( *It )->GetSystemIdentifier() == inIdentifier )
				return true;
		}

		return false;
	}

}