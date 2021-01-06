/*==================================================================================================
	Hyperion Engine
	Source/Framework/StaticModelComponent.cpp
	© 2019, Zachary Berry
==================================================================================================*/


#include "Hyperion/Framework/StaticModelComponent.h"
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Streaming/BasicStreamingManager.h"
#include "Hyperion/Renderer/Resource/ResourceManager.h"
#include "Hyperion/Renderer/Proxy/ProxyStaticModel.h"
#include "Hyperion/Assets/MaterialAsset.h"
#include "Hyperion/Assets/StaticModelAsset.h"


namespace Hyperion
{

	void StaticModelComponent::SetMaterial( const std::shared_ptr< MaterialAsset >& inMat, uint8 inSlot )
	{
		if( inSlot > 11 )
		{
			Console::WriteLine( "[Warning] StaticModelComponent: Attempt to set material to an invalid slot! There are only 12 material slots (Slot 0 to Slot 11)" );
			return;
		}

		m_MaterialSlots[ inSlot ] = inMat;
		MarkStale();
	}

	void StaticModelComponent::SetModel( const std::shared_ptr< StaticModelAsset >& inMdl )
	{
		m_Model = inMdl;
		MarkStale();
	}
	
	std::shared_ptr< ProxyPrimitive > StaticModelComponent::CreateProxy()
	{
		auto newProxy = std::make_shared< ProxyStaticModel >( GetIdentifier() );
		
		// Setup render resources needed
		auto resManager = Engine::GetRenderer()->GetResourceManager();
		newProxy->m_Model = resManager->GetGeometry( m_Model );
		
		for( auto it = m_MaterialSlots.begin(); it != m_MaterialSlots.end(); it++ )
		{
			if( it->second )
			{
				newProxy->m_Materials[ it->first ] = resManager->CreateMaterial( it->second );
			}
		}

		m_Proxy = newProxy;
		return newProxy;
	}

	bool StaticModelComponent::UpdateProxy( const std::shared_ptr< ProxyPrimitive >& inPrimitive )
	{
		auto ptr = std::dynamic_pointer_cast< ProxyStaticModel >( inPrimitive );
		if( ptr )
		{

		}

		return true;
	}
	
}


/*
*	Register Type
*/
HYPERION_REGISTER_OBJECT_TYPE( StaticModelComponent, PrimitiveComponent );