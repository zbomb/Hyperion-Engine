/*==================================================================================================
	Hyperion Engine
	Source/Framework/StaticModelComponent.cpp
	© 2019, Zachary Berry
==================================================================================================*/


#include "Hyperion/Framework/StaticModelComponent.h"
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Streaming/BasicStreamingManager.h"
#include "Hyperion/Renderer/ResourceManager.h"
#include "Hyperion/Renderer/Proxy/ProxyStaticModel.h"
#include "Hyperion/Assets/MaterialAsset.h"
#include "Hyperion/Assets/StaticModelAsset.h"


namespace Hyperion
{

	void StaticModelComponent::SetMaterial( const std::shared_ptr< MaterialAsset >& inMat, uint8 inSlot )
	{
		if( inSlot >= 32 )
		{
			Console::WriteLine( "[Warning] StaticModelComponent: Attempt to set material to an invalid slot! There are only 12 material slots (Slot 0 to Slot 11)" );
			return;
		}

		auto entry = m_MaterialSlots.find( inSlot );
		auto streaming = Engine::GetRenderer()->GetStreamingManager();

		if( entry != m_MaterialSlots.end() && entry->second )
		{
			// There was already an existing texture in this slot, so dereference the textures being used
			for( auto it = entry->second->TexturesBegin(); it != entry->second->TexturesEnd(); it++ )
			{
				if( it->second )
				{
					streaming->DereferenceTexture( it->second );
				}
			}
		}

		if( inMat != nullptr )
		{
			for( auto it = inMat->TexturesBegin(); it != inMat->TexturesEnd(); it++ )
			{
				if( it->second )
				{
					streaming->ReferenceTexture( it->second );
				}
			}
		}

		m_MaterialSlots[ inSlot ] = inMat;
		MarkStale();
	}

	void StaticModelComponent::SetModel( const std::shared_ptr< StaticModelAsset >& inMdl )
	{
		if( m_Model != inMdl )
		{
			if( m_Model != nullptr )
			{
				Engine::GetRenderer()->GetStreamingManager()->DereferenceStaticModel( m_Model );
			}

			if( inMdl != nullptr )
			{
				Engine::GetRenderer()->GetStreamingManager()->ReferenceStaticModel( inMdl );
			}
		}

		m_Model = inMdl;
		MarkStale();
		
	}
	
	std::shared_ptr< ProxyPrimitive > StaticModelComponent::CreateProxy()
	{
		auto newProxy = std::make_shared< ProxyStaticModel >( GetIdentifier() );

		// Assign assets
		newProxy->m_ModelAsset		= m_Model;
		newProxy->m_MaterialAssets	= m_MaterialSlots;

		// TODO: Full LOD system
		// The system to select the proper LOD should happen in the game thread
		// We then inform the streaming system to load the data in
		// Then mark the component as dirty
		// When the proxy gets updated, we update m_ActiveLOD
		newProxy->m_ActiveLOD = 0;
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