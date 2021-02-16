/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DeferredRenderer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DeferredRenderer.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Streaming/BasicStreamingManager.h"
#include "Hyperion/Renderer/RenderPipeline.h"
#include "Hyperion/Core/InputManager.h"
#include "Hyperion/Renderer/PostProcessFX.h"


namespace Hyperion
{

	

	DeferredRenderer::DeferredRenderer( GraphicsAPI inAPI, void* inOutput, const ScreenResolution& inResolution, bool bVSync )
		: Renderer( inAPI, inOutput, inResolution, bVSync )
	{

	}

	DeferredRenderer::~DeferredRenderer()
	{
		Shutdown();
	}


	bool DeferredRenderer::Initialize()
	{
		// Ensure super class method gets called first
		if( !Renderer::Initialize() ) { return false; }

		// Create the G-Buffer we want the renderer to use
		auto gbuffer = std::make_shared< GBuffer >( m_API, m_Resolution.Width, m_Resolution.Height );
		if( !gbuffer )
		{
			Console::WriteLine( "[ERROR] DeferredRenderer: Failed to create GBUffer!" );
			return false;
		}

		SetGBuffer( gbuffer );

		// Create our renderer pipelines
		// GBuffer Pipeline
		auto sceneVertexShader = m_API->CreateVertexShader( VertexShaderType::Scene );
		
		m_GBufferPipeline = std::make_shared< RenderPipeline >();
		m_GBufferPipeline->AttachVertexShader( sceneVertexShader );
		m_GBufferPipeline->AttachPixelShader( m_API->CreatePixelShader( PixelShaderType::GBuffer ) );
		m_GBufferPipeline->SetCollectionFlags( RENDERER_GEOMETRY_COLLECTION_FLAG_OPAQUE );
		m_GBufferPipeline->SetCollectionSource( GeometryCollectionSource::Scene );
		m_GBufferPipeline->SetRenderTarget( PipelineRenderTarget::GBuffer );
		m_GBufferPipeline->SetDepthStencilTarget( PipelineDepthStencilTarget::Screen );
		
		m_LightingPipeline = std::make_shared< RenderPipeline >();
		m_LightingPipeline->AttachVertexShader( m_API->CreateVertexShader( VertexShaderType::Screen ) );
		m_LightingPipeline->AttachPixelShader( m_API->CreatePixelShader( PixelShaderType::Lighting ) );
		m_LightingPipeline->SetCollectionSource( GeometryCollectionSource::ScreenQuad );
		m_LightingPipeline->SetRenderTarget( PipelineRenderTarget::PostProcessBuffer );
		m_LightingPipeline->EnableGBuffer();
		m_LightingPipeline->EnableViewClusters();
		m_LightingPipeline->EnableLightBuffer();
		m_LightingPipeline->DisableZBuffer();
		m_LightingPipeline->SetDepthStencilTarget( PipelineDepthStencilTarget::None );

		m_ForwardPreZPipeline = std::make_shared< RenderPipeline >();
		m_ForwardPreZPipeline->AttachVertexShader( sceneVertexShader );
		m_ForwardPreZPipeline->AttachPixelShader( m_API->CreatePixelShader( PixelShaderType::ForwardPreZ ) );
		m_ForwardPreZPipeline->SetCollectionFlags( RENDERER_GEOMETRY_COLLECTION_FLAG_TRANSLUCENT );
		m_ForwardPreZPipeline->SetCollectionSource( GeometryCollectionSource::Scene );
		m_ForwardPreZPipeline->SetRenderTarget( PipelineRenderTarget::ViewClusters );
		
		m_ForwardPipeline = std::make_shared< RenderPipeline >();
		m_ForwardPipeline->AttachVertexShader( sceneVertexShader );
		m_ForwardPipeline->AttachPixelShader( m_API->CreatePixelShader( PixelShaderType::Forward ) );
		m_ForwardPipeline->SetCollectionFlags( RENDERER_GEOMETRY_COLLECTION_FLAG_TRANSLUCENT ); // This is deprecated?
		m_ForwardPipeline->SetCollectionSource( GeometryCollectionSource::Scene );
		m_ForwardPipeline->SetRenderTarget( PipelineRenderTarget::PostProcessBuffer );
		m_ForwardPipeline->EnableViewClusters();
		m_ForwardPipeline->EnableLightBuffer();
		m_ForwardPipeline->DisableRenderTargetClearing();
		m_ForwardPipeline->DisableDepthBufferClearing();
		m_ForwardPipeline->EnableAlphaBlending();

		// Create the post-processing FX
		m_FXAA = std::make_shared< PostProcessFX >();
		m_FXAA->AttachShader( m_API->CreatePostProcessShader( PostProcessShaderType::FXAA ) );
		m_FXAA->SetRenderTarget( PostProcessRenderTarget::BackBuffer );

		// Create a 'debug floor' material
		m_FloorAsset = AssetManager::Get< MaterialAsset >( "materials/floor_material.hmat" );
		if( !m_FloorAsset )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to load debug floor material!" );
		}
		else
		{
			this->GetStreamingManager()->ReferenceTexture( m_FloorAsset->GetTexture( "base_map" ) );
			m_FloorMaterial = this->GetResourceManager()->CreateMaterial( m_FloorAsset );
			if( !m_FloorMaterial )
			{
				Console::WriteLine( "[Warning] Renderer: failed to create floor material instance!" );
			}
		}

		return true;
	}


	void DeferredRenderer::Shutdown()
	{
		m_FloorMaterial.reset();
		m_FloorAsset.reset();

		m_GBufferPipeline.reset();
		m_LightingPipeline.reset();

		Renderer::Shutdown();
	}


	void DeferredRenderer::OnResolutionChanged( const ScreenResolution& inRes )
	{

	}



	void DeferredRenderer::RenderScene()
	{
		if( !m_Scene )
		{
			Console::WriteLine( "[ERROR] DeferredRenderer: Failed to render scene.. scene/buffers were null!" );
			return;
		}

		//auto frameBegin = std::chrono::high_resolution_clock::now();

		// Update view clusters
		if( AreViewClustersDirty() )
		{
			// We need to rebuild the view clusters whenever the resolution or FOV changes
			if( !RebuildViewClusters() )
			{
				Console::WriteLine( "[ERROR] DeferredRenderer: Failed to rebuild view clusters" );
			}
		}
		else
		{
			if( !ResetViewClusters() )
			{
				Console::WriteLine( "[ERROR] DeferredRenderer: Failed to reset view clusters" );
			}
		}

		//auto endClusterReset = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > clusterResetTime = endClusterReset - frameBegin;

		// Update light buffer
		if( !RebuildLightBuffer() )
		{
			Console::WriteLine( "[ERROR] DeferredRenderer: Failed to rebuild light buffer" );
			return;
		}

		//auto endLightRebuild = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > lightRebuildTime = endLightRebuild - endClusterReset;

		// Get the 'debug floor' buffers
		std::shared_ptr< RBuffer > vertexBuffer;
		std::shared_ptr< RBuffer > indexBuffer;
		std::vector< Matrix > floorMatricies;
		m_API->GetDebugFloorQuad( vertexBuffer, indexBuffer, floorMatricies );

		// Collect scene geometry, seperated into opaque and translucent
		BatchCollector collector {};
		CollectBatches( collector );

		//auto endBatchCollect = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > batchCollectTime = endBatchCollect - endLightRebuild;

		if( m_FloorMaterial->AreTexturesLoaded() )
		{
			for( int i = 0; i < floorMatricies.size(); i++ )
			{
				collector.CollectOpaque( floorMatricies[ i ], m_FloorMaterial, indexBuffer, vertexBuffer );
			}
		}

		// Dispatch the GBuffer pass
		//auto beginAttach = std::chrono::high_resolution_clock::now();
		AttachPipeline( m_GBufferPipeline );
		//auto endAttach = std::chrono::high_resolution_clock::now();
		DispatchPipeline( collector );
		//auto endDispatch = std::chrono::high_resolution_clock::now();
		DetachPipeline();
		//auto endDetach = std::chrono::high_resolution_clock::now();

		//std::chrono::duration< double, std::nano > attachTime = endAttach - beginAttach;
		//std::chrono::duration< double, std::nano > dispatchTime = endDispatch - endAttach;
		//std::chrono::duration< double, std::nano > detachTime = endDetach - endDispatch;

		//Console::WriteLine( "====> Attach Time: ", attachTime.count(), "ns \t Dispatch Time: ", dispatchTime.count(), "ns \t Detach Time: ", detachTime.count(), "ns" );

		//auto endGBufferPass = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > gbufferTime = endGBufferPass - endBatchCollect;

		// Find active view clusters
		DispatchComputeShader( m_FindClustersShader );

		//auto endFindCluster = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > findClusterTime = endFindCluster - endGBufferPass;

		// Perform Pre-Z pass
		//AttachPipeline( m_ForwardPreZPipeline );
		//DispatchPipeline( collector );
		//DetachPipeline();

		// Perform light culling
		DispatchComputeShader( m_CullLightsShader );

		//auto endCullLights = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > cullLightTime = endCullLights - endFindCluster;

		// Perform lighting pass
		AttachPipeline( m_LightingPipeline );
		DispatchPipeline();
		DetachPipeline();

		//auto endFrame = std::chrono::high_resolution_clock::now();
		//std::chrono::duration< double, std::nano > lightingTime = endFrame - endCullLights;

		// Perform final pass
		//AttachPipeline( m_ForwardPipeline );
		//DispatchPipeline( collector );
		//DetachPipeline();

		//Console::WriteLine( "===> Build Clusters: ", clusterResetTime.count(), "ns \t Light Rebuild: ", lightRebuildTime.count(), "ns \t Batch Collection: ", batchCollectTime.count(), "ns \t ",
		//					"GBuffer Pass: ", gbufferTime.count(), "ns \t Find Clusters: ", findClusterTime.count(), "ns \t Cull Lights: ", cullLightTime.count(), "ns \t Lighting: ", lightingTime.count(), "ns" );

	}


	void DeferredRenderer::RenderPostProcessFX()
	{
		ApplyPostProcessEffect( m_FXAA );
	}

}