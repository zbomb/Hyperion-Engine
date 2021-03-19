/*==================================================================================================
	Hyperion Engine
	Source/Simulation/Game/Components/PrimitiveComponent.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Simulation/Game/Components/PrimitiveComponent.h"
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
		newProxy->m_Transform		= GetWorldTransform();
		newProxy->m_bMatrixDirty	= true;

		// Initialize primitive
		newProxy->GameInit();
		m_Proxy = newProxy;

		// Add to renderer
		Engine::GetRenderer()->AddCommand( std::make_unique< AddPrimitiveProxyCommand >( newProxy ) );
		return true;
	}


	bool PrimitiveComponent::PerformProxyUpdate()
	{
		// Test.. this really should be ran on the renderer.. im not sure why its not currently
		auto p = m_Proxy.lock();
		if( p )
		{
			return UpdateProxy( p );
		}
		else
		{
			return false;
		}
	}

}


/*
*	Register Type
*/
HYPERION_REGISTER_ABSTRACT_OBJECT_TYPE( PrimitiveComponent, RenderComponent );