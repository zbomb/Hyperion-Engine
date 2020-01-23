/*==================================================================================================
	Hyperion Engine
	Source/Framework/TestComponent.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/TestComponent.h"
#include "Hyperion/Renderer/Proxy/ProxyTest.h"



namespace Hyperion
{

	TestComponent::TestComponent()
		: m_Proxy( nullptr )
	{}


	std::shared_ptr< ProxyPrimitive > TestComponent::CreateProxy()
	{
		// Create new proxy
		auto ptr = std::make_shared< ProxyTest >( GetIdentifier() );
		if( !ptr ) return nullptr;

		// Cache this proxy
		m_Proxy = ptr;

		// Return so the syste can init and add it to the renderer
		return ptr;
	}


	bool TestComponent::UpdateProxy()
	{
		auto ptr = m_Proxy;
		if( !ptr ) return false;

		HYPERION_RENDER_COMMAND(
			[ ptr ] ( Renderer& inRenderer )
			{
				ptr->m_Value = 10;
				Console::WriteLine( "[DEBUG] TestComponent: Updated proxy.. new value: ", ptr->m_Value );
			}
		);

		return true;
	}


	void TestComponent::OnSpawn( const HypPtr< World >& inWorld )
	{
		// Ensure the baseclass method gets called, so the proxy gets setup properly
		PrimitiveComponent::OnSpawn( inWorld );

		Console::WriteLine( "[DEBUG] TestComponent: On spawn!" );
	}


	void TestComponent::OnDespawn( const HypPtr< World >& inWorld )
	{
		// Ensure the baseclass method gets called, so the proxy gets removed properly
		PrimitiveComponent::OnDespawn( inWorld );

		Console::WriteLine( "[DEBUG] TestComponent: On despawn!" );
	}

}
