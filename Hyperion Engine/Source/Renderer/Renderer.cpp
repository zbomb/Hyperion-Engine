/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Renderer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/Proxy/ProxyScene.h"
#include "Hyperion/Renderer/Proxy/ProxyPrimitive.h"
#include "Hyperion/Renderer/Proxy/ProxyLight.h"
#include "Hyperion/Renderer/Proxy/ProxyCamera.h"
#include "Hyperion/Core/RenderManager.h"
#include "Hyperion/Core/ThreadManager.h"
#include "Hyperion/Core/Asset.h"
#include "Hyperion/Assets/TextureAsset.h"

/*
	Include various graphics APIs?
*/
#if HYPERION_OS_WIN32
#include "Hyperion/Renderer/DirectX11/DirectX11Graphics.h"
#endif


namespace Hyperion
{


	Renderer::Renderer( std::shared_ptr< IGraphics >& inAPI, const IRenderOutput& inOutput, const ScreenResolution& inRes, bool bVSync )
		: m_API( inAPI ), m_Output( inOutput ), m_Resolution( inRes ), m_bVSync( bVSync ), m_Scene( std::make_shared< ProxyScene >() )
	{
		HYPERION_VERIFY( m_API != nullptr, "Failed to load graphics api!" );
	}


	Renderer::~Renderer()
	{
		Shutdown();
	}


	bool Renderer::Initialize()
	{
		if( !m_API )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to initialize.. graphics api was null!" );
			return false;
		}

		// We need to setup and init our graphics api
		m_API->SetResolution( m_Resolution );
		m_API->SetVSync( m_bVSync );

		if( !m_API->Initialize( m_Output ) )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to initialize graphics API!" );
			std::terminate();
			return false;
		}

		// Initialize the scene
		m_Scene->Initialize();
		return true;
	}


	void Renderer::Shutdown()
	{
		// Shutdown the scene
		if( m_Scene )
		{
			m_Scene.reset();
		}

		// We need to shutdown the graphics API
		if( m_API )
		{
			m_API.reset();
		}
	}


	void Renderer::Frame()
	{
		// First, we need to update the proxy scene
		UpdateScene();

		// Next, we need to prepare the API for the frame
		m_API->BeginFrame();

		// TODO: Render scene

		m_API->EndFrame();
	}


	void Renderer::UpdateScene()
	{
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


	bool Renderer::AddCamera( std::shared_ptr< ProxyCamera >& inCamera )
	{
		if( !m_Scene )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to add camera.. scene was null!" );
			return false;
		}

		auto ptr = m_Scene->RemoveCamera( inCamera->GetIdentifier() );
		if( ptr )
		{
			ShutdownProxy( ptr );
		}

		if( m_Scene->AddCamera( inCamera ) )
		{
			inCamera->RenderInit();
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


	bool Renderer::RemoveCamera( uint32 inIdentifier )
	{
		if( !m_Scene )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to remove camera.. scene was null!" );
			return false;
		}

		auto ptr = m_Scene->RemoveCamera( inIdentifier );
		if( ptr )
		{
			ShutdownProxy( ptr );
			return true;
		}

		return false;
	}


	Transform3D Renderer::GetViewTransform()
	{
		if( m_Scene )
		{
			auto camera = m_Scene->GetActiveCamera();
			if( camera )
			{
				return camera->GetTransform();
			}
		}

		return Transform3D(
			Vector3D( 0.f, 0.f, 0.f ),
			Angle3D( 0.f, 0.f, 0.f ),
			Vector3D( 0.f, 0.f, 0.f )
		);
	}

	std::shared_ptr< ITexture2D > Renderer::Load2DTexture( const std::shared_ptr< RawImageData >& inData )
	{
		HYPERION_VERIFY( m_API, "API Instance was null!" );

		if( !inData )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to load 2d texture.. raw image data was null" );
			return nullptr;
		}

		Texture2DParameters params;

		params.CanCPURead	= false;
		params.Dynamic		= false;
		params.Format		= TextureFormat::RGBA_8BIT_UNORM;
		params.Height		= inData->Height;
		params.Width		= inData->Width;
		params.MipLevels	= 1;
		params.Target		= TextureBindTarget::Shader;
		params.RowDataSize	= inData->Width * 4;
		params.Data			= inData->Data.data();

		auto ret = m_API->CreateTexture2D( params );
		if( !ret )
		{
			Console::WriteLine( "[ERROR] Renderer: Failed to load 2d texture, api call returned null!" );
			return nullptr;
		}

		return ret;
	}

}