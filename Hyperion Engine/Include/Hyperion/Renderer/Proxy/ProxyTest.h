/*==================================================================================================
	Hyperion Engine
	Hyperion/Renderer/Proxy/ProxyTest.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Proxy/ProxyPrimitive.h"


namespace Hyperion
{

	class ProxyTest : public ProxyPrimitive
	{

	public:

		ProxyTest() = delete;
		ProxyTest( uint32 inIdentifier );

		int m_Value;

		void GameInit() override;
		void RenderInit() override;

		void BeginShutdown() override;
		void Shutdown() override;

	};

}