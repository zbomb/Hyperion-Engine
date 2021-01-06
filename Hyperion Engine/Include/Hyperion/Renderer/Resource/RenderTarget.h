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

		RRenderTarget( std::shared_ptr< RTexture2D > inTexture )
			: m_TargetTexture( inTexture )
		{
			if( !m_TargetTexture || !m_TargetTexture->IsValid() )
			{
				Console::WriteLine( "[ERROR] Renderer: Failed to create render target! Source texture was null or invalid!" );
				m_TargetTexture = nullptr;
			}
			else
			{
				// Ensure this texture is able to be used as a render target
				if( ( (int) m_TargetTexture->GetBindTarget() & (int) TextureBindTarget::Render ) == 0 )
				{
					Console::WriteLine( "[ERROR] Renderer: failed to create render target! Source texture is not bound properly!" );
					m_TargetTexture = nullptr;
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
			return m_TargetTexture;
		}

		virtual bool IsValid() const = 0;
		virtual void Shutdown() = 0;

	};

}