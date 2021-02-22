/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resources/RRenderTarget.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{
	// Forward Declarations
	class RTexture2D;


	class RRenderTarget
	{

	protected:

		std::shared_ptr< RTexture2D > m_TargetTexture;

	public:

		RRenderTarget( const std::shared_ptr< RTexture2D >& inTexture )
			: m_TargetTexture( inTexture )
		{}

		virtual ~RRenderTarget()
		{}

		inline std::shared_ptr< RTexture2D > GetTargetTexture() const { return m_TargetTexture; }

		virtual bool IsValid() const = 0;
		virtual void Shutdown()
		{
			m_TargetTexture.reset();
		}

	};

}