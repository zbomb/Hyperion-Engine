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
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Core/Async.h"
#include <iostream>



namespace Hyperion
{

	/*
		Static Definitions
	*/
	std::shared_ptr< Thread > TRenderSystem::m_Thread;
	std::shared_ptr< Renderer > TRenderSystem::m_Renderer;
	RenderSettings TRenderSystem::m_CurrentSettings;
	std::atomic< bool > TRenderSystem::m_bIsRunning;


	/*
		Render Thread
	*/
	bool TRenderSystem::Stop()
	{
		if( !m_Thread )
		{
			std::cout << "[ERROR] Render Thread: Attempt to stop renderer.. but the render thread is not running!\n";
			return false;
		}

		// Stop and destroy the thread
		m_Thread->Stop();

		auto threadManager = Engine::GetInstance().GetThreadManager();
		if( !threadManager )
		{
			std::cout << "[ERROR] Render Thread: Failed to completley stop renderer.. couldnt access the thread manager to delete the stopped thread!\n";
			m_Thread.reset();

			return false;
		}

		threadManager->DestroyThread( THREAD_RENDERER );
		return true;
	}


	bool TRenderSystem::IsRunning()
	{
		return m_bIsRunning;
	}


	void TRenderSystem::Init()
	{
		HYPERION_VERIFY( m_Renderer, "Failed to init render system.. renderer instance was null!" );
		std::cout << "[STATE] Render Thread Initializing...\n";

		// We need to perform initialization of the renderer and the graphics API
		if( !m_Renderer->Initialize() )
		{
			std::cout << "[ERROR] Render System: Failed to initialize! Shutting down...\n";
			Stop();
			return;
		}

		m_bIsRunning = true;
	}


	void TRenderSystem::Shutdown()
	{
		HYPERION_VERIFY( m_Renderer, "Failed to shutdown render system.. render instance was null" );
		std::cout << "[STATE] Render Thread Shutting Down...\n";

		m_bIsRunning = false;

		m_Renderer->Shutdown();
		m_Renderer.reset();
	}


	void TRenderSystem::Tick()
	{
		HYPERION_VERIFY( m_Renderer, "Failed to tick renderer.. instance was null!" );
		
		m_Renderer->Frame();
	}


	bool TRenderSystem::ValidateSettings( const RenderSettings& inSettings )
	{
		return true;
	}

	const RenderSettings& TRenderSystem::GetCurrentSettings()
	{
		return m_CurrentSettings;
	}


	void TRenderSystem::AddCommand( std::unique_ptr< RenderCommandBase >&& inCommand )
	{
		if( m_Renderer )
		{
			m_Renderer->m_Commands.Push( std::move( inCommand ) );
		}
	}




	Renderer::Renderer( const IRenderOutput& startingOutput, const RenderSettings& startingSettings, const std::shared_ptr< IGraphics >& inAPI )
		: m_Output( startingOutput ), m_bVSync( startingSettings.bVSync ), m_Resolution( startingSettings.resolution ), m_API( inAPI ), m_Scene( std::make_shared< ProxyScene >() )
	{
		// This should already be validate by the TRenderSystem before passing it along
		HYPERION_VERIFY( m_Output, "Attempt to start renderer with invalid output device" );
		HYPERION_VERIFY( m_API, "Attempt to start renderer with invalid graphics API" );
	}


	Renderer::~Renderer()
	{

	}


	bool Renderer::Initialize()
	{
		// We need to initialize the graphics API
		// First, set resolution & vsync
		m_API->SetResolution( m_Resolution );
		m_API->SetVSync( m_bVSync );

		if( !m_API->Initialize( m_Output ) )
		{
			std::cout << "[ERROR] Renderer: Failed to initiliaze graphics api! Crashing game....\n";
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
		m_Scene->Shutdown();
		m_Scene.reset();

		// We need to shutdown the graphics API
		m_API->Shutdown();
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
		}
	}


}