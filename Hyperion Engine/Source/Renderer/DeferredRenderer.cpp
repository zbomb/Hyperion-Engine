/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DeferredRenderer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DeferredRenderer.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Streaming/BasicStreamingManager.h"


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

		// Create our shaders
		m_GBufferShader		= m_API->CreateGBufferShader( SHADER_PATH_GBUFFER_PIXEL, SHADER_PATH_GBUFFER_VERTEX );
		m_LightingShader	= m_API->CreateLightingShader( SHADER_PATH_LIGHTING_PIXEL, SHADER_PATH_LIGHTING_VERTEX );
		//m_ForwardShader		= m_API->CreateForwardShader( SHADER_PATH_FORWARD_PIXEL, SHADER_PATH_FORWARD_VERTEX );
		m_BuildClusterShader = m_API->CreateBuildClusterShader( SHADER_PATH_COMPUTE_BUILD_CLUSTERS );
		m_CompressClustersShader = m_API->CreateCompressClustersShader( SHADER_PATH_COMPUTE_COMPRESS_CLUSTERS );

		if( !m_GBufferShader || !m_LightingShader || !m_GBufferShader->IsValid() || !m_LightingShader->IsValid() || !m_BuildClusterShader || !m_CompressClustersShader )
		{
			Console::WriteLine( "[ERROR] DeferredRenderer: Failed to create shaders!" );
			return false;
		}

		m_GBuffer = std::make_shared< GBuffer >( m_API, m_Resolution.Width, m_Resolution.Height );
		if( !m_GBuffer )
		{
			Console::WriteLine( "[ERROR] DeferredRenderer: Failed to create GBUffer!" );
			return false;
		}

		m_Clusters = m_API->CreateViewClusters();
		if( !m_Clusters )
		{
			Console::WriteLine( "[ERROR] DeferredRenderer: Failed to create view clusters!" );
			return false;
		}

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
		m_Clusters.reset();
		m_GBuffer.reset();
		m_BuildClusterShader.reset();
		m_CompressClustersShader.reset();
		m_GBufferShader.reset();
		m_LightingShader.reset();
		m_ForwardShader.reset();

		Renderer::Shutdown();
	}


	void DeferredRenderer::OnResolutionChanged( const ScreenResolution& inRes )
	{
		// We need to update the size of the G-Buffer to match the new screen resolution
		if( m_GBuffer )
		{
			if( !m_GBuffer->UpdateDimensions( m_API, inRes.Width, inRes.Height ) )
			{
				Console::WriteLine( "[ERROR] DeferredRenderer: Failed to update the size of the G-Buffer!" );
			}
		}

		if( m_Clusters )
		{
			m_Clusters->MarkDirty();
		}
	}



	void DeferredRenderer::RenderScene()
	{
		if( !m_Scene || !m_GBuffer || !m_Clusters )
		{
			Console::WriteLine( "[ERROR] DeferredRenderer: Failed to render scene.. scene/buffers were null!" );
			return;
		}

		//if( m_Clusters->IsDirty() )
		{
			BuildClusters();
			m_Clusters->MarkClean();
		}

		// Clear the depth buffer and render target
		Color4F clearColor{ 0.f, 0.f, 0.f, 0.f };
		m_API->ClearRenderTarget( m_API->GetRenderTarget(), clearColor );
		m_API->ClearDepthStencil( m_API->GetDepthStencil(), clearColor );

		PerformGBufferPass();
		CompressClusters();
		PerformLightingPass();
	}


	void DeferredRenderer::BuildClusters()
	{
		Matrix projectionMatrix;
		m_API->GetProjectionMatrix( projectionMatrix );

		auto screenRes = GetResolutionUnsafe();

		m_BuildClusterShader->Attach();
		m_BuildClusterShader->UploadViewInfo( projectionMatrix, screenRes, SCREEN_NEAR, SCREEN_FAR );
		m_BuildClusterShader->Dispatch( m_Clusters );
		m_BuildClusterShader->Detach();
	}


	void DeferredRenderer::CompressClusters()
	{
		m_CompressClustersShader->Attach();
		m_CompressClustersShader->Dispatch( m_Clusters );
		m_CompressClustersShader->Detach();
	}


	void DeferredRenderer::PerformGBufferPass()
	{
		Matrix worldMatrix, viewMatrix, projectionMatrix;

		m_API->GetViewMatrix( viewMatrix );
		m_API->GetProjectionMatrix( projectionMatrix );

		// Clear the G-Buffer and set render output
		m_API->SetRenderOutputToGBuffer( m_GBuffer, m_Clusters );
		m_GBuffer->Clear( m_API, Color4F( 0.f, 0.f, 0.f, 1.f ) );

		// Set the shader to the g-buffer shader
		m_API->SetShader( m_GBufferShader );

		// DEBUG
		// Draw the debug floor
		if( m_FloorMaterial && m_FloorMaterial->AreTexturesLoaded() )
		{
			m_API->GetWorldMatrix( worldMatrix );
			m_GBufferShader->UploadMatrixData( worldMatrix, viewMatrix, projectionMatrix );
			m_GBufferShader->UploadMaterial( m_FloorMaterial );
			m_API->RenderDebugFloor();
		}
		
		
		for( auto it = m_Scene->PrimitivesBegin(); it != m_Scene->PrimitivesEnd(); it++ )
		{
			if( it->second )
			{
				ProxyPrimitive& primitive = *it->second.get();
				
				// Perform view frustum culling
				if( m_API->CheckViewCull( primitive.m_Transform, primitive.GetAABB() ) )
				{
					// Calculate the world matrix
					Matrix worldMatrix;
					m_API->GetWorldMatrix( primitive.m_Transform, worldMatrix );

					auto activeLOD = primitive.GetActiveLOD();

					// Get the buffers and materails we need to render this primitive
					std::vector< std::tuple< std::shared_ptr< RBuffer >, std::shared_ptr< RBuffer >, std::shared_ptr< RMaterial > > > renderData;
					if( primitive.GetLODResources( activeLOD, renderData ) )
					{						

						// Upload matricies
						m_GBufferShader->UploadMatrixData( worldMatrix, viewMatrix, projectionMatrix );

						// Loop through each subobject and render them to the g-buffer
						for( auto bit = renderData.begin(); bit != renderData.end(); bit++ )
						{
							auto& vertBuffer	= std::get< 0 >( *bit );
							auto& indxBuffer	= std::get< 1 >( *bit );
							auto& matPtr		= std::get< 2 >( *bit );

							if( vertBuffer && indxBuffer && matPtr )
							{
								m_GBufferShader->UploadMaterial( matPtr );
								m_API->RenderMesh( vertBuffer, indxBuffer, indxBuffer->GetCount() );
							}

						}
					}

				}
			}
		}

		m_API->DetachGBuffer();
	}


	void DeferredRenderer::PerformLightingPass()
	{
		// First, set the render target back to the back buffer, and set the shader type
		m_API->SetShader( m_LightingShader );

		m_API->SetRenderOutputToScreen();
		m_API->DisableZBuffer();

		// Get the matricies we need
		Matrix worldMatrix, viewMatrix, projectionMatrix, gbufferView, gbufferProjection;
		m_API->GetWorldMatrix( worldMatrix );
		m_API->GetScreenViewMatrix( viewMatrix );
		m_API->GetOrthoMatrix( projectionMatrix );
		m_API->GetViewMatrix( gbufferView );
		m_API->GetProjectionMatrix( gbufferProjection );

		m_LightingShader->UploadGBuffer( m_GBuffer );
		m_LightingShader->UploadMatrixData( worldMatrix, viewMatrix, projectionMatrix );
		m_LightingShader->UploadGBufferData( gbufferView, gbufferProjection );

		// We need to get a list of lights
		std::vector< std::shared_ptr< ProxyLight > > lightList;
		for( auto it = m_Scene->LightsBegin(); it != m_Scene->LightsEnd(); it++ )
		{
			if( it->second )
			{
				lightList.push_back( it->second );
			}
		}

		// Get ambient light info
		Color3F ambientColor( 1.f, 1.f, 1.f );
		float ambientIntensity = 0.25f;

		m_LightingShader->UploadLighting( ambientColor, ambientIntensity, lightList );
		
		m_API->RenderScreenMesh();

		m_API->EnableZBuffer();

		// We need to clear the G-Buffer textures from the pixel shader, otherwise we will get an error next time we try and render
		// the G-Buffer, beause it will be bound to both the input and output simultaneously
		m_LightingShader->ClearResources();
	}

}