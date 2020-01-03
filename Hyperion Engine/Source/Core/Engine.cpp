/*==================================================================================================
	Hyperion Engine
	Source/Core/Engine.cpp
	© 2019, Zachary Berry
==================================================================================================*/


#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Framework/Entity.h"
#include "Hyperion/Framework/World.h"
#include "Hyperion/Renderer/RenderFactory.h"
#include "Hyperion/Core/File.h"
#include "Hyperion/Core/Platform.h"
#include <typeindex>
#include <type_traits>

// Assets
#include "Hyperion/Assets/TestAsset.h"

// DEBUG
#include <thread>
#include <chrono>

std::unique_ptr< Hyperion::Engine > Hyperion::Engine::m_Instance;

namespace Hyperion
{

	/*
		Constructor
	*/
	Engine::Engine()
	{
		m_Status		= EngineStatus::NotStarted;

		// Initialize any platform services
		Platform::Init();

		// Register asset loader functions
		RegisterAssetLoaders();
	}

	Engine::~Engine()
	{
		if( m_Status != EngineStatus::Shutdown )
		{
			std::cout << "[FATAL] Engine: Shutdown detected.. but the proper shutdown sequence was never called!\n";
			Shutdown();
		}
	}

	/*
		Engine Main Thread Functions
	*/
	void Engine::TickEngine()
	{
		// Wait until were within a frame of the renderer
		if( m_Renderer && m_Renderer->IsRunning() && m_FenceWatcher )
		{
			// If we dont catch up within a few milliseconds.. then we just timeout and skip this tick
			// This also prevents this thread from locking up when shutting down
			if( !m_FenceWatcher->WaitForCount( 1, std::chrono::milliseconds( 3 ), ComparisonType::LESS_THAN_OR_EQUAL ) )
				return;
		}

		auto Now = std::chrono::high_resolution_clock::now();
		std::chrono::duration< double > newDelta = Now - m_lastTick;

		// Dispatch input events
		m_InputManager.DispatchQueue( newDelta.count() );

		// Store current time
		m_lastTick = Now;

		auto ms = std::chrono::duration_cast< std::chrono::microseconds >( newDelta ).count();
		//std::cout << "---> Engine took " << ( ms / 1000 ) << "ms to tick (Approx: " << ( 1000000.f / (float)ms ) << "hz)\n";

		// Tick all objects
		TickObjects();

		// Tell renderer that were finished with ticking
		if( m_Renderer && m_Renderer->IsRunning() && m_FenceWatcher )
		{
			auto eofCommand = m_FenceWatcher->CreateCommand();
			eofCommand->EnableFlag( RENDERER_COMMAND_FLAG_END_OF_FRAME );

			m_Renderer->PushCommand( std::move( eofCommand ) );
		}
	}

	void Engine::InitEngine()
	{
		std::cout << "[DEBUG] Engine: Main engine thread init...\n";
		m_lastTick = std::chrono::high_resolution_clock::now();

		// Allow derived class to initialize
		OnInitialize();
	}

	void Engine::ShutdownEngine()
	{
		std::cout << "[DEBUG] Engine: Main engine thread shutdown...\n";

		// Allow derived class to shutdown
		OnShutdown();

		// Clear the active world
		ClearActiveWorld();
	}

	/*
		Engine::Startup()
		* Starts the engine, creates threads, etc...
	*/
	bool Engine::Startup()
	{
		// Check status
		if( m_Status != EngineStatus::NotStarted )
		{
			std::cout << "[ERROR] Engine: Attempt to start the engine, while the engine is in the wrong state!\n";
			return false;
		}

		// Update Status
		m_Status = EngineStatus::Init;

		// Create sub-objects
		m_ThreadManager		= CreateObject< ThreadManager >();
		m_Renderer			= IRenderFactory::CreateRenderer( RendererType::DirectX11 );

		m_FenceWatcher = std::make_unique< RenderFenceWatcher >();

		// Create our game thread
		TickedThreadParameters gameThreadParams;
		gameThreadParams.Identifier				= THREAD_GAME;
		gameThreadParams.Frequency				= 0.f;
		gameThreadParams.Deviation				= 0.f;
		gameThreadParams.MinimumTasksPerTick	= 1;
		gameThreadParams.MaximumTasksPerTick	= 0;
		gameThreadParams.AllowTasks				= true;
		gameThreadParams.StartAutomatically		= true;
		gameThreadParams.InitFunction			= std::bind( &Engine::InitEngine, this );
		gameThreadParams.TickFunction			= std::bind( &Engine::TickEngine, this );
		gameThreadParams.ShutdownFunction		= std::bind( &Engine::ShutdownEngine, this );

		auto gameThread = m_ThreadManager->CreateThread( gameThreadParams );
		if( !gameThread )
		{
			std::cout << "[ERROR] Engine: Failed to create the main game thread!\n";
			Shutdown();
			return false;
		}
		
		// Start the renderer
		if( !m_renderTarget )
		{
			std::cout << "[ERROR] Engine: Failed to startup.. render target wasnt set!\n";
			Shutdown();
			return false;
		}

		ScreenResolution resolution;
		resolution.FullScreen = false;
		resolution.Width = 1280;
		resolution.Height = 720;

		m_Renderer->SetRenderTarget( m_renderTarget );
		m_Renderer->SetVSync( false );
		m_Renderer->SetScreenResolution( resolution );
		m_Renderer->Start();

		// Create the game world
		HypPtr< World > newWorld = CreateObject< World >();
		if( !SetActiveWorld( newWorld ) || !newWorld->SpawnWorld() )
		{
			std::cout << "[ERROR] Engine: Failed to set the active starting world!\n";
		}
		
		// Store thread id's for the game thread
		__gGameThreadId		= gameThread->GetSystemIdentifier();

		// Update status
		m_Status = EngineStatus::Running;

		return true;
	}

	bool Engine::Shutdown()
	{
		if( m_Status != EngineStatus::Running )
		{
			std::cout << "[ERROR] Engine: Attempt to stop the engine when it wasnt running!\n";
			//return false;
		}

		// Update Status
		m_Status = EngineStatus::Shutdown;

		// Clear all input mappings
		m_InputManager.ClearAllBindings();

		// Shutdown the world
		ClearActiveWorld();

		// Shutdown renderer
		if( m_Renderer )
		{
			m_Renderer->Stop();
		}

		// Stop all of our threads.. for now, this is a blocking call and it will call thread shutdown functions
		if( m_ThreadManager )
		{
			m_ThreadManager->StopThreads();
			DestroyObject( m_ThreadManager );
		}

		// Clear stored thread id's
		__gGameThreadId				= std::thread::id();
		__gRenderThreadId			= std::thread::id();
		__gRenderMarshalThreadId	= std::thread::id();

		return true;
	}


	bool Engine::SetActiveWorld( const HypPtr< World >& inWorld )
	{
		// If there is an active world.. then clear it
		ClearActiveWorld();

		// Validate the world
		if( !inWorld || !inWorld->IsValid() )
		{
			std::cout << "[ERROR] Engine: Attempt to set an invalid active world!\n";
			return false;
		}

		m_ActiveWorld = inWorld;
		m_ActiveWorld->m_bActive = true;
		m_ActiveWorld->OnSetActive();

		return true;
	}

	bool Engine::ClearActiveWorld()
	{
		if( !m_ActiveWorld || !m_ActiveWorld->IsValid() )
		{
			return false;
		}

		if( m_ActiveWorld->IsSpawned() )
		{
			if( !m_ActiveWorld->DespawnWorld() )
			{
				std::cout << "[ERROR] Engine: Failed to despawn world!\n";
				return false;
			}
		}

		m_ActiveWorld->m_bActive = false;
		m_ActiveWorld->OnSetDeactive();

		m_ActiveWorld.Clear();
		return true;
	}


	void Engine::RegisterAssetLoaders()
	{
		AssetLoader::RegisterAssetType< TestAsset >( "test_asset.htxt" );
	}


}