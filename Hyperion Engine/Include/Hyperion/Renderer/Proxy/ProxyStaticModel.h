/*==================================================================================================
	Hyperion Engine
	Hyperion/Renderer/Proxy/ProxyStaticModel.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Proxy/ProxyPrimitive.h"


namespace Hyperion
{

	class ProxyStaticModel : public ProxyPrimitive
	{

	protected:



	public:

		ProxyStaticModel() = delete;
		ProxyStaticModel( uint32 inIdentifier );

		void GameInit() override;
		void RenderInit() override;

		void BeginShutdown() override;
		void Shutdown() override;

	};

}