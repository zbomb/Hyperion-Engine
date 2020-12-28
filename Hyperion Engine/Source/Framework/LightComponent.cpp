/*==================================================================================================
	Hyperion Engine
	Source/Framework/LightComponent.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/LightComponent.h"
#include "Hyperion/Core/GameManager.h"
#include "Hyperion/Renderer/Proxy/ProxyLight.h"
#include "Hyperion/Core/RenderManager.h"


namespace Hyperion
{

	void LightComponent::AddToRenderer()
	{
		GameManager::GetInstance()->RegisterRenderComponent( AquirePointer< LightComponent >() );
	}


	void LightComponent::RemoveFromRenderer()
	{
		GameManager::GetInstance()->RemoveRenderComponent( AquirePointer< LightComponent >() );
	}


	bool LightComponent::PerformProxyCreation()
	{
		// Create the new proxy
		auto newProxy = CreateProxy();
		if( !newProxy )
		{
			return false;
		}

		// Initialize proxy
		newProxy->GameInit();

		// Add to renderer
		RenderManager::AddCommand( std::make_unique< AddLightProxyCommand >( newProxy ) );
		return true;
	}

}


/*
*	Register Type
*/
HYPERION_REGISTER_ABSTRACT_OBJECT_TYPE( LightComponent, RenderComponent );