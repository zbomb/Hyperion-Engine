/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DeferredRenderer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DeferredRenderer.h"


namespace Hyperion
{

	

	DeferredRenderer::DeferredRenderer( std::shared_ptr<IGraphics>& inAPI, const IRenderOutput& inOutput, const ScreenResolution& inResolution, bool bVSync )
		: Renderer( inAPI, inOutput, inResolution, bVSync )
	{

	}


	void DeferredRenderer::OnInitialize()
	{

	}


	void DeferredRenderer::OnShutdown()
	{

	}


	void DeferredRenderer::RenderScene()
	{
		if( m_Scene )
		{
			PerformGBufferPass( *m_Scene );
		}
		else
		{
			// Write out error in efficient way?
		}
	}


	void DeferredRenderer::PerformGBufferPass( ProxyScene& inScene )
	{
		// How to render to the G-Buffer?
		// Lets learn... Im assuming on initialize we have to create the G-Buffer, each frame, clear it before rendering? (maybe not)
		// and we also need some code to set the render target to the G-Buffer


	}

}