/*==================================================================================================
	Hyperion Engine
	Include / Hyperion / Renderer / Types / RRenderTarget.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resource/Texture.h"


namespace Hyperion
{

	class RRenderTarget
	{

	protected:

		std::shared_ptr< RTexture2D > m_TargetTexture;
		TextureFormat m_Format;

		RRenderTarget( const std::shared_ptr< RTexture2D >& inTexture )
			: m_TargetTexture( inTexture ), m_Format( inTexture ? inTexture->GetFormat() : TextureFormat::NONE )
		{
			if( !m_TargetTexture || !m_TargetTexture->IsValid() )
			{
				Console::WriteLine( "[Warning] Renderer: Failed to create render target! Source texture was null or invalid!" );
				m_TargetTexture.reset();
			}
			else
			{
				// Ensure this texture is able to be used as a render target
				if( !m_TargetTexture->HasBindTarget( TextureBindTarget::Render ) )
				{
					Console::WriteLine( "[Warning] Renderer: Failed to create render target! Target texture isnt able to be rendered to" );
					m_TargetTexture.reset();
				}
			}
		}

	public:

		virtual ~RRenderTarget()
		{
			m_TargetTexture.reset();
		}

		RRenderTarget() = delete;
		RRenderTarget( const RRenderTarget& ) = delete;
		RRenderTarget( RRenderTarget&& ) = delete;
		RRenderTarget& operator=( const RRenderTarget& ) = delete;
		RRenderTarget& operator=( RRenderTarget&& ) = delete;

		std::shared_ptr< RTexture2D > GetTargetTexture()
		{
			return m_TargetTexture && m_TargetTexture->IsValid() ? m_TargetTexture : nullptr;
		}

		inline TextureFormat GetFormat() const { return m_Format; }

		virtual bool IsValid() const = 0;
		virtual void Shutdown() = 0;

	};

}