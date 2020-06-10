/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DeferredRenderer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/Renderer.h"


namespace Hyperion
{

	class DeferredRenderer : public Renderer
	{

	public:

		DeferredRenderer( std::shared_ptr< IGraphics >& inAPI, const IRenderOutput& inOutput, const ScreenResolution& inResolution, bool bVSync );

	protected:

		void OnInitialize() final;
		void OnShutdown() final;
		void RenderScene() final;

		void PerformGBufferPass( ProxyScene& inScene );

	};

}