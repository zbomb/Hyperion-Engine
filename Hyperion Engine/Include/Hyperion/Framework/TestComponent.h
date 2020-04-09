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
	class TextureAsset;


	class TestComponent : public PrimitiveComponent
	{

	protected:

		std::shared_ptr< ProxyTest > m_Proxy;
		std::shared_ptr< TextureAsset > m_Asset;

		bool UpdateProxy() override;
		std::shared_ptr< ProxyPrimitive > CreateProxy() override;

	public:

		TestComponent();

		inline std::shared_ptr< TextureAsset > GetAsset() const { return m_Asset; }

	protected:

		void OnSpawn( const HypPtr< World >& inWorld ) override;
		void OnDespawn( const HypPtr< World >& inWorld ) override;

	};

}
