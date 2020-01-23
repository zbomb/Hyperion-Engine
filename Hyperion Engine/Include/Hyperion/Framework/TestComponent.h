/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/TestComponent.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/PrimitiveComponent.h"


namespace Hyperion
{

	class ProxyTest;


	class TestComponent : public PrimitiveComponent
	{

	protected:

		std::shared_ptr< ProxyTest > m_Proxy;

		bool UpdateProxy() override;
		std::shared_ptr< ProxyPrimitive > CreateProxy() override;

	public:

		TestComponent();

	protected:

		void OnSpawn( const HypPtr< World >& inWorld ) override;
		void OnDespawn( const HypPtr< World >& inWorld ) override;

	};

}
