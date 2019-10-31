/*==================================================================================================
	Hyperion Engine
	Source/Framework/Entity.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/Entity.h"
#include <iostream>

namespace Hyperion
{

	Entity::Entity()
	{
		m_IsAlive				= false;
		m_bRenderingEnabled		= false;
	}

	Entity::~Entity()
	{
		m_IsAlive				= false;
		m_bRenderingEnabled		= false;
	}

	void Entity::PerformSpawn()
	{
		OnSpawn();
	}

	void Entity::PerformDestroy()
	{
		// Destroy any components
		for( auto It = m_Components.begin(); It != m_Components.end(); )
		{
			std::shared_ptr< Component > Target = It->second;
			if( Target && Target->IsValid() )
			{
				Target->PerformDetach();
				It = m_Components.erase( It );
			}
			else
			{
				It++;
			}
		}

		m_Components.clear();

		OnDestroy();

		m_IsAlive		= false;
		m_World.reset();
	}

	void Entity::PerformTick( float Delta )
	{
		Tick( Delta );
	}


	void Entity::OnSpawn()
	{

	}

	void Entity::OnDestroy()
	{

	}

	void Entity::Tick( float Delta )
	{

	}

	bool Entity::IsAlive() const
	{
		return( m_IsAlive && !m_World.expired() );
	}

	std::weak_ptr< World > Entity::GetWorld() const
	{
		return m_World;
	}

	Vector3D Entity::GetWorldPosition() const
	{
		if( m_Parent.expired() )
			return GetPosition();

		return m_Parent.lock()->GetWorldPosition() + m_Position;
	}

	Angle3D Entity::GetWorldRotation() const
	{
		if( m_Parent.expired() )
			return GetRotation();

		return m_Parent.lock()->GetWorldRotation() + m_Rotation;
	}

	Vector3D Entity::GetWorldScale() const
	{
		if( m_Parent.expired() )
			return GetScale();

		return m_Parent.lock()->GetWorldScale() + m_Scale;
	}

	void Entity::SetPosition( const Vector3D& Position )
	{
		m_Position = Position;
	}

	void Entity::SetRotation( const Angle3D& Rotation )
	{
		m_Rotation = Rotation;
	}

	void Entity::SetScale( const Vector3D& Scale )
	{
		m_Scale = Scale;
	}

	bool Entity::RemoveComponent( std::weak_ptr< Component > Target )
	{
		// Ensure the target is valid, and part of this entity
		auto sharedTarget = Target.lock();
		if( !sharedTarget || !sharedTarget->IsValid() )
		{
			std::cout << "[ERROR] Entity: Attempt to destroy an invalid/null component!\n";
			return false;
		}

		auto ownerEntity = sharedTarget->GetOwner().lock();
		if( !ownerEntity || !ownerEntity->IsValid() || ownerEntity->GetID() != GetID() )
		{
			std::cout << "[ERROR] Entity: Attempt to destroy a component that isnt part of this entity!\n";
			return false;
		}

		// Now that everything looks good, lets actually destroy its children
		sharedTarget->PerformDetach();

		// Now we need to figure out where this component actually lives, so we can remove its shared pointer
		auto targetParent = sharedTarget->m_Parent.lock();

		if( !targetParent || !targetParent->IsValid() )
		{
			auto targetPos = m_Components.find( sharedTarget->GetName() );
			if( targetPos == m_Components.end() )
			{
				std::cout << "[WARNING] Entity: Attempt to destroy a root component.. but couldnt find it in the list!\n";
				return false;
			}

			m_Components.erase( targetPos );
		}
		else
		{
			// Look through the parents component list and remove it
			auto targetPos = targetParent->m_Children.find( sharedTarget->GetName() );
			if( targetPos == targetParent->m_Children.end() )
			{
				std::cout << "[WARNING] Entity: Attempt to destroy a non-root component, but couldnt find it in its parents' component list!\n";
				return false;
			}

			targetParent->m_Children.erase( targetPos );
		}

		// Remove weak reference from the full list
		m_FullComponentList.erase( sharedTarget->GetName() );

		return true;
	}
	
	/*==========================================================================
		Rendering Stuff
	==========================================================================*/
	void Entity::EnableRendering()
	{
		m_bRenderingEnabled = true;
	}

	void Entity::DisableRendering()
	{
		m_bRenderingEnabled = false;
	}

	bool Entity::IsRenderingEnabled() const
	{
		return m_bRenderingEnabled;
	}



}