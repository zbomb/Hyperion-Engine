/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DeferredRenderer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DeferredRenderer.h"


namespace Hyperion
{

	

	DeferredRenderer::DeferredRenderer( GraphicsAPI inAPI, void* inOutput, const ScreenResolution& inResolution, bool bVSync )
		: Renderer( inAPI, inOutput, inResolution, bVSync )
	{

	}

	DeferredRenderer::~DeferredRenderer()
	{
		OnShutdown();
	}

	void DeferredRenderer::OnInitialize()
	{
		// Create our shaders
		m_GBufferShader		= m_API->CreateGBufferShader( SHADER_PATH_GBUFFER_PIXEL, SHADER_PATH_GBUFFER_VERTEX );
		m_LightingShader	= m_API->CreateLightingShader( SHADER_PATH_LIGHTING_PIXEL, SHADER_PATH_LIGHTING_VERTEX );
		//m_ForwardShader		= m_API->CreateForwardShader( SHADER_PATH_FORWARD_PIXEL, SHADER_PATH_FORWARD_VERTEX );

		HYPERION_VERIFY( m_GBufferShader && m_LightingShader && m_GBufferShader->IsValid() && m_LightingShader->IsValid(),
						 "[Renderer] Failed to create the sahders!" );

		m_GBuffer = std::make_shared< GBuffer >( m_API, m_Resolution.Width, m_Resolution.Height );

		HYPERION_VERIFY( m_GBuffer, "[Renderer] Failed to create the G-Buffer" );
	}


	void DeferredRenderer::OnShutdown()
	{
		m_GBuffer.reset();
		m_GBufferShader.reset();
		m_LightingShader.reset();
		m_ForwardShader.reset();
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
	}



	void DeferredRenderer::RenderScene()
	{
		if( !m_Scene || !m_GBuffer )
		{
			Console::WriteLine( "[ERROR] DeferredRenderer: Failed to render scene.. scene/buffers were null!" );
			return;
		}

		//m_API->SetRenderOutputToScreen(); // DEBUG

		// Clear the depth buffer and render target
		Color4F clearColor{ 0.f, 0.f, 0.f, 0.f };
		m_API->ClearRenderTarget( m_API->GetRenderTarget(), clearColor );
		m_API->ClearDepthStencil( m_API->GetDepthStencil(), clearColor );

		// Perform the G-Buffer pass, then the lighting pass
		PerformGBufferPass( *m_Scene );
		//PerformLightingPass( *m_GBuffer );
	}


	void DeferredRenderer::PerformGBufferPass( ProxyScene& inScene )
	{
		// Next we need to get matricies and view state
		Matrix worldMatrix, viewMatrix, projectionMatrix;

		m_API->GetViewMatrix( viewMatrix );
		m_API->GetProjectionMatrix( projectionMatrix );

		// Clear the G-Buffer and set render output
		//m_API->SetRenderOutputToGBuffer( m_GBuffer );
		m_GBuffer->Clear( m_API, Color4F( 0.f, 0.f, 0.f, 1.f ) );

		// Set the shader to the g-buffer shader
		m_API->SetShader( m_GBufferShader );
		
		for( auto it = inScene.PrimitivesBegin(); it != inScene.PrimitivesEnd(); it++ )
		{
			if( it->second )
			{
				ProxyPrimitive& primitive = *it->second.get();
				
				// Perform view frustum culling
				//if( m_API->CheckViewCull( primitive.m_Transform, primitive.GetAABB() ) )
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
								m_API->RenderGeometry( vertBuffer, indxBuffer, indxBuffer->GetCount() );
							}

						}
					}

				}
			}
		}
	}


	void DeferredRenderer::PerformLightingPass( GBuffer& inBuffers )
	{
		// First, set the render target back to the back buffer, and set the shader type
		m_API->SetShader( m_LightingShader );

		m_API->SetRenderOutputToScreen();
		m_API->DisableZBuffer();

		// Get the matricies we need
		Matrix worldMatrix, viewMatrix, projectionMatrix;
		m_API->GetWorldMatrix( worldMatrix );
		m_API->GetViewMatrix( viewMatrix );
		m_API->GetProjectionMatrix( projectionMatrix );

		m_LightingShader->UploadGBuffer( m_GBuffer );
		m_LightingShader->UploadMatrixData( worldMatrix, viewMatrix, projectionMatrix );
		
		m_API->RenderScreenGeometry();

		m_API->EnableZBuffer();

		// We need to clear the G-Buffer textures from the pixel shader, otherwise we will get an error next time we try and render
		// the G-Buffer, beause it will be bound to both the input and output simultaneously
		m_LightingShader->ClearGBufferResources();

	}

}