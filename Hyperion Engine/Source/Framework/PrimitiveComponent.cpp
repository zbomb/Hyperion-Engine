/*==================================================================================================
	Hyperion Engine
	Source/Framework/PrimitiveComponent.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/PrimitiveComponent.h"
#include "Hyperion/Core/GameManager.h"
#include "Hyperion/Renderer/Proxy/ProxyPrimitive.h"
#include "Hyperion/Core/RenderManager.h"


namespace Hyperion
{

	void PrimitiveComponent::AddToRenderer()
	{
		GameManager::GetInstance()->RegisterRenderComponent( AquirePointer< PrimitiveComponent >() );
	}

	void PrimitiveComponent::RemoveFromRenderer()
	{
		GameManager::GetInstance()->RemoveRenderComponent( AquirePointer< PrimitiveComponent >() );
	}


	bool PrimitiveComponent::PerformProxyCreation()
	{
		// Create new primitive
		auto newProxy = CreateProxy();
		if( !newProxy )
		{
			return false;
		}

		// Ensure transform is set
		newProxy->m_Transform = GetWorldTransform();

		// Initialize primitive
		newProxy->GameInit();

		// Add to renderer
		RenderManager::AddCommand( std::make_unique< AddPrimitiveProxyCommand >( newProxy ) );
		return true;
	}

}


/*
*	Register Type
*/
HYPERION_REGISTER_ABSTRACT_OBJECT_TYPE( PrimitiveComponent, RenderComponent );