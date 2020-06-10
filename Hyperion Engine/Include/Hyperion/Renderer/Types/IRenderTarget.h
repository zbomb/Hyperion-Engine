/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Types/IRenderTarget.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Types/Resource.h"
#include "Hyperion/Renderer/Types/ITexture.h"


namespace Hyperion
{

	class IRenderTarget : public IAPIResourceBase
	{

	protected:

		std::shared_ptr< ITexture2D > m_TargetTexture;

		IRenderTarget( std::shared_ptr< ITexture2D > inTexture )
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

		virtual ~IRenderTarget()
		{
			m_TargetTexture.reset();
		}

		IRenderTarget() = delete;
		IRenderTarget( const IRenderTarget& ) = delete;
		IRenderTarget( IRenderTarget&& ) = delete;
		IRenderTarget& operator=( const IRenderTarget& ) = delete;
		IRenderTarget& operator=( IRenderTarget&& ) = delete;

		std::shared_ptr< ITexture2D > GetTargetTexture()
		{
			return m_TargetTexture;
		}

	};

}