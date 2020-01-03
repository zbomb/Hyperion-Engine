/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Proxy/ProxyPrimitive.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/Proxy/ProxyBase.h"


namespace Hyperion
{

	struct ProxyPrimitive : public ProxyBase
	{

	public:

		ProxyPrimitive() = delete;
		ProxyPrimitive( uint32 inIdentifier )
			: ProxyBase( inIdentifier )
		{}

		void Engine_Init()
		{

		}

		void Render_Init()
		{

		}

		void Shutdown()
		{

		}

	};

}