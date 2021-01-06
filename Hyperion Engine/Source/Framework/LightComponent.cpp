/*==================================================================================================
	Hyperion Engine
	Source/Framework/LightComponent.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/LightComponent.h"
#include "Hyperion/Renderer/Proxy/ProxyLight.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Core/GameInstance.h"
#include "Hyperion/Renderer/Renderer.h"


namespace Hyperion
{

	void LightComponent::AddToRenderer()
	{
		Engine::GetGame()->RegisterRenderComponent( AquirePointer< LightComponent >() );
	}


	void LightComponent::RemoveFromRenderer()
	{
		Engine::GetGame()->RemoveRenderComponent( AquirePointer< LightComponent >() );
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
		Engine::GetRenderer()->AddCommand( std::make_unique< AddLightProxyCommand >( newProxy ) );
		m_Proxy = newProxy;

		return true;
	}


	bool LightComponent::PerformProxyUpdate()
	{
		auto ptr = m_Proxy.lock();
		if( ptr )
		{
			ptr->m_Transform = GetWorldTransform();
			return UpdateProxy( ptr );
		}
		else
		{
			Console::WriteLine( "[Warning] LightComponent: Failed to update light proxy, the pointer wasnt set" );
			return true;
		}
	}

}


/*
*	Register Type
*/
HYPERION_REGISTER_ABSTRACT_OBJECT_TYPE( LightComponent, RenderComponent );