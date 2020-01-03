/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Proxy/ProxyCamera.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/Proxy/ProxyBase.h"


namespace Hyperion
{

	struct ProxyCamera : public ProxyBase
	{

	public:

		ProxyCamera() = delete;
		ProxyCamera( uint32 inIdentifier )
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