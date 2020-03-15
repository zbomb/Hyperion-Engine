/*==================================================================================================
	Hyperion Engine
	Source/Managers/GameManager.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/GameManager.h"
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Core/RenderManager.h"
#include "Hyperion/Framework/World.h"


namespace Hyperion
{

	/*
		Static Definitions
	*/
	HypPtr< GameInstance > GameManager::m_Instance( nullptr );
	std::shared_ptr< Thread > GameManager::m_Thread( nullptr );
	std::shared_ptr< InstanceFactoryBase > GameManager::m_Factory( nullptr );
	std::unique_ptr< RenderFenceWatcher > GameManager::m_FenceWatcher( std::make_unique< RenderFenceWatcher >() );
	std::chrono::time_point< std::chrono::high_resolution_clock > GameManager::m_LastTick;
	InputManager GameManager::m_InputManager;


	bool GameManager::Start( uint32 inFlags /* = 0 */, std::shared_ptr< InstanceFactoryBase > inFactory /* = nullptr */ )
	{
		// Check if were already running
		if( m_Thread )
		{
			Console::WriteLine( "[ERROR] GameSystem: Failed to start... system is still running!" );
			return false;
		}

		HYPERION_VERIFY( m_Instance == nullptr, "Went to start game system.. but instance wasnt null!" );

		m_Factory = inFactory;

		// Create our thread
		TickedThreadParameters gameThreadParams;
		gameThreadParams.Identifier = THREAD_GAME;
		gameThreadParams.Frequency = 0.f;
		gameThreadParams.Deviation = 0.f;
		gameThreadParams.MinimumTasksPerTick = 1;
		gameThreadParams.MaximumTasksPerTick = 0;
		gameThreadParams.AllowTasks = true;
		gameThreadParams.StartAutomatically = true;
		gameThreadParams.InitFunction = std::bind( &GameManager::Init );
		gameThreadParams.TickFunction = std::bind( &GameManager::Tick );
		gameThreadParams.ShutdownFunction = std::bind( &GameManager::Shutdown );

		m_Thread = ThreadManager::CreateThread( gameThreadParams );
		Console::WriteLine( "[STATUS] GameSystem: Starting..." );

		return true;
	}


	bool GameManager::Stop()
	{
		// Check if were running
		if( !m_Thread )
		{
			Console::WriteLine( "[ERROR] GameSystem: Failed to stop the game system.. wasnt running!" );
			return false;
		}

		// Stop & destroy the thread
		if( m_Thread->IsRunning() )
		{
			m_Thread->Stop();
		}

		ThreadManager::DestroyThread( THREAD_GAME );
		m_Thread.reset();

		Console::WriteLine( "[STATUS] GameSystem: Shutdown successfully!" );
		return true;
	}


	std::thread::id GameManager::GetThreadId()
	{
		if( m_Thread )
		{
			return m_Thread->GetSystemIdentifier();
		}

		return std::thread::id();
	}


	void GameManager::Init()
	{
		// Create our game instance and initialize it
		HYPERION_VERIFY( m_Instance == nullptr, "During thread initialization.. the game instance wasnt null!" );

		if( m_Factory )
		{
			m_Instance = m_Factory->Create();
			if( !m_Instance )
			{
				m_Instance = CreateObject< GameInstance >();
			}
		}
		else
		{
			m_Instance = CreateObject< GameInstance >();
		}

		// Reset the render fence
		m_FenceWatcher->Reset();
		m_LastTick = std::chrono::high_resolution_clock::now();

		Console::WriteLine( "[STATUS] GameSystem: Started successfully!" );
	}


	void GameManager::Tick()
	{
		HYPERION_VERIFY( m_Instance, "During tick... game instance was null!" );

		// Were going to sync with the render system
		// If the renderer isnt running, then we wont tick either
		// Wait until the renderer catches up to the game thread, after a little, we will skip this tick
		if( !RenderManager::IsRunning() ||
			!m_FenceWatcher->WaitForCount( 1, std::chrono::milliseconds( 10 ), ComparisonType::LESS_THAN_OR_EQUAL ) )
		{
			return;
		}

		auto Now = std::chrono::high_resolution_clock::now();

		// Tick Input System
		m_InputManager.DispatchQueue( std::chrono::duration_cast<std::chrono::duration< double, std::milli >>( Now - m_LastTick ).count() );
		m_LastTick = Now;

		// Tick Objects
		TickObjects();

		// Send proxy updates
		m_Instance->ProcessRenderUpdates();

		// Now, let the render thread know that the commands for this frame are complete
		if( RenderManager::IsRunning() )
		{
			auto frameEndCommand = m_FenceWatcher->CreateCommand();
			frameEndCommand->EnableFlag( RENDERER_COMMAND_FLAG_END_OF_FRAME );

			RenderManager::AddCommand( std::move( frameEndCommand ) );
		}


	}


	void GameManager::Shutdown()
	{
		HYPERION_VERIFY( m_Instance, "During shutdown.. game instance was null!" );

		DestroyObject( m_Instance );
		m_Instance.Clear();

		m_FenceWatcher->Reset();
		m_InputManager.ClearAllBindings();
	}

}