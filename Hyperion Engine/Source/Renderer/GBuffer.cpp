/*==================================================================================================
	Hyperion Engine
	Source/Renderer/GBuffer.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/GBuffer.h"
#include "Hyperion/Renderer/IGraphics.h"
#include "Hyperion/Renderer/Resources/RTexture.h"
#include "Hyperion/Renderer/Resources/RRenderTarget.h"



namespace Hyperion
{

	GBuffer::GBuffer( const std::shared_ptr< IGraphics >& inAPI, uint32 inWidth, uint32 inHeight )
		: m_Width( 0 ), m_Height( 0 )
	{
		UpdateDimensions( inAPI, inWidth, inHeight );
	}


	GBuffer::~GBuffer()
	{
		Shutdown();
	}


	void GBuffer::Shutdown()
	{
		m_DiffuseRoughnessTarget.reset();
		m_DiffuseRoughnessTexture.reset();
		m_NormalDepthTarget.reset();
		m_NormalDepthTexture.reset();
		m_SpecularTarget.reset();
		m_SpecularTexture.reset();

		m_Width		= 0;
		m_Height	= 0;
	}


	bool GBuffer::IsValid() const
	{
		return m_DiffuseRoughnessTarget && m_DiffuseRoughnessTexture &&
			m_NormalDepthTarget && m_NormalDepthTexture && m_SpecularTarget && m_SpecularTexture;
	}


	void GBuffer::ClearRenderTargets( const std::shared_ptr< IGraphics >& inAPI )
	{
		HYPERION_VERIFY( inAPI, "[GBuffer] API was null!" );

		/*
		if( m_DiffuseRoughnessTarget )	{ inAPI->ClearRenderTarget( m_DiffuseRoughnessTarget ); }
		if( m_NormalDepthTarget )		{ inAPI->ClearRenderTarget( m_NormalDepthTarget, 0.f, 0.f, 0.f, SCREEN_FAR + 1.f ); }
		if( m_SpecularTarget )			{ inAPI->ClearRenderTarget( m_SpecularTarget ); }
		*/
	}

	void GBuffer::ClearDepthBuffer( const std::shared_ptr< IGraphics >& inAPI, const Color4F& inColor )
	{
		HYPERION_VERIFY( inAPI, "[GBuffer] API was null" );

		//if( m_DepthStencil ) { inAPI->ClearDepthStencil( m_DepthStencil, inColor ); }
	}


	bool GBuffer::UpdateDimensions( const std::shared_ptr< IGraphics >& inAPI, uint32 inWidth, uint32 inHeight )
	{
		return true;

		/*
		if( !inAPI )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to update G-Buffer dimensions, API was null" );
			return false;
		}
		else if( inWidth == 0 || inHeight == 0 )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to update G-Buffer dimensions, dimensions were null" );
			return false;
		}

		// Shutdown old resources
		Shutdown();

		// The G-Buffer contains 3 RGBA textures
		// Texture 1: Diffuse color (RGB) + Roughness (A) [Will be implemented in the future]
		// Texture 2: Normal (RGB) + Depth (A)
		// Texture 3: Specular (RGB) + Unused (A)
		TextureParameters params;

		params.Format			= TextureFormat::RGBA_32BIT_FLOAT;
		params.Width			= inWidth;
		params.Height			= inHeight;
		params.bDynamic			= false;
		params.BindTargets		= RENDERER_TEXTURE_BIND_FLAG_SHADER | RENDERER_TEXTURE_BIND_FLAG_RENDER;
		params.bCPURead			= false;
		params.Depth			= 1;
		params.bAutogenMips		= false;

		m_DiffuseRoughnessTexture	= inAPI->CreateTexture2D( params );
		m_NormalDepthTexture		= inAPI->CreateTexture2D( params );
		m_SpecularTexture			= inAPI->CreateTexture2D( params );

		if( !m_DiffuseRoughnessTexture || !m_NormalDepthTexture || !m_SpecularTexture ||
			!m_DiffuseRoughnessTexture->IsValid() || !m_NormalDepthTexture->IsValid() || !m_SpecularTexture->IsValid() )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to create G-Buffer! The textures couldnt be created" );

			m_DiffuseRoughnessTexture.reset();
			m_NormalDepthTexture.reset();
			m_SpecularTexture.reset();

			return false;
		}

		// Create render target views
		m_DiffuseRoughnessTarget	= inAPI->CreateRenderTarget( m_DiffuseRoughnessTexture );
		m_NormalDepthTarget			= inAPI->CreateRenderTarget( m_NormalDepthTexture );
		m_SpecularTarget			= inAPI->CreateRenderTarget( m_SpecularTexture );

		if( !m_DiffuseRoughnessTarget || !m_NormalDepthTarget || !m_SpecularTarget ||
			!m_DiffuseRoughnessTarget->IsValid() || !m_NormalDepthTarget->IsValid() || !m_SpecularTarget->IsValid() )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to create G-Buffer! The render targets couldnt be created" );
			Shutdown();

			return false;
		}

		// Setup the depth stencil
		m_DepthStencil = inAPI->CreateDepthStencil( inWidth, inHeight );
		if( !m_DepthStencil || !m_DepthStencil->IsValid() )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to create G-Buffer! The depth stencil couldnt be created" );
			Shutdown();

			return false;
		}

		m_Width		= inWidth;
		m_Height	= inHeight;

		return true;
		*/
	}


}