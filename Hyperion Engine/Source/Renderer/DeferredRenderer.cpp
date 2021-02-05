/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DeferredRenderer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DeferredRenderer.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Streaming/BasicStreamingManager.h"
#include "Hyperion/Renderer/RenderPipeline.h"


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
		m_LightingPipeline->SetRenderTarget( PipelineRenderTarget::Screen );
		m_LightingPipeline->EnableGBuffer();
		m_LightingPipeline->EnableViewClusters();
		m_LightingPipeline->EnableLightBuffer();
		m_LightingPipeline->DisableZBuffer();
		m_LightingPipeline->SetDepthStencilTarget( PipelineDepthStencilTarget::None );

		/*
		m_ForwardPreZPipeline = std::make_shared< RenderPipeline >();
		m_ForwardPreZPipeline->AttachVertexShader( sceneVertexShader );
		m_ForwardPreZPipeline->AttachPixelShader( m_API->CreatePixelShader( PixelShaderType::ForwardPreZ ) );
		m_ForwardPreZPipeline->SetCollectionFlags( RENDERER_GEOMETRY_COLLECTION_FLAG_TRANSLUCENT );
		m_ForwardPreZPipeline->SetCollectionSource( GeometryCollectionSource::Scene );
		m_ForwardPreZPipeline->SetRenderTarget( PipelineRenderTarget::ViewClusters );
		*/

		m_ForwardPipeline = std::make_shared< RenderPipeline >();
		m_ForwardPipeline->AttachVertexShader( sceneVertexShader );
		m_ForwardPipeline->AttachPixelShader( m_API->CreatePixelShader( PixelShaderType::Forward ) );
		m_ForwardPipeline->SetCollectionFlags( RENDERER_GEOMETRY_COLLECTION_FLAG_TRANSLUCENT ); // This is deprecated?
		m_ForwardPipeline->SetCollectionSource( GeometryCollectionSource::Scene );
		m_ForwardPipeline->SetRenderTarget( PipelineRenderTarget::Screen );
		m_ForwardPipeline->EnableViewClusters();
		m_ForwardPipeline->EnableLightBuffer();
		m_ForwardPipeline->DisableRenderTargetClearing();
		m_ForwardPipeline->DisableDepthBufferClearing();
		m_ForwardPipeline->EnableAlphaBlending();

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
			/*
			if( !ResetViewClusters() )
			{
				Console::WriteLine( "[ERROR] DeferredRenderer: Failed to reset view clusters" );
			}
			*/
		}

		// Update light buffer
		if( !RebuildLightBuffer() )
		{
			Console::WriteLine( "[ERROR] DeferredRenderer: Failed to rebuild light buffer" );
			return;
		}

		// Get the 'debug floor' buffers
		Matrix floorWorldMatrix = Matrix::GetIdentity();
		std::shared_ptr< RBuffer > vertexBuffer;
		std::shared_ptr< RBuffer > indexBuffer;
		m_API->GetDebugFloorQuad( vertexBuffer, indexBuffer );

		// Collect scene geometry, seperated into opaque and translucent
		BatchCollector opaqueCollector {};
		CollectBatches( opaqueCollector, RENDERER_GEOMETRY_COLLECTION_FLAG_OPAQUE );

		BatchCollector translucentCollector {};
		//CollectBatches( translucentCollector, RENDERER_GEOMETRY_COLLECTION_FLAG_TRANSLUCENT );

		if( m_FloorMaterial->AreTexturesLoaded() )
		{
			opaqueCollector.CollectBatch( floorWorldMatrix, indexBuffer, vertexBuffer, m_FloorMaterial );
		}

		// Dispatch the GBuffer pass
		AttachPipeline( m_GBufferPipeline );
		DispatchPipeline( opaqueCollector );
		DetachPipeline();

		// Find active view clusters
		DispatchComputeShader( m_FindClustersShader );

		// Perform Pre-Z pass
		//AttachPipeline( m_ForwardPreZPipeline );
		//DispatchPipeline( translucentCollector );
		//DetachPipeline();

		// Perform light culling
		DispatchComputeShader( m_CullLightsShader );

		// Perform lighting pass
		AttachPipeline( m_LightingPipeline );
		DispatchPipeline();
		DetachPipeline();

		// Perform final pass
		//AttachPipeline( m_ForwardPipeline );
		//DispatchPipeline( translucentCollector );
		//DetachPipeline();

	}

}