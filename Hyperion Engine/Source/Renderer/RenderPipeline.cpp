/*==================================================================================================
	Hyperion Engine
	Source/Renderer/RenderPipeline.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/RenderPipeline.h"
#include "Hyperion/Renderer/Resources/RShader.h"


namespace Hyperion
{

	RenderPipeline::RenderPipeline()
		: m_CollectionFlags( RENDERER_GEOMETRY_COLLECTION_FLAG_NONE ), m_CollectionSource( GeometryCollectionSource::Scene ), m_bUseZBuffer( true ),
		m_RenderTarget( PipelineRenderTarget::Screen ), m_bUseGBuffer( false ), m_bUseLightBuffer( false ), m_bUseViewClusters( false ),
		m_bClearRenderTargets( true ), m_bClearDepthBuffers( true ), m_StencilTarget( PipelineDepthStencilTarget::Screen ), m_bAlphaBlending( false )
	{

	}

	
	RenderPipeline::~RenderPipeline()
	{
		m_VertexShader.reset();
		m_GeometryShader.reset();
		m_PixelShader.reset();
	}


	bool RenderPipeline::IsValid() const
	{
		return m_VertexShader && m_PixelShader && m_VertexShader->IsValid() && m_PixelShader->IsValid();
	}


	bool RenderPipeline::AttachVertexShader( const std::shared_ptr< RVertexShader >& inShader )
	{
		if( !inShader || !inShader->IsValid() )
		{
			Console::WriteLine( "[ERROR] Pipeline: Failed to attach vertex shader, it was null/invalid" );
			return false;
		}

		m_VertexShader = inShader;
		return true;
	}


	bool RenderPipeline::AttachGeometryShader( const std::shared_ptr< RGeometryShader >& inShader )
	{
		if( !inShader || !inShader->IsValid() )
		{
			Console::WriteLine( "[ERROR] Pipeline: Failed to attach geometry shader, it was null/invalid" );
			return false;
		}

		m_GeometryShader = inShader;
		return true;
	}


	bool RenderPipeline::AttachPixelShader( const std::shared_ptr< RPixelShader >& inShader )
	{
		if( !inShader || !inShader->IsValid() )
		{
			Console::WriteLine( "[ERROR] Pipeline: Failed to attach pixel shader, it was null/invalid" );
			return false;
		}

		m_PixelShader = inShader;
		return true;
	}


	void RenderPipeline::SetCollectionFlags( uint32 inFlags )
	{
		m_CollectionFlags = inFlags;
	}


	void RenderPipeline::SetCollectionSource( GeometryCollectionSource inSource )
	{
		m_CollectionSource = inSource;
	}


	void RenderPipeline::SetRenderTarget( PipelineRenderTarget inTarget )
	{
		m_RenderTarget = inTarget;
	}


	void RenderPipeline::SetDepthStencilTarget( PipelineDepthStencilTarget inTarget )
	{
		m_StencilTarget = inTarget;
	}

}