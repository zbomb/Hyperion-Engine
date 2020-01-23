/*==================================================================================================
	Hyperion Engine
	Source/Framework/RenderComponent.cpp
	© 2019, Zachary Berry
==================================================================================================*/


#include "Hyperion/Framework/RenderComponent.h"


namespace Hyperion
{

	void RenderComponent::MarkDirty()
	{
		// If the component is already dirty or stale, we dont want to do anything
		if( m_RenderState == RenderComponentState::Clean )
		{
			m_RenderState = RenderComponentState::Dirty;
		}
	}

	void RenderComponent::MarkStale()
	{
		m_RenderState = RenderComponentState::Stale;
	}


	void RenderComponent::OnSpawn( const HypPtr< World >& inWorld )
	{
		// Add this to the renderer
		AddToRenderer();
	}


	void RenderComponent::OnDespawn( const HypPtr< World >& inWorld )
	{
		// Remove this from the renderer
		RemoveFromRenderer();
	}

}