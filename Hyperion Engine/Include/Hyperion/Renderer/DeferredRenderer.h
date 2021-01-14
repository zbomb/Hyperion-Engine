/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DeferredRenderer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/GBuffer.h"


namespace Hyperion
{
	/*
	*	Forward Declarations
	*/


	class DeferredRenderer : public Renderer
	{

	protected:

		std::shared_ptr< RGBufferShader > m_GBufferShader;
		std::shared_ptr< RLightingShader > m_LightingShader;
		std::shared_ptr< RForwardShader > m_ForwardShader;

		std::shared_ptr< GBuffer > m_GBuffer;

	public:

		DeferredRenderer( GraphicsAPI inAPI, void* inOutput, const ScreenResolution& inResolution, bool bVSync );
		~DeferredRenderer();

		inline auto GetGBufferShader() const { return m_GBufferShader; }
		inline auto GetLightingShader() const { return m_LightingShader; }
		inline auto GetForwardShader() const { return m_ForwardShader; }

	protected:

		void OnShutdown();
		void OnInitialize() final;
		void RenderScene() final;
		void OnResolutionChanged( const ScreenResolution& inRes ) final;

		void PerformGBufferPass( ProxyScene& inScene );
		void PerformLightingPass( GBuffer& inBuffers );

	};

}