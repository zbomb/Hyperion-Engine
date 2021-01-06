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

		DeferredRenderer( GraphicsAPI inAPI, void* inOutput, const ScreenResolution& inResolution, bool bVSync );
		~DeferredRenderer();

	protected:

		void OnInitialize() final;
		void OnShutdown() final;
		void RenderScene() final;

		void PerformGBufferPass( ProxyScene& inScene );

	};

}