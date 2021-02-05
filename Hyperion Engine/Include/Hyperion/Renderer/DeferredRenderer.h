/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DeferredRenderer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/GBuffer.h"
#include "Hyperion/Renderer/ViewClusters.h"
#include "Hyperion/Renderer/LightBuffer.h"


namespace Hyperion
{
	/*
	*	Forward Declarations
	*/


	class DeferredRenderer : public Renderer
	{

	protected:

		std::shared_ptr< RenderPipeline > m_GBufferPipeline;
		std::shared_ptr< RenderPipeline > m_LightingPipeline;
		std::shared_ptr< RenderPipeline > m_ForwardPreZPipeline;
		std::shared_ptr< RenderPipeline > m_ForwardPipeline;

		// DEBUG
		std::shared_ptr< MaterialAsset > m_FloorAsset;
		std::shared_ptr< RMaterial > m_FloorMaterial;

	public:

		DeferredRenderer( GraphicsAPI inAPI, void* inOutput, const ScreenResolution& inResolution, bool bVSync );
		~DeferredRenderer();

		bool Initialize() override;
		void Shutdown() override;

	protected:

		void RenderScene() final;
		void OnResolutionChanged( const ScreenResolution& inRes ) final;

	};

}