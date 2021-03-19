/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/GBuffer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Library/Color.h"


namespace Hyperion
{
	/*
	*	Forward Declarations
	*/
	class IGraphics;
	class RTexture2D;
	class RRenderTarget;


	class GBuffer
	{

	protected:

		std::shared_ptr< RTexture2D > m_DiffuseRoughnessTexture;
		std::shared_ptr< RTexture2D > m_NormalDepthTexture;
		std::shared_ptr< RTexture2D > m_SpecularTexture;

		std::shared_ptr< RRenderTarget > m_DiffuseRoughnessTarget;
		std::shared_ptr< RRenderTarget > m_NormalDepthTarget;
		std::shared_ptr< RRenderTarget > m_SpecularTarget;

		uint32 m_Width, m_Height;

	public:

		~GBuffer();
		GBuffer( const std::shared_ptr< IGraphics >& inAPI, uint32 inWidth, uint32 inHeight );

		void Shutdown();
		bool IsValid() const;

		void ClearRenderTargets( const std::shared_ptr< IGraphics >& inAPI );
		void ClearDepthBuffer( const std::shared_ptr< IGraphics >& inAPI, const Color4F& inColor );

		inline std::shared_ptr< RTexture2D > GetDiffuseRoughnessTexture() const { return m_DiffuseRoughnessTexture; }
		inline std::shared_ptr< RTexture2D > GetNormalDepthTexture() const { return m_NormalDepthTexture; }
		inline std::shared_ptr< RTexture2D > GetSpecularTexture() const { return m_SpecularTexture; }

		inline std::shared_ptr< RRenderTarget > GetDiffuseRoughnessTarget() const { return m_DiffuseRoughnessTarget; }
		inline std::shared_ptr< RRenderTarget > GetNormalDepthTarget() const { return m_NormalDepthTarget; }
		inline std::shared_ptr< RRenderTarget > GetSpecularTarget() const { return m_SpecularTarget; }

		bool UpdateDimensions( const std::shared_ptr< IGraphics >& inAPI, uint32 inWidth, uint32 inHeight );

		inline uint32 GetWidth() const		{ return m_Width; }
		inline uint32 GetHeight() const		{ return m_Height; }

		friend class IGraphics;
		friend class DirectX11Graphics;
	};

}
