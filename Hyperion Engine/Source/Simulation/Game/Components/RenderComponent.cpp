/*==================================================================================================
	Hyperion Engine
	Source/Simulation/Game/Components/RenderComponent.cpp
	© 2021, Zachary Berry
==================================================================================================*/


#include "Hyperion/Simulation/Game/Components/RenderComponent.h"


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

	void RenderComponent::SetPosition( const Vector3D& inPosition )
	{
		Component::SetPosition( inPosition );
		MarkDirty();
	}

	void RenderComponent::SetRotation( const Angle3D& inRotation )
	{
		Component::SetRotation( inRotation );
		MarkDirty();
	}

	void RenderComponent::SetQuaternion( const Quaternion& inQuat )
	{
		Component::SetQuaternion( inQuat );
		MarkDirty();
	}

	void RenderComponent::SetScale( const Vector3D& inScale )
	{
		Component::SetScale( inScale );
		MarkDirty();
	}

	void RenderComponent::SetTransform( const Transform& inTransform )
	{
		Component::SetTransform( inTransform );
		MarkDirty();
	}

	void RenderComponent::Translate( const Vector3D& inPos )
	{
		Component::Translate( inPos );
		MarkDirty();
	}

	void RenderComponent::Rotate( const Quaternion& inQuat )
	{
		Component::Rotate( inQuat );
		MarkDirty();
	}

	void RenderComponent::Rotate( const Angle3D& inEuler )
	{
		Component::Rotate( inEuler );
		MarkDirty();
	}

}


/*
*	Register Type
*/
HYPERION_REGISTER_ABSTRACT_OBJECT_TYPE( RenderComponent, Component );