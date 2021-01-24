/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DeferredRenderer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/GBuffer.h"
#include "Hyperion/Renderer/ViewClusters.h"


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
		std::shared_ptr< RBuildClusterShader > m_BuildClusterShader;
		std::shared_ptr< RCompressClustersShader > m_CompressClustersShader;

		std::shared_ptr< GBuffer > m_GBuffer;
		std::shared_ptr< RViewClusters > m_Clusters;

		// DEBUG
		std::shared_ptr< MaterialAsset > m_FloorAsset;
		std::shared_ptr< RMaterial > m_FloorMaterial;

	public:

		DeferredRenderer( GraphicsAPI inAPI, void* inOutput, const ScreenResolution& inResolution, bool bVSync );
		~DeferredRenderer();

		inline auto GetGBufferShader() const { return m_GBufferShader; }
		inline auto GetLightingShader() const { return m_LightingShader; }
		inline auto GetForwardShader() const { return m_ForwardShader; }

		bool Initialize() override;
		void Shutdown() override;

	protected:

		void RenderScene() final;
		void OnResolutionChanged( const ScreenResolution& inRes ) final;

		void BuildClusters();
		void CompressClusters();
		void PerformGBufferPass();
		void PerformLightingPass();

	};

}