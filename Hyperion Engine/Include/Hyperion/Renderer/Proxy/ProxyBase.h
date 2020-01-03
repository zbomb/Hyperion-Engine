/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Proxy/ProxyBase.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

// Hyperion Includes
#include "Hyperion/Hyperion.h"


namespace Hyperion
{

	struct ProxyBase
	{
		
	protected:

		uint32 m_Identifier;

	public:

		ProxyBase() = delete;

		ProxyBase( uint32 inIdentifier )
			: m_Identifier( inIdentifier )
		{}

		virtual void Engine_Init() = 0;
		virtual void Render_Init() = 0;

		virtual void BeginShutdown() = 0;
		virtual void Shutdown() = 0;
		inline uint32 GetIdentifier() const { return m_Identifier; }
	};

}