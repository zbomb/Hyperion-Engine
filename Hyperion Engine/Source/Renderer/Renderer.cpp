/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Renderer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/Proxy/ProxyScene.h"
#include "Hyperion/Core/Engine.h"
#include <iostream>



namespace Hyperion
{

	Renderer::Renderer()
		: m_isRunning( false )
	{
	}


	Renderer::~Renderer()
	{
		// Shutdown the scene
		if( m_Scene )
		{
			m_Scene->Shutdown();
			m_Scene.reset();
		}

		// Shutdown the thread
		if( m_renderThread )
		{
			m_renderThread->Stop();
		}

		auto& eng = Engine::GetInstance();
		auto tm = eng.GetThreadManager();

		if( tm )
			tm->DestroyThread( THREAD_RENDERER );

		m_isRunning = false;
	}


	bool Renderer::Start()
	{
		auto& eng = Engine::GetInstance();
		auto tm = eng.GetThreadManager();

		if( !tm )
		{
			std::cout << "[ERROR] Renderer: Couldnt initialize.. thread manager was null\n";
			return false;
		}

		// Check if were already running
		if( m_isRunning )
		{
			std::cout << "[ERROR] Renderer: Couldnt initialize.. already running\n";
			return false;
		}

		// If the render thread is still running.. stop it
		tm->DestroyThread( THREAD_RENDERER );

		// Create our new render thread
		TickedThreadParameters params;

		params.Identifier			= THREAD_RENDERER;
		params.AllowTasks			= true;
		params.Deviation			= 0.f;
		params.Frequency			= 0.f;
		params.MinimumTasksPerTick	= 3;
		params.MaximumTasksPerTick	= 0;
		params.StartAutomatically	= true;
		
		params.InitFunction			= std::bind( &Renderer::PerformInit, this );
		params.TickFunction			= std::bind( &Renderer::PerformTick, this );
		params.ShutdownFunction		= std::bind( &Renderer::PerformShutdown, this );

		m_renderThread = tm->CreateThread( params );
		if( !m_renderThread )
		{
			std::cout << "[ERROR] Renderer: Couldnt initialize.. failed to create render thread\n";
			return false;
		}

		m_isRunning			= true;
		__gRenderThreadId	= m_renderThread->GetSystemIdentifier();

		return true;
	}


	void Renderer::Stop()
	{
		if( !m_isRunning && !m_renderThread )
		{
			std::cout << "[WARNING] Renderer: Attempt to stop renderer.. but it wasnt running!\n";
			return;
		}

		m_isRunning = false;
		if( m_renderThread )
		{
			m_renderThread->Stop();
		}

		auto& eng = Engine::GetInstance();
		auto tm = eng.GetThreadManager();

		if( !tm )
		{
			std::cout << "[ERROR] Renderer: Failed to properly shutdown.. couldnt access thread manager to destroy thread!\n";
			return;
		}

		tm->DestroyThread( THREAD_RENDERER );

	}


	void Renderer::PerformInit()
	{
		std::cout << "[DEBUG] Renderer: Performing init...\n";

		// Reset the scene
		if( m_Scene )
		{
			m_Scene->Shutdown();
			m_Scene.reset();
		}

		m_Scene = std::make_shared< ProxyScene >();
		m_Scene->Initialize();

		// Initialize derived renderer
		if( !Init() )
		{
			std::cout << "[ERROR] Renderer: Failed to initialize!\n";
		}
	}

	void Renderer::PerformTick()
	{
		UpdateProxyScene();
		Frame();
	}

	void Renderer::PerformShutdown()
	{
		std::cout << "[DEBUG] Renderer: Performing shutdown...\n";

		if( m_Scene )
		{
			m_Scene->Shutdown();
			m_Scene.reset();
		}

		// Shutdown derived renderer
		Shutdown();
	}

	void Renderer::UpdateProxyScene()
	{
		// TODO
	}

	void Renderer::UpdateParameters( const RendererParameters& inParameters )
	{
		// Check if this update requires a restart of the renderer
		bool bRequireRestart = false;

		if( m_isRunning &&
			inParameters.OutputWindow != m_Params.OutputWindow ||
			inParameters.ScreenWidth != m_Params.ScreenWidth ||
			inParameters.ScreenHeight != m_Params.ScreenHeight )
		{
			bRequireRestart = true;
		}

		// TODO: Restart for certain parameters?
		if( bRequireRestart )
		{
			Stop();
			m_Params = inParameters;
			
			if( !Start() )
			{
				std::cout << "[ERROR] Renderer: Failed to restart after parameter update!\n";
			}
		}
		else
		{
			m_Params = inParameters;
		}
	}

	RendererParameters Renderer::GetParameters()
	{
		return m_Params;
	}
}