/*==================================================================================================
	Hyperion Engine
	Source/Core/Engine.cpp
	© 2020, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/Engine.h"
#include "Hyperion/Core/GameInstance.h"
#include "Hyperion/Core/InputManager.h"
#include "Hyperion/Renderer/DeferredRenderer.h"
#include "Hyperion/File/FileSystem.h"
#include "Hyperion/Core/ThreadManager.h"
#include "Hyperion/Core/Platform.h"

#include <sstream>
#include <iomanip>


namespace Hyperion
{

	/*
		Console Vars
	*/
	ConsoleVar< String > g_CVar_Resolution = ConsoleVar< String >(
		"r_resolution", "Screen Resolution [x, y]", "1280, 720",
		[] ( const String& ) { Engine::Get()->OnResolutionUpdated(); }, THREAD_RENDERER
		);

	ConsoleVar< String > g_CVar_GraphicsAPI = ConsoleVar< String >(
		"r_api", "Graphics API, changes wont take effect until restart!", ""
		);

	ConsoleVar< uint32 > g_CVar_Fullscreen = ConsoleVar< uint32 >(
		"r_fullscreen", "Fullscreen mode, 1: Fullscreen, 0: Windowed",
		1, 0, 1, [] ( uint32 ) { Engine::Get()->OnResolutionUpdated(); }, THREAD_RENDERER
		);

	ConsoleVar< uint32 > g_CVar_VSync = ConsoleVar< uint32 >(
		"r_vsync", "Vertical Sync, 1: On, 0: Off",
		0, 0, 1, [] ( uint32 ) { Engine::Get()->OnVSyncUpdated(); }, THREAD_RENDERER
		);

	std::function< void( const String& ) > Engine::s_FatalErrorCallback( nullptr );
	std::atomic< bool > Engine::s_bFatalError( false );


	HypPtr< Engine > Engine::Get()
	{
		static HypPtr< Engine > engineInst = CreateObject< Engine >();
		return engineInst;
	}


	Engine::Engine()
		: m_bServicesRunning( false ), m_FenceWatcher( std::make_unique< RenderFenceWatcher >() ), m_Input( CreateObject< InputManager >() ),
		m_bGameInit( false ), m_bRenderInit( false )
	{

	}


	bool Engine::InitializeServices( uint32 inFlags /* = FLAG_NONE */ )
	{
		if( m_bServicesRunning )
		{
			Console::WriteLine( "[Warning] Engine: Attempt to initialize runtime services, but they are already running!" );
			return true;
		}

		// First, we need to initialize Runtime-Type-Information
		RTTI::Initialize();

		// Next, lets initialize console
		if( !Console::Start( inFlags ) )
		{
			FatalError( "Failed to initialize console" );
			return false;
		}

		// Initialize file system, and have it run asset discovery
		if( !FileSystem::Initialize( true ) )
		{
			FatalError( "Failed to initialize file system" );
			return false;
		}

		// Finally, initialize thread manager
		if( !ThreadManager::Start( inFlags ) )
		{
			FatalError( "Failed to initialize thread manager" );
			return false;
		}

		m_bServicesRunning = true;
		return true;
	}

	/*--------------------------------------------------------------------------------------------------
		Renderer Code
	--------------------------------------------------------------------------------------------------*/
	ScreenResolution Engine::GetStartupScreenResolution()
	{
		// First, lets check if were supposed to be full screen
		uint32 iFullScreen	= 0;
		bool bFullScreen		= false;
		if( Console::GetVar< uint32 >( "r_fullscreen", iFullScreen ) )
		{
			bFullScreen = ( iFullScreen != 0 );
		}
		else
		{
			// Use default value
			bFullScreen = DEFAULT_RESOLUTION_FULLSCREEN;
		}

		// Now, lets figure out the resolution selected
		String sResolution;
		ScreenResolution Output{};
		Output.FullScreen = bFullScreen;

		if( Console::GetVarAsString( "r_resolution", sResolution ) &&
			Output.LoadFromString( sResolution ) &&
			Output.Width >= MIN_RESOLUTION_WIDTH &&
			Output.Height >= MIN_RESOLUTION_HEIGHT )
		{
			// TODO: Further resolution validation?
		}
		else
		{
			Console::WriteLine( "[Warning] Engine: Failed to read startup resolution from 'r_resolution'.. falling back on engine default" );

			// Use default values.. set console value
			Output.Width	= DEFAULT_RESOLUTION_WIDTH;
			Output.Height	= DEFAULT_RESOLUTION_HEIGHT;

			Console::SetVar< String >( "r_resolution", ToString( Output.Width ) + "," + ToString( Output.Height ), false );
		}

		return Output;
	}


	void Engine::DoRenderThreadInit( void* pWindow, ScreenResolution inResolution, uint32 inFlags )
	{
		// Ensure the renderer wasnt created
		HYPERION_VERIFY( !m_Renderer && pWindow != nullptr, "During render thread init, the renderer object is already created?" );

		// Check if VSync is on
		bool bVSync		= g_CVar_VSync.GetValue() == 0 ? false : true;
		auto loadStart	= std::chrono::high_resolution_clock::now();

		// Determine which api to use
		// First priority is checking for an override in the renderer flags
		GraphicsAPI selectedAPI		= GraphicsAPI::None;
		bool bWasOverride			= true;
		bool bLoadedDefault			= false;

		if( ( inFlags & FLAG_RENDERER_DX11 ) == FLAG_RENDERER_DX11 )
		{
			selectedAPI = GraphicsAPI::DX11;
		}
		else if( ( inFlags & FLAG_RENDERER_DX12 ) == FLAG_RENDERER_DX12 )
		{
			selectedAPI = GraphicsAPI::DX12;
		}
		else if( ( inFlags & FLAG_RENDERER_OGL ) == FLAG_RENDERER_OGL )
		{
			selectedAPI = GraphicsAPI::OpenGL;
		}
		else
		{
			// If there isnt an override specified through flags, then lets check if an api type is set in console
			auto str		= g_CVar_GraphicsAPI.GetValue().TrimBoth().ToLower();
			bWasOverride	= false;

			if( str.Equals( "dx11" ) )
			{
				selectedAPI = GraphicsAPI::DX11;
			}
			else if( str.Equals( "dx12" ) )
			{
				selectedAPI = GraphicsAPI::DX12;
			}
			else if( str.Equals( "opengl" ) )
			{
				selectedAPI = GraphicsAPI::OpenGL;
			}
			else
			{
				// Fallback onto system default
				auto apiId		= Platform::GetDefaultGraphicsAPI();
				bLoadedDefault	= true;

				switch( apiId )
				{
				case FLAG_RENDERER_DX11:
					selectedAPI = GraphicsAPI::DX11;
					g_CVar_GraphicsAPI.SetValue( "dx11" );
					Console::WriteLine( "[Engine] Graphics API set to system default (DirectX11)" );
					break;
				case FLAG_RENDERER_DX12:
					selectedAPI = GraphicsAPI::DX12;
					g_CVar_GraphicsAPI.SetValue( "dx12" );
					Console::WriteLine( "[Engine] Graphics API set to system default (DirectX12)" );
					break;
				case FLAG_RENDERER_OGL:
					selectedAPI = GraphicsAPI::OpenGL;
					g_CVar_GraphicsAPI.SetValue( "opengl" );
					Console::WriteLine( "[Engine] Graphics API set to system default (OpenGL)" );

					break;
				default:
					HYPERION_VERIFY( true, "[Engine] Invalid platform default graphics api" );
					break;
				}
			}
		}

		String apiName;
		switch( selectedAPI )
		{
		case GraphicsAPI::DX11:
			apiName = "DirectX 11";
			break;
		case GraphicsAPI::DX12:
			apiName = "DirectX 12";
			break;
		case GraphicsAPI::OpenGL:
			apiName = "OpenGL";
			break;
		default:
			HYPERION_VERIFY( true, "[Engine] A valid graphics api was not selected!" );
		}

		if( bWasOverride )
		{
			Console::WriteLine( "[Engine] Graphics API selection overriden by launch arguments, now set to ", apiName );
		}
		else
		{
			Console::WriteLine( "[Engine] Graphics API is set to ", apiName );
		}

		// Now we should have all the info needed to create the renderer
		auto rendererInstance = std::make_shared< Hyperion::DeferredRenderer >( selectedAPI, pWindow, inResolution, bVSync );
		m_Renderer = rendererInstance;
		
		if( !rendererInstance->Initialize() )
		{
			// Renderer failed to initialize! We need to stop the render thread from running, and output an error through the OS layer
			FatalError( "Failed to initialize renderer!" );
			return;
		}

		m_LastRenderTick = std::chrono::high_resolution_clock::now();

		std::chrono::duration< double > loadTime = std::chrono::high_resolution_clock::now() - loadStart;
		std::ostringstream ss;
		ss << std::fixed;
		ss << std::setprecision( 2 );
		ss << loadTime.count();

		Console::WriteLine( "[Engine] Renderer initialized in ", ss.str(), " seconds" );

		{
			std::unique_lock< std::mutex > lock( m_RenderWaitMutex );
			m_bRenderInit = true;
			m_RenderWaitCondition.notify_all();
		}

	}


	void Engine::DoRenderThreadTick()
	{
		if( !m_bRenderInit ) { return; }
		HYPERION_VERIFY( m_Renderer, "[Engine] Renderer was null!" );

		m_Renderer->Frame();
	}


	void Engine::DoRenderThreadShutdown()
	{
		HYPERION_VERIFY( m_Renderer, "[Engine] Renderer was null!" );

		m_Renderer->Shutdown();
		m_Renderer.reset();
	}


	void Engine::OnResolutionUpdated()
	{
		// TODO
	}


	void Engine::OnVSyncUpdated()
	{
		// TODO
	}


	bool Engine::InitializeRenderer( void* pWindow, ScreenResolution inResolution, uint32 inFlags /* = FLAG_NONE */ )
	{
		// First, check if the renderer is already created
		if( m_Renderer || m_RenderThread.IsValid() )
		{
			Console::WriteLine( "[Warning] Engine: Attempt to intiialize renderer, but it is already running!" );
			return true;
		}

		if( pWindow == nullptr )
		{
			FatalError( "Invalid window handle" );
			return false;
		}

		// Next, we need to perform basic validation on the resolution were being told to initialize at
		// TODO: Better way to handle this, maybe a better check, or remove the check all together
		if( inResolution.Width < MIN_RESOLUTION_WIDTH || inResolution.Height < MIN_RESOLUTION_HEIGHT )
		{
			inResolution.Width		= DEFAULT_RESOLUTION_WIDTH;
			inResolution.Height		= DEFAULT_RESOLUTION_HEIGHT;
		}

		// Create the render thread
		TickedThreadParameters params;

		params.Identifier			= THREAD_RENDERER;
		params.AllowTasks			= true;
		params.Deviation			= 0.f;
		params.Frequency			= 60.f;
		params.MinimumTasksPerTick	= 3;
		params.MaximumTasksPerTick	= 0;
		params.StartAutomatically	= true;
		params.InitFunction			= std::bind( &Engine::DoRenderThreadInit, this, pWindow, inResolution, inFlags );
		params.ShutdownFunction		= std::bind( &Engine::DoRenderThreadShutdown, this );
		params.TickFunction			= std::bind( &Engine::DoRenderThreadTick, this );

		m_RenderThread = ThreadManager::CreateThread( params );
		if( !m_RenderThread )
		{
			FatalError( "Failed to create render thread" );
			return false;
		}

		return true;
	}


	void Engine::ShutdownRenderer()
	{
		if( m_RenderThread )
		{
			if( m_RenderThread->IsRunning() )
			{
				m_RenderThread->Stop();
			}

			ThreadManager::DestroyThread( THREAD_RENDERER );
			m_RenderThread.Clear();
		}
		else
		{
			Console::WriteLine( "[Engine] ERROR: Failed to shutdown renderer.. thread wasnt running?" );
		}
	}


	/*--------------------------------------------------------------------------------------------------
		GameInstance Code
	--------------------------------------------------------------------------------------------------*/
	bool Engine::InitializeGame( uint32 inFlags /* = FLAG_NONE */ )
	{
		// Check if its already running
		if( m_Game || m_GameThread.IsValid() )
		{
			Console::WriteLine( "[ERROR] Engine: Attempt to initialize game, but it is already running!" );
			return false;
		}

		// Create our thread
		TickedThreadParameters gameThreadParams;
		gameThreadParams.Identifier = THREAD_GAME;
		gameThreadParams.Frequency = 0.f;
		gameThreadParams.Deviation = 0.f;
		gameThreadParams.MinimumTasksPerTick = 1;
		gameThreadParams.MaximumTasksPerTick = 0;
		gameThreadParams.AllowTasks = true;
		gameThreadParams.StartAutomatically = true;
		gameThreadParams.InitFunction = std::bind( &Engine::DoGameThreadInit, this );
		gameThreadParams.TickFunction = std::bind( &Engine::DoGameThreadTick, this );
		gameThreadParams.ShutdownFunction = std::bind( &Engine::DoGameThreadShutdown, this );

		m_GameThread = ThreadManager::CreateThread( gameThreadParams );
		return true;
	}


	void Engine::ShutdownGame()
	{
		if( m_GameThread )
		{
			if( m_GameThread->IsRunning() )
			{
				m_GameThread->Stop();
			}

			ThreadManager::DestroyThread( THREAD_GAME );
			m_GameThread.Clear();
		}
		else
		{
			Console::WriteLine( "[ERROR] Engine: Failed to shutdown game thread.. thread wasnt running?" );
		}
	}


	void Engine::DoGameThreadInit()
	{
		HYPERION_VERIFY( m_Game == nullptr, "[Engine] Game system was already initialized?" );

		// Create the 'correct' type of game instance
		// TODO: Create 'StaticConsoleVars' where we can define these types using program memory, so its constant
		// For now, were going to use the name to lookup the class we want
		auto loadStart = std::chrono::high_resolution_clock::now();

		HypPtr< Type > instanceType = Type::Get( TYPE_OVERRIDE_GAME_INSTANCE );
		m_Game = instanceType.IsValid() ? CastObject< GameInstance >( instanceType->CreateInstance() ) : nullptr;

		if( !instanceType || !m_Game )
		{
			Console::WriteLine( "[ERROR] Engine: Invalid game instance type selected!" );
			HYPERION_VERIFY( true, "Invalid game instance type selected" );
			return;
		}

		m_LastGameTick = std::chrono::high_resolution_clock::now();
		m_FenceWatcher->Reset();

		// Load key bindings
		m_Input->LoadBindingsFromDisk();

		std::chrono::duration< double > loadTime = std::chrono::high_resolution_clock::now() - loadStart;
		std::ostringstream ss;
		ss << std::fixed;
		ss << std::setprecision( 2 );
		ss << loadTime.count();

		Console::WriteLine( "[Engine] Game thread initialized in ", ss.str(), " seconds" );

		{
			std::unique_lock< std::mutex > lock( m_GameWaitMutex );
			m_bGameInit = true;
			m_GameWaitCondition.notify_all();
		}
	}


	void Engine::DoGameThreadTick()
	{
		if( !m_bGameInit ) { return; }
		HYPERION_VERIFY( m_Game && m_Input, "[Engine] Game instance (or input manager) was null during tick!" );

		// If the render thread isnt running, or is behind on processing frames, we will skip this tick
		// TODO: Is this the best way to do this? Probably not
		if( !m_RenderThread || !m_RenderThread->IsRunning() || !m_Renderer ||
			!m_FenceWatcher->WaitForCount( 1, std::chrono::milliseconds( 10 ), ComparisonType::LESS_THAN_OR_EQUAL ) )
		{
			return;
		}

		// Tick Input System
		m_Input->ProcessUpdates();
		auto now = std::chrono::high_resolution_clock::now();
		std::chrono::duration< double > dur = now - m_LastGameTick;
		TickObjectsInput( *m_Input, dur.count() );
		m_LastGameTick = now;

		// Tick Objects
		TickObjects();

		// Send updates to render thread for next frame
		m_Game->ProcessRenderUpdates();

		// Update Fence
		auto frameEndCmd = m_FenceWatcher->CreateCommand();
		frameEndCmd->EnableFlag( RENDERER_COMMAND_FLAG_END_OF_FRAME );

		m_Renderer->AddCommand( std::move( frameEndCmd ) );
	}


	void Engine::DoGameThreadShutdown()
	{
		HYPERION_VERIFY( m_Game, "Game was null during shutdown" );
		HYPERION_VERIFY( m_Input, "Input Manager was null during shutdown" );

		DestroyObject( m_Game );
		m_Game.Clear();

		m_FenceWatcher->Reset();

		// Clear key bindings
		m_Input->RemoveAllBindings();
		
	}


	void Engine::ShutdownServices()
	{
		ThreadManager::Stop();
		Console::Stop();
		FileSystem::Shutdown();
	}


	ScreenResolution Engine::GetResolution() const
	{
		if( m_Renderer )
		{
			return m_Renderer->GetResolutionSafe();
		}
		else
		{
			ScreenResolution output{};

			output.Width		= DEFAULT_RESOLUTION_WIDTH;
			output.Height		= DEFAULT_RESOLUTION_HEIGHT;
			output.FullScreen	= DEFAULT_RESOLUTION_FULLSCREEN;

			return output;
		}
	}

	bool Engine::IsVSyncOn() const
	{
		if( m_Renderer )
		{
			return m_Renderer->IsVSyncOnSafe();
		}
		else
		{
			return false;
		}
	}


	void Engine::FatalError( const String& inDescription )
	{
		// Push the erorr message up into the OS layer, so it can be dipslayed to the user
		if( s_FatalErrorCallback ) { s_FatalErrorCallback( inDescription ); }
		Console::WriteLine( "[FATAL] There was a fatal error! ", inDescription );

		// If the OS is waiting for the renderer or game thread to init, we need to stop blocking the OS thread 
		// so it can process the fatal error were pushing to it
		s_bFatalError.store( true );
	}


	void Engine::Stop()
	{
		ShutdownGame();
		ShutdownRenderer();
		ShutdownServices();
	}


	void Engine::Shutdown()
	{
		Stop();
	}


	void Engine::WaitForInitComplete()
	{
		{
			std::unique_lock< std::mutex > lock( m_GameWaitMutex );
			while( !m_bGameInit && !s_bFatalError ) { m_GameWaitCondition.wait_for( lock, std::chrono::milliseconds( 10 ) ); }
			
		}

		{
			std::unique_lock< std::mutex > lock( m_RenderWaitMutex );
			while( !m_bRenderInit && !s_bFatalError ) { m_RenderWaitCondition.wait_for( lock, std::chrono::milliseconds( 10 ) ); }
		}
	}

}

HYPERION_REGISTER_OBJECT_TYPE( Engine, Object );