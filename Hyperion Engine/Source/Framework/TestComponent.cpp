/*==================================================================================================
	Hyperion Engine
	Source/Framework/TestComponent.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/TestComponent.h"
#include "Hyperion/Renderer/Proxy/ProxyTest.h"
#include "Hyperion/Streaming/AdaptiveAssetManager.h"



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

		// Inform the AA Manager of the adaptive assets we have
		AdaptiveAssetManagerSpawnEvent Event;

		// First, fill out data about the component itself
		Event.ObjectInfo.m_Identifier	= GetIdentifier();
		Event.ObjectInfo.m_Position		= GetPosition();
		Event.ObjectInfo.m_Radius		= 10.f; // TODO
		Event.ObjectInfo.m_ScreenSize	= 0.f; // TODO
		Event.ObjectInfo.m_Type			= AdaptiveAssetObjectType::Static;
		Event.ObjectInfo.m_Valid		= true;
		Event.ObjectInfo.m_Dirty		= true;

		// Next, we need to fill out info about the textures and models were using
		AdaptiveTextureInfo texInfo;
		texInfo.m_Asset = GetAsset();

		Event.Textures.push_back( texInfo );

		RenderManager::GetStreamingManager().OnPrimitiveSpawned( Event );

		Console::WriteLine( "[DEBUG] TestComponent: On spawn!" );
	}


	void TestComponent::OnDespawn( const HypPtr< World >& inWorld )
	{
		// Ensure the baseclass method gets called, so the proxy gets removed properly
		PrimitiveComponent::OnDespawn( inWorld );

		// Inform the streaming manager of the despawn
		AdaptiveAssetManagerDespawnEvent Event;
		Event.ObjectIdentifier = GetIdentifier();

		RenderManager::GetStreamingManager().OnPrimitiveDeSpawned( Event );

		Console::WriteLine( "[DEBUG] TestComponent: On despawn!" );
	}

}
