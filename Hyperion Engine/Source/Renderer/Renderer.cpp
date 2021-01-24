/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Renderer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/Proxy/ProxyScene.h"
#include "Hyperion/Renderer/Proxy/ProxyPrimitive.h"
#include "Hyperion/Renderer/Proxy/ProxyLight.h"
#include "Hyperion/Core/ThreadManager.h"
#include "Hyperion/Core/Asset.h"
#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Library/Math.h"


/*
	Include various graphics APIs?
*/
#if HYPERION_OS_WIN32
#include "Hyperion/Renderer/DirectX11/DirectX11Graphics.h"
#endif


namespace Hyperion
{


	Renderer::Renderer( GraphicsAPI inAPI, void* inWindow, const ScreenResolution& inRes, bool bVSync )
		: m_APIType( inAPI ), m_pWindow( inWindow ), m_Resolution( inRes ), m_bVSync( bVSync ), m_Scene( std::make_shared< ProxyScene >() ), m_AllowCommands( true )
	{
		// We store a copy of the resolution info in an atomic variable
		// This way we can get this info from other threads without a data race
		m_CachedResolution.store( inRes );
		m_bCachedVSync.store( bVSync );

		Console::WriteLine( "[Renderer] Starting renderer at a resolution of ", inRes.Width, "x", inRes.Height, " and in ", inRes.FullScreen ? "fullscreen" : "windowed", " mode" );

		// Create the adaptive asset manaer
		m_StreamingManager	= CreateObject< BasicStreamingManager >();
		m_ResourceManager	= std::make_shared< ResourceManager >();

		// Create the correct API type
		switch( inAPI )
		{
		case GraphicsAPI::DX11:
			m_API = std::make_shared< DirectX11Graphics >();
			break;
		case GraphicsAPI::DX12:
			HYPERION_NOT_IMPLEMENTED( "DirectX 12 not yet impelemented" );
			break;
		case GraphicsAPI::OpenGL:
			HYPERION_NOT_IMPLEMENTED( "OpenGL not yet implemented" );
			break;
		default:
			HYPERION_VERIFY( true, "[Renderer] Invalid graphics API type!" );
			break;
		}

		// Tell the graphics api about the resolution and other settings
		m_API->SetResolution( inRes );
		m_API->SetVSync( bVSync );
	}


	Renderer::~Renderer()
	{
	}


	bool Renderer::Initialize()
	{
		HYPERION_VERIFY( m_API, "[Renderer] API was null during call to 'Initialize'" );

		// Initialize the graphics api
		if( !m_API->Initialize( m_pWindow ) )
		{
			// The api would have printed an error message giving info on why init failed
			return false;
		}

		// Cache a list of available resolutions
		m_AvailableResolutions	= m_API->GetAvailableResolutions();
		m_bCanChangeResolution	= true;

		// Iniitalize the scene
		m_Scene->Initialize();

		return true;
	}


	bool Renderer::ChangeResolution( const ScreenResolution& inRes )
	{
		// Check if we have started up and cached the list of available resolutions
		if( !m_bCanChangeResolution )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to change resolution.. renderer hasnt finished init yet!" );
			return false;
		}

		// Check if the target resolution is supported
		if( inRes.Width < MIN_RESOLUTION_WIDTH || inRes.Height < MIN_RESOLUTION_HEIGHT )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to change resolution.. target resolution is lower than the minimum" );
			return false;
		}

		bool bValid = false;
		for( auto it = m_AvailableResolutions.begin(); it != m_AvailableResolutions.end(); it++ )
		{
			if( inRes.Width == ( *it ).Width && inRes.Height == ( *it ).Height )
			{
				bValid = true;
				break;
			}
		}

		if( !bValid )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to change resolution.. target resolution is not supported" );
			return false;
		}

		// Add a render command to change the resolution, this way we can ensure it happens between render passes
		AddCommand( std::make_unique< RenderCommand >(
			[ inRes ] ( Renderer& r )
			{
				// Tell the API to update the resolution
				if( !r.m_API->SetResolution( inRes ) )
				{
					Console::WriteLine( "[Warning] Renderer: Failed to change resolution.. API call failed" );
					return;
				}
				
				// Cache the resolution
				r.m_CachedResolution.store( inRes );

				// Call OnResolutionUpdated
				r.OnResolutionChanged( inRes );

				Console::WriteLine( "Renderer: Resolution changed to ", inRes.Width, "x", inRes.Height, " with fullscreen ", ( inRes.FullScreen ? "enabled" : "disabled" ) );
			} )
		);

		return true;
	}


	void Renderer::Shutdown()
	{
		// Kill the command queue
		// TODO: Add a 'locking' feature to ConcurrentQueue, where we can stop new entries from being added
		m_AllowCommands = false;
		std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );

		m_Commands.Clear();
		m_ImmediateCommands.Clear();

		//if( m_ResourceManager ) { m_ResourceManager->Shutdown(); }
		m_ResourceManager.reset();

		//if( m_StreamingManager ) { DestroyObject( m_StreamingManager ); }
		if( m_StreamingManager ) { DestroyObject( m_StreamingManager ); }
		m_StreamingManager.Clear();

		//if( m_Scene ) { m_Scene->Shutdown(); }
		m_Scene.reset();

		//if( m_API ) { m_API->Shutdown(); }
		m_API.reset();
	}


	void Renderer::Frame()
	{
		// First, we need to update the proxy scene
		UpdateScene();

		// Get the current camera position, so we can pass that data into the renderer
		ViewState view;
		GetViewState( view );

		m_API->SetCameraInfo( view );

		// Next, we need to prepare the API for the frame
		m_API->BeginFrame();

		// Call derived class to render the current scene
		RenderScene();

		m_API->EndFrame();
	}


	void Renderer::UpdateScene()
	{
		// Execute all immediate commands
		// Were going to run them until the list is empty
		auto nextImmediateCommand = m_ImmediateCommands.PopValue();
		while( nextImmediateCommand.first )
		{
			// Execute
			if( nextImmediateCommand.second )
			{
				nextImmediateCommand.second->Execute( *this );
			}

			// Pop next command
			nextImmediateCommand = m_ImmediateCommands.PopValue();
		}

		// For now, were just going to run the next frame of commands
		auto nextCommand = m_Commands.PopValue();
		while( nextCommand.first )
		{
			// Execute this command
			if( nextCommand.second )
			{
				nextCommand.second->Execute( *this );
			}

			// Check if this is the end of the frame, by the EOF flag
			if( nextCommand.second->HasFlag( RENDERER_COMMAND_FLAG_END_OF_FRAME ) )
			{
				break;
			}

			// Pop the next command in the list
			nextCommand = m_Commands.PopValue();
		}
	}


	void Renderer::AddImmediateCommand( std::unique_ptr< RenderCommandBase >&& inCommand )
	{
		if( !m_AllowCommands ) 
		{ 
			Console::WriteLine( "[Warning] Renderer: Failed to push immediate command, the queue was closed" ); 
			return; 
		}

		m_ImmediateCommands.Push( std::move( inCommand ) );
	}

	void Renderer::AddCommand( std::unique_ptr< RenderCommandBase >&& inCommand )
	{
		if( !m_AllowCommands )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to push command, the queue was closed" );
			return;
		}

		m_Commands.Push( std::move( inCommand ) );
	}


	bool Renderer::AddPrimitive( std::shared_ptr< ProxyPrimitive >& inPrimitive )
	{
		if( !m_Scene )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to add primitive.. scene was null!" );
			return false;
		}

		// Check if a primitive already exists with this identifier
		auto ptr = m_Scene->RemovePrimitive( inPrimitive->GetIdentifier() );
		if( ptr )
		{
			ShutdownProxy( ptr );
		}

		if( m_Scene->AddPrimitive( inPrimitive ) )
		{
			inPrimitive->RenderInit();
			return true;
		}
		else
		{
			return false;
		}
	}


	bool Renderer::AddLight( std::shared_ptr< ProxyLight >& inLight )
	{
		if( !m_Scene )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to add light.. scene was null!" );
			return false;
		}

		auto ptr = m_Scene->RemoveLight( inLight->GetIdentifier() );
		if( ptr )
		{
			ShutdownProxy( ptr );
		}

		if( m_Scene->AddLight( inLight ) )
		{
			inLight->RenderInit();
			return true;
		}
		else
		{
			return false;
		}
	}


	void Renderer::ShutdownProxy( const std::shared_ptr< ProxyBase >& inProxy )
	{
		if( inProxy )
		{
			// Call begin shutdown directly on render thread
			inProxy->BeginShutdown();

			// Create task on pool to finalize shutdown of the proxy so we dont block the render thread
			ThreadManager::CreateTask< void >(
				[ inProxy ] ()
				{
					if( inProxy )
					{
						inProxy->Shutdown();
					}

				} );
		}
	}


	bool Renderer::RemovePrimitive( uint32 inIdentifier )
	{
		if( !m_Scene )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to remove primitive.. scene was null!" );
			return false;
		}

		auto ptr = m_Scene->RemovePrimitive( inIdentifier );
		if( ptr )
		{
			ShutdownProxy( ptr );
			return true;
		}
		else
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to remove primitive.. couldnt find! Id = ", inIdentifier );
			return false;
		}
	}


	bool Renderer::RemoveLight( uint32 inIdentifier )
	{
		if( !m_Scene )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to remove light.. scene was null!" );
			return false;
		}

		auto ptr = m_Scene->RemoveLight( inIdentifier );
		if( ptr )
		{
			ShutdownProxy( ptr );
			return true;
		}

		return false;
	}


	void Renderer::GetViewState( ViewState& outState ) const
	{
		if( m_Scene )
		{
			m_Scene->GetViewState( outState );
		}
		else
		{
			// Return a default view state
			outState.FOV = Math::PIf / 4.f;
			outState.Position.Clear();
			outState.Rotation.Clear();
		}
	}

}