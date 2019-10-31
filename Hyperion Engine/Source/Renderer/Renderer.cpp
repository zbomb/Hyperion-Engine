/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Renderer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/Proxy/ProxyScene.h"
#include <iostream>



namespace Hyperion
{

	Renderer::Renderer()
		: m_MainPassBarrier( 2 ), m_UpdateBarrier( 2 )
	{

	}

	Renderer::~Renderer()
	{
		if( m_Scene )
		{
			m_Scene->Shutdown();
			m_Scene.reset();
		}
	}

	void Renderer::InitThread()
	{
		std::cout << "[DEBUG] Renderer: Initializing thread...\n";

		// Create the proxy scene, and initialize it
		if( m_Scene )
		{
			m_Scene->Shutdown();
			m_Scene.reset();
		}

		m_Scene = std::make_shared< ProxyScene >();
		m_Scene->Initialize();
	}

	void Renderer::TickThread()
	{
		// Sync with Marshal thread for the main pass
		m_MainPassBarrier.Wait();
		PerformRenderPass();

		// Sync with Marshal thread for update pass
		m_UpdateBarrier.Wait();
		ApplyProxyUpdates();
	}

	void Renderer::ShutdownThread()
	{
		std::cout << "[DEBUG] Renderer: Shutting down thread....\n";

		if( m_Scene )
		{
			m_Scene->Shutdown();
			m_Scene.reset();
		}
	}

	void Renderer::PerformRenderPass()
	{

	}

	void Renderer::ApplyProxyUpdates()
	{

	}
}