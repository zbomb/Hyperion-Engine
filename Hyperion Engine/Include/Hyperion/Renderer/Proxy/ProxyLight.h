/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Proxy/ProxyLight.h
	� 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/Proxy/ProxyBase.h"


namespace Hyperion
{

	struct ProxyLight : public ProxyBase
	{

	public:

		ProxyLight() = delete;
		ProxyLight( uint32 inIdentifier )
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