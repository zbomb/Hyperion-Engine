/*==================================================================================================
	Hyperion Engine
	Source/Renderer/RenderMarshal.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/RenderMarshal.h"
#include "Hyperion/Renderer/Renderer.h"
#include <iostream>


namespace Hyperion
{


	void RenderMarshal::InitThread()
	{
		std::cout << "[DEBUG] RenderMarshal: Thread init....\n";
	}

	void RenderMarshal::TickThread()
	{
		auto lRenderer = m_RendererRef.lock();
		if( lRenderer )
		{
			// Sync with main render thread for the main pass
			lRenderer->m_MainPassBarrier.Wait();
			PerformUpdatePass();

			// Sync with main render thread for the cleanup pass
			lRenderer->m_UpdateBarrier.Wait();
			PerformCleanupPass();
		}
	}

	void RenderMarshal::ShutdownThread()
	{
		std::cout << "[DEBUG] Render Marshal: Thread shutdown....\n";
	}


	void RenderMarshal::PerformUpdatePass()
	{

	}

	void RenderMarshal::PerformCleanupPass()
	{

	}

}