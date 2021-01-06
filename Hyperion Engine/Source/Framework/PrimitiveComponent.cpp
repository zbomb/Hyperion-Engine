/*==================================================================================================
	Hyperion Engine
	Source/Framework/PrimitiveComponent.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/PrimitiveComponent.h"
#include "Hyperion/Renderer/Proxy/ProxyPrimitive.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Core/GameInstance.h"
#include "Hyperion/Renderer/Renderer.h"


namespace Hyperion
{

	void PrimitiveComponent::AddToRenderer()
	{
		Engine::GetGame()->RegisterRenderComponent( AquirePointer< PrimitiveComponent >() );
	}

	void PrimitiveComponent::RemoveFromRenderer()
	{
		Engine::GetGame()->RemoveRenderComponent( AquirePointer< PrimitiveComponent >() );
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
		m_Proxy = newProxy;

		// Add to renderer
		Engine::GetRenderer()->AddCommand( std::make_unique< AddPrimitiveProxyCommand >( newProxy ) );
		return true;
	}


	bool PrimitiveComponent::PerformProxyUpdate()
	{
		auto ptr = m_Proxy.lock();
		if( ptr )
		{
			ptr->m_Transform = GetWorldTransform();
			
			return UpdateProxy( ptr );
		}
		else
		{
			Console::WriteLine( "[Warning] PrimitiveComponent: Failed to update render proxy! The stored pointer was invalid" );
			return true;
		}
	}

}


/*
*	Register Type
*/
HYPERION_REGISTER_ABSTRACT_OBJECT_TYPE( PrimitiveComponent, RenderComponent );