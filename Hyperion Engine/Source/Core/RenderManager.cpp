/*==================================================================================================
	Hyperion Engine
	Source/Managers/RenderManager.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/RenderManager.h"
#include "Hyperion/Core/ThreadManager.h"
#include "Hyperion/Console/Console.h"
#include "Hyperion/Renderer/TextureCache.h"

/*
	Graphics API Headers
*/
#if HYPERION_OS_WIN32
#include "Hyperion/Renderer/DirectX11/DirectX11Graphics.h"
#endif


namespace Hyperion
{

	/*
		Console Vars
	*/
	static ConsoleVar< String > g_CVar_Resolution = ConsoleVar< String >(
		"r_resolution", "Screen Resolution [x, y]", "1280, 720",
		std::bind( &RenderManager::OnResolutionUpdated ), THREAD_RENDERER
		);

	static ConsoleVar< String > g_CVar_GraphicsAPI = ConsoleVar< String >(
		"r_api", "Graphics API, changes wont take effect until restart!", ""
		);

	static ConsoleVar< uint32 > g_CVar_Fullscreen = ConsoleVar< uint32 >(
		"r_fullscreen", "Fullscreen mode, 1: Fullscreen, 0: Windowed",
		1, 0, 1, std::bind( &RenderManager::OnResolutionUpdated ), THREAD_RENDERER
		);

	static ConsoleVar< uint32 > g_CVar_VSync = ConsoleVar< uint32 >(
		"r_vsync", "Vertical Sync, 1: On, 0: Off",
		0, 0, 1, std::bind( &RenderManager::OnVSyncUpdated ), THREAD_RENDERER
		);

	/*
		Static Definitions
	*/
	std::shared_ptr< Thread > RenderManager::m_Thread( nullptr );
	std::shared_ptr< Renderer > RenderManager::m_Instance( nullptr );
	IRenderOutput RenderManager::m_OutputWindow;
	std::atomic< ScreenResolution > m_CachedResolution;

	std::mutex m_InitMutex;
	std::condition_variable m_InitCV;
	bool m_InitBool = false;


	bool RenderManager::Start( const IRenderOutput& inOutput, uint32 inFlags /* = 0 */ )
	{
		// Check if were running
		if( m_Thread )
		{
			Console::WriteLine( "[ERROR] RenderManager: Failed to start.. system was still running!" );
			return false;
		}

		// Ensure output target is valid
		if( !inOutput.Value )
		{
			Console::WriteLine( "[ERROR] RenderManager: Failed to start.. invalid output window!" );
			return false;
		}

		// Set output window
		m_OutputWindow = inOutput;

		// Create thread
		TickedThreadParameters params;

		params.Identifier = THREAD_RENDERER;
		params.AllowTasks = true;
		params.Deviation = 0.f;
		params.Frequency = 60.f;
		params.MinimumTasksPerTick = 3;
		params.MaximumTasksPerTick = 0;
		params.StartAutomatically = true;
		params.InitFunction = std::bind( &RenderManager::Init );
		params.ShutdownFunction = std::bind( &RenderManager::Shutdown );
		params.TickFunction = std::bind( &RenderManager::Tick );

		m_Thread = ThreadManager::CreateThread( params );
		Console::WriteLine( "[STATUS] RenderManager: Starting..." );

		// Wait for the API to initialize
		{
			std::unique_lock< std::mutex > lock( m_InitMutex );
			if( !m_InitBool )
			{
				m_InitCV.wait( lock, [] () { return m_InitBool; } );
			}
		}

		return true;
	}


	bool RenderManager::Stop()
	{
		// Check if were running
		if( !m_Thread )
		{
			Console::WriteLine( "[ERROR] RenderManager: Failed to stop.. system was not running!" );
			return false;
		}

		// Shutdown & destroy thread
		if( m_Thread->IsRunning() )
		{
			m_Thread->Stop();
		}

		ThreadManager::DestroyThread( THREAD_RENDERER );
		m_Thread.reset();

		Console::WriteLine( "[STATUS] RenderManager: Shutdown successfully!" );
		return true;
	}


	std::thread::id RenderManager::GetThreadId()
	{
		if( m_Thread )
		{
			return m_Thread->GetSystemIdentifier();
		}

		return std::thread::id();
	}


	bool RenderManager::IsRunning()
	{
		if( !m_Instance || !m_Thread )
		{
			return false;
		}

		return m_Thread->IsRunning();
	}


	void RenderManager::AddImmediateCommand( std::unique_ptr< RenderCommandBase >&& inCommand )
	{
		if( m_Instance )
		{
			m_Instance->AddImmediateCommand( std::move( inCommand ) );
		}
	}


	void RenderManager::AddCommand( std::unique_ptr< RenderCommandBase >&& inCommand )
	{
		if( m_Instance )
		{
			m_Instance->AddCommand( std::move( inCommand ) );
		}
	}


	void RenderManager::OnResolutionUpdated()
	{
		// TODO
	}

	void RenderManager::OnVSyncUpdated()
	{
		// TODO
	}

	std::shared_ptr< IGraphics > RenderManager::CreateAPI( const String& inStr )
	{
		String str = inStr.TrimBoth();
		
		if( str.Equals( "dx11" ) )
		{
			#if HYPERION_OS_WIN32
			return std::make_shared< DirectX11Graphics >();
			#else
			return nullptr;
			#endif
		}
		else
		{
			return nullptr;
		}
	}

	ScreenResolution RenderManager::ReadResolution( const String& inStr, uint32 inFullscreen, bool bPrintErrors )
	{
		auto exp = inStr.Explode( ',' );

		ScreenResolution Output;
		Output.Width = 0;
		Output.Height = 0;
		Output.FullScreen = ( inFullscreen != 0 );

		if( exp.size() == 2 )
		{
			uint32 width;
			uint32 height;

			if( exp.at( 0 ).ToUInt( width ) && exp.at( 1 ).ToUInt( height ) &&
				width >= 720 && height >= 480 )
			{
				Output.Width = width;
				Output.Height = height;
			}
			else if( bPrintErrors )
			{
				Console::WriteLine( "[WARNING] RenderManager: Invalid resolution selected in console.. defaulting to screen resolution" );
			}
		}
		else if( bPrintErrors )
		{
			Console::WriteLine( "[WARNING] RenderManager: Invalid resolution selected in console.. defaulting to screen resolution" );
		}

		return Output;
	}


	void RenderManager::Init()
	{
		HYPERION_VERIFY( m_Instance == nullptr, "During init.. renderer wasnt null!" );

		// Read in the console settings
		String apiStr	= g_CVar_GraphicsAPI.GetValue();
		String resStr	= g_CVar_Resolution.GetValue();
		uint32 vyncVar	= g_CVar_VSync.GetValue();
		uint32 fullVar	= g_CVar_Fullscreen.GetValue();

		// Go through and validate each
		auto apiInst = CreateAPI( apiStr );
		if( !apiInst )
		{
			Console::WriteLine( "[ERROR] RenderManager: Invalid graphics API selected '", apiStr, "'! Falling back to system default" );
			#if HYPERION_OS_WIN32
			apiInst = std::make_shared< DirectX11Graphics >();
			#else
			std::terminate(); // TODO
			#endif
		}

		auto resolution		= ReadResolution( resStr, fullVar );
		bool bVSync			= ( vyncVar != 0 );
	
		/*
			DEBUG
		*/
		resolution.FullScreen = false;
		m_CachedResolution.store( resolution );

		// Now that we have the console settings read in, and the api created, lets setup the renderer
		m_Instance = std::make_shared< Renderer >( apiInst, m_OutputWindow, resolution, bVSync );
		m_Instance->Initialize();

		// Trigger cv
		{
			std::unique_lock< std::mutex > lock( m_InitMutex );
			m_InitBool = true;
		}

		Console::WriteLine( "RenderManager: Finished initializing renderer!" );
		m_InitCV.notify_all();
	}


	void RenderManager::Tick()
	{
		HYPERION_VERIFY( m_Instance, "During tick.. renderer was null!" );
		m_Instance->Frame();
	}


	void RenderManager::Shutdown()
	{
		HYPERION_VERIFY( m_Instance, "During shutdown.. renderer was null!" );

		m_Instance->Shutdown();
		m_Instance.reset();

	}
	
	ScreenResolution RenderManager::GetActiveResolution()
	{
		return m_CachedResolution.load();
	}

}