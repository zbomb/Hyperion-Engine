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
#include "Hyperion/Library/Math/MathCore.h"


/*
	Include various graphics APIs?
*/
#if HYPERION_OS_WIN32
#include "Hyperion/Renderer/DirectX11/DirectX11Graphics.h"
#endif


namespace Hyperion
{


	Renderer::Renderer( GraphicsAPI inAPI, void* inWindow, const ScreenResolution& inRes, bool bVSync )
		: m_APIType( inAPI ), m_pWindow( inWindow ), m_Resolution( inRes ), m_bVSync( bVSync ), m_Scene( std::make_shared< ProxyScene >() )
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
		Shutdown();
	}


	bool Renderer::Initialize()
	{
		HYPERION_VERIFY( m_API, "[Renderer] API was null during call to 'Initialize'" );

		// Initialize the graphics api
		if( !m_API->Initialize( m_pWindow ) )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to initialize! Graphics API init failed!" );
			HYPERION_VERIFY( true, "[Renderer] GraphicsAPI failed to initialize" );
			return false;
		}

		// Iniitalize the scene
		m_Scene->Initialize();
		OnInitialize();

		return true;
	}


	void Renderer::Shutdown()
	{
		m_StreamingManager.Clear();
		m_Scene.reset();
		m_API.reset();
	}


	void Renderer::Frame()
	{
		// First, we need to update the proxy scene
		UpdateScene();

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
		m_ImmediateCommands.Push( std::move( inCommand ) );
	}

	void Renderer::AddCommand( std::unique_ptr< RenderCommandBase >&& inCommand )
	{
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