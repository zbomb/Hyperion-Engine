/*==================================================================================================
	Hyperion Engine
	Source/Framework/Entity.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/Entity.h"
#include "Hyperion/Framework/World.h"
#include "Hyperion/Core/InputManager.h"
#include <iostream>

namespace Hyperion
{

	/*----------------------------------------------------------------------
		Entity Constructor
	----------------------------------------------------------------------*/
	Entity::Entity()
		: m_bIsSpawned( false )
	{
	}

	/*----------------------------------------------------------------------
		Entity Destructor
	----------------------------------------------------------------------*/
	Entity::~Entity()
	{
		m_bIsSpawned = false;
	}


	/*----------------------------------------------------------------------
		Entity::Initialize 
	----------------------------------------------------------------------*/
	void Entity::Initialize()
	{
		OnCreate();

		// Bind Input
		auto& eng = Engine::GetInstance();
		BindUserInput( eng.GetInputManager() );
	}


	/*----------------------------------------------------------------------
		Entity::Shutdown 
	----------------------------------------------------------------------*/
	void Entity::Shutdown()
	{
		// If were attached to another entity.. remove from that entity
		// If were attached to a world.. remove from that world
		if( m_Parent.IsValid() )
		{
			if( !RemoveFromParent() )
			{
				std::cout << "[ERROR] Entity: Failed to remove self from parent during shutdown!\n";
			}
		}
		else if( m_World.IsValid() )
		{
			if( !m_World->RemoveEntity( AquirePointer< Entity >() ) )
			{
				std::cout << "[ERROR] Entity: Failed to remove self from world during shutdown!\n";
			}
		}

		// If we have components or children.. we want to destroy them
		auto childrenCopy = m_Children;
		for( auto It = childrenCopy.begin(); It != childrenCopy.end(); It++ )
		{
			if( *It )
			{
				DestroyObject( *It );
			}
		}

		childrenCopy.clear();
		m_Children.clear();

		auto compListCopy = m_Components;
		for( auto It = compListCopy.begin(); It != compListCopy.end(); It++ )
		{
			if( It->second )
			{
				DestroyObject( It->second );
			}
		}

		compListCopy.clear();
		m_Components.clear();

		// Clear Bindings
		auto& eng = Engine::GetInstance();
		eng.GetInputManager().ClearBindings( AquirePointer< Entity >() );

		m_bIsSpawned = false;
		OnDestroy();
	}


	void Entity::SetWorld( const HypPtr< World >& inWorld )
	{
		m_World = inWorld;

		for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
		{
			if( *It ) (*It)->SetWorld( inWorld );
		}

		for( auto It = m_Components.begin(); It != m_Components.end(); It++ )
		{
			if( It->second ) It->second->SetWorld( inWorld );
		}
	}

	void Entity::PerformSpawn()
	{
		// Check if were already active
		if( m_bIsSpawned )
		{
			return;
		}

		// Validate world
		if( !m_World || !m_World->IsValid() )
		{
			std::cout << "[ERROR] Entity: Attempt to spawn this entity without having a valid world set!\n";
			return;
		}

		m_bIsSpawned = true;
		OnSpawn( m_World );

		for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
		{
			if( *It ) (*It)->PerformSpawn();
		}

		for( auto It = m_Components.begin(); It != m_Components.end(); It++ )
		{
			if( It->second ) It->second->PerformSpawn();
		}
	}

	void Entity::PerformDespawn()
	{
		if( !m_bIsSpawned )
		{
			return;
		}

		m_bIsSpawned = false;
		
		for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
		{
			if( *It ) (*It)->PerformDespawn();
		}

		for( auto It = m_Components.begin(); It != m_Components.end(); It++ )
		{
			if( It->second ) It->second->PerformDespawn();
		}

		OnDespawn( m_World );
	}


	/*----------------------------------------------------------------------
		Entity::AddChild 
	----------------------------------------------------------------------*/
	bool Entity::AddChild( const HypPtr< Entity >& inChild )
	{
		// Validate the child entity
		if( !inChild || !inChild->IsValid() )
		{
			std::cout << "[ERROR] Entity: Attempt to add an invalid sub-entity!\n";
			return false;
		}

		if( inChild->GetParent() )
		{
			std::cout << "[ERROR] Entity: Attempt to add a sub-entity that is already attached to another entity!\n";
			return false;
		}

		if( inChild->IsSpawned() )
		{
			std::cout << "[ERROR] Entity: Attempt to add a sub-entity that is already spawned into the world!\n";
			return false;
		}

		// Update state
		m_Children.push_back( inChild );

		auto thisPointer = AquirePointer< Entity >();
		inChild->m_Parent = thisPointer;

		// Call Hooks
		OnChildAdded( inChild );
		inChild->OnParentChanged( thisPointer, nullptr );

		// Spawn this entity if needed
		if( m_World )
		{
			inChild->SetWorld( m_World );
		}

		if( IsSpawned() )
		{
			inChild->PerformSpawn();
		}

		return true;
	}


	/*----------------------------------------------------------------------
		Entity::RemoveChild [By Ptr] 
	----------------------------------------------------------------------*/
	bool Entity::RemoveChild( const HypPtr< Entity >& inTarget )
	{
		if( !inTarget || !inTarget->IsValid() )
		{
			std::cout << "[ERROR] Entity: Attempt to remove invalid child from this entity!\n";
			return false;
		}

		auto thisPointer = AquirePointer< Entity >();

		if( inTarget->GetParent() != thisPointer )
		{
			std::cout << "[ERROR] Entity: Attempt to remove child from this entity that doesnt belong to this entity!\n";
			return false;
		}

		// First we need to despawn this entity and its children
		if( inTarget->IsSpawned() )
		{
			inTarget->PerformDespawn();
		}

		inTarget->SetWorld( nullptr );

		// Next, call hooks
		inTarget->OnParentChanged( nullptr, AquirePointer< Entity >() );
		OnChildRemoved( inTarget );

		// Finally, update state
		inTarget->m_Parent = nullptr;
		
		for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
		{
			if( *It && (*It) == inTarget )
			{
				m_Children.erase( It );
				break;
			}
		}

		return true;
	}


	/*----------------------------------------------------------------------
		Entity::RemoveChild [By Id] 
	----------------------------------------------------------------------*/
	bool Entity::RemoveChild( uint32 inIdentifier )
	{
		for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
		{
			if( *It && (*It)->GetIdentifier() == inIdentifier )
			{
				return RemoveChild( *It );
			}
		}

		std::cout << "[ERROR] Entity: Attempt to remove sub-entity but couldnt find based on the object identifier!\n";
		return false;
	}

	/*----------------------------------------------------------------------
		Entity::AddComponent
	----------------------------------------------------------------------*/
	bool Entity::AddComponent( const HypPtr< Component >& inComponent, const String& inIdentifier, const HypPtr< Component >& inParent /* = nullptr */ )
	{
		// First, we want to ensure the parameters are valid
		if( !inComponent || inComponent->IsAttached() || !inComponent->IsValid() )
		{
			std::cout << "[ERROR] Entity: Attempt to attach an invalid component!\n";
			return false;
		}

		auto thisPointer = AquirePointer< Entity >();

		if( inParent )
		{
			if( !inParent->IsValid() || inParent->GetOwner() != thisPointer )
			{
				std::cout << "[ERROR] Entity: Attempt to attach a component to this entity, but the parent component specified is invalid\n";
				return false;
			}
		}

		if( inIdentifier.IsEmpty() || m_AllComponents.find( inIdentifier ) != m_AllComponents.end() )
		{
			std::cout << "[ERROR] Entity: Attempt to attach a component with an invalid/existing identifier\n";
			return false;
		}

		// Now we can start to setup the state changes
		if( inParent )
		{
			inParent->m_Children[ inIdentifier ] = inComponent;
			inComponent->m_Parent = inParent;
		}
		else
		{
			m_Components[ inIdentifier ] = inComponent;
		}

		m_AllComponents[ inIdentifier ] = inComponent;

		inComponent->m_ComponentIdentifier	= inIdentifier;
		inComponent->m_Owner				= thisPointer;

		if( m_World )
		{
			inComponent->m_World = m_World;
		}

		// Call hooks for attachment
		OnComponentAdded( inComponent );

		if( inParent )
		{
			inParent->OnChildAdded( inComponent );
		}

		inComponent->OnAttach( thisPointer, inParent );

		// Spawn this component if this entity is already spawned
		if( IsSpawned() )
		{
			inComponent->m_bIsSpawned = true;
			inComponent->OnSpawn( m_World );
		}

		return true;
	}


	/*----------------------------------------------------------------------
		Entity::RemoveComponent [By Ptr]
	----------------------------------------------------------------------*/
	bool Entity::RemoveComponent( const HypPtr< Component >& inTarget )
	{
		// Validate the target component
		if( !inTarget )
		{
			std::cout << "[ERROR] Entity: Attempt to detach a component from this entity that is not valid!\n";
			return false;
		}

		auto thisPointer = AquirePointer< Entity >();
		if( inTarget->m_Owner != thisPointer )
		{
			std::cout << "[ERROR] Entity: Attempt to detach a component that doesnt belong to this entity!\n";
			return false;
		}

		// Now, if this component has children.. were going to remove all of them first
		if( inTarget->m_Children.size() > 0 )
		{
			std::cout << "[WARNING] Entity: Detaching a component with subcomponents.. this means all subcomponents will also be detached from their parents\n";
		}

		for( auto It = inTarget->m_Children.begin(); It != inTarget->m_Children.end(); )
		{
			if( It->second )
			{
				RemoveComponent( It->second );
				It++;
			}
			else
			{
				It = inTarget->m_Children.erase( It );
			}
		}

		// First, lets despawn the component if its spawned
		if( inTarget->IsActive() )
		{
			inTarget->OnDespawn( inTarget->m_World );
			inTarget->m_bIsSpawned = false;
		}

		inTarget->m_World.Clear();

		// Next, lets call the detach hooks
		auto targetParent = inTarget->GetParent();

		inTarget->OnDetach( thisPointer, targetParent );
		if( targetParent )
		{
			targetParent->OnChildRemoved( inTarget );
		}

		OnComponentRemoved( inTarget );

		// Finally, lets update the state for everything
		inTarget->m_ComponentIdentifier		= nullptr;
		inTarget->m_Owner					= nullptr;
		inTarget->m_Parent					= nullptr;

		// Erase this component from our full component list
		for( auto It = m_AllComponents.begin(); It != m_AllComponents.end(); It++ )
		{
			if( It->second == inTarget )
			{
				m_AllComponents.erase( It );
				break;
			}
		}

		if( targetParent )
		{
			// Erase this component from its parents component list
			for( auto It = targetParent->m_Children.begin(); It != targetParent->m_Children.end(); It++ )
			{
				if( It->second == inTarget )
				{
					targetParent->m_Children.erase( It );
					break;
				}
			}
		}
		else
		{
			// Erase this component from our root component list
			for( auto It = m_Components.begin(); It != m_Components.end(); It++ )
			{
				if( It->second == inTarget )
				{
					m_Components.erase( It );
					break;
				}
			}
		}

		return true;
	}


	/*----------------------------------------------------------------------
		Entity::RemoveComponent [By Str] 
	----------------------------------------------------------------------*/
	bool Entity::RemoveComponent( const String& inIdentifier )
	{
		if( inIdentifier.IsEmpty() )
		{
			std::cout << "[ERROR] Entity: Attempt to remove a component from this entity with a null identifier string\n";
			return false;
		}

		auto listEntry = m_AllComponents.find( inIdentifier );
		if( listEntry == m_AllComponents.end() )
		{
			std::cout << "[ERROR] Entity: Attempt to remove component from this entity by string identifier.. but it couldnt be found!\n";
			return false;
		}

		return RemoveComponent( listEntry->second );
	}


	/*----------------------------------------------------------------------
		Entity::RemoveComponent [By Id] 
	----------------------------------------------------------------------*/
	bool Entity::RemoveComponent( uint32 inIdentifier )
	{
		for( auto It = m_AllComponents.begin(); It != m_AllComponents.end(); It++ )
		{
			if( It->second && It->second->GetIdentifier() == inIdentifier )
			{
				return RemoveComponent( It->second );
			}
		}

		std::cout << "[ERROR] Entity: Attempt to remove a component from this entity by object identifier.. but it couldnt be found!\n";
		return false;
	}


	/*----------------------------------------------------------------------
		Entity::RemoveFromParent 
	----------------------------------------------------------------------*/
	bool Entity::RemoveFromParent()
	{
		auto parentEntity = GetParent();

		if( !parentEntity || !parentEntity->IsValid() )
		{
			std::cout << "[ERROR] Entity: Attempt to remove self from parent.. but there is no parent!\n";
			return false;
		}

		return parentEntity->RemoveChild( AquirePointer< Entity >() );
	}


	/*----------------------------------------------------------------------
		Entity::GetRootComponents 
	----------------------------------------------------------------------*/
	void Entity::GetRootComponents( std::vector< HypPtr< Component > >& outComponents, bool bIncludeChildEnts /* = false */ ) const
	{
		if( bIncludeChildEnts )
		{
			for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
			{
				if( (*It) && (*It)->IsValid() )
				{
					(*It)->GetRootComponents( outComponents, true );
				}
			}
		}

		for( auto It = m_Components.begin(); It != m_Components.end(); It++ )
		{
			if( It->second ) outComponents.push_back( It->second );
		}
	}


	/*----------------------------------------------------------------------
		Entity::GetAllComponents 
	----------------------------------------------------------------------*/
	void Entity::GetAllComponents( std::vector< HypPtr< Component > >& outComponents, bool bIncludeChildEnts /* = false */ ) const
	{
		if( bIncludeChildEnts )
		{
			for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
			{
				if( (*It) && (*It)->IsValid() )
				{
					(*It)->GetAllComponents( outComponents, true );
				}
			}
		}

		for( auto It = m_AllComponents.begin(); It != m_AllComponents.end(); It++ )
		{
			if( It->second ) outComponents.push_back( It->second );
		}
	}


	/*----------------------------------------------------------------------
		Entity::GetComponentIdentifiers 
	----------------------------------------------------------------------*/
	void Entity::GetComponentIdentifiers( std::vector< String >& outIdentifiers, bool bOnlyRoot /* = false */ ) const
	{
		if( bOnlyRoot )
		{
			for( auto It = m_Components.begin(); It != m_Components.end(); It++ )
			{
				if( It->second ) outIdentifiers.push_back( It->first );
			}
		}
		else
		{
			for( auto It = m_AllComponents.begin(); It != m_AllComponents.end(); It++ )
			{
				if( It->second ) outIdentifiers.push_back( It->first );
			}
		}
	}


	/*----------------------------------------------------------------------
		Entity::GetChildren 
	----------------------------------------------------------------------*/
	void Entity::GetChildren( std::vector< HypPtr< Entity > >& outChildren, bool bOnlyRoot /* = false */ ) const
	{
		for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
		{
			if( (*It) && (*It)->IsValid() )
			{
				outChildren.push_back( *It );

				if( !bOnlyRoot )
				{
					(*It)->GetChildren( outChildren, false );
				}
			}
		}
	}


	/*----------------------------------------------------------------------
		Entity::GetParent
	----------------------------------------------------------------------*/
	HypPtr< Entity > Entity::GetParent() const
	{
		return( ( m_Parent && m_Parent->IsValid() ) ? m_Parent : nullptr );
	}


	/*----------------------------------------------------------------------
		Entity::IsActive
	----------------------------------------------------------------------*/
	bool Entity::IsSpawned() const
	{
		return IsValid() && m_World.IsValid() && m_bIsSpawned;
	}


	/*----------------------------------------------------------------------
		Entity::IsRoot
	----------------------------------------------------------------------*/
	bool Entity::IsRoot() const
	{
		return m_World.IsValid() && !m_Parent.IsValid();
	}


	/*----------------------------------------------------------------------
		Entity::GetWorldPosition 
	----------------------------------------------------------------------*/
	Vector3D Entity::GetWorldPosition() const
	{
		if( m_Parent && m_Parent->IsValid() )
		{
			return m_Parent->GetWorldPosition() + GetPosition();
		}
		else
		{
			return GetPosition();
		}
	}


	/*----------------------------------------------------------------------
		Entity::GetWorldRotation
	----------------------------------------------------------------------*/
	Angle3D Entity::GetWorldRotation() const
	{
		if( m_Parent && m_Parent->IsValid() )
		{
			return m_Parent->GetWorldRotation() + GetRotation();
		}
		else
		{
			return GetRotation();
		}
	}


	/*----------------------------------------------------------------------
		Entity::GetWorldScale
	----------------------------------------------------------------------*/
	Vector3D Entity::GetWorldScale() const
	{
		if( m_Parent && m_Parent->IsValid() )
		{
			return m_Parent->GetWorldScale() + GetScale();
		}
		else
		{
			return GetScale();
		}
	}


	/*----------------------------------------------------------------------
		Entity::GetWorldTransform 
	----------------------------------------------------------------------*/
	Transform3D Entity::GetWorldTransform() const
	{
		if( m_Parent && m_Parent->IsValid() )
		{
			return m_Parent->GetWorldTransform() + GetTransform();
		}
		else
		{
			return GetTransform();
		}
	}


	/*----------------------------------------------------------------------
		Entity::SetPosition 
	----------------------------------------------------------------------*/
	void Entity::SetPosition( const Vector3D& inPosition )
	{
		m_Transform.Position = inPosition;
		OnLocalTransformChanged();

		// Tell children and components about transform update
		TransmitEntityFunction( &Entity::OnWorldTransformChanged );
		TransmitComponentFunction( true, &Component::OnWorldTransformChanged );
	}


	/*----------------------------------------------------------------------
		Entity::SetRotation 
	----------------------------------------------------------------------*/
	void Entity::SetRotation( const Angle3D& inRotation )
	{
		m_Transform.Rotation = inRotation;
		OnLocalTransformChanged();

		TransmitEntityFunction( &Entity::OnWorldTransformChanged );
		TransmitComponentFunction( true, &Component::OnWorldTransformChanged );
	}


	/*----------------------------------------------------------------------
		Entity::SetScale 
	----------------------------------------------------------------------*/
	void Entity::SetScale( const Vector3D& inScale )
	{
		m_Transform.Scale = inScale;
		OnLocalTransformChanged();

		TransmitEntityFunction( &Entity::OnWorldTransformChanged );
		TransmitComponentFunction( true, &Component::OnWorldTransformChanged );
	}


	/*----------------------------------------------------------------------
		Entity::SetTransform 
	----------------------------------------------------------------------*/
	void Entity::SetTransform( const Transform3D& inTransform )
	{
		m_Transform = inTransform;
		OnLocalTransformChanged();

		TransmitEntityFunction( &Entity::OnWorldTransformChanged );
		TransmitComponentFunction( true, &Component::OnWorldTransformChanged );
	}


	/*----------------------------------------------------------------------
		Entity::OnCreate [Hook] 
	----------------------------------------------------------------------*/
	void Entity::OnCreate()
	{

	}


	/*----------------------------------------------------------------------
		Entity::OnDestroy [Hook] 
	----------------------------------------------------------------------*/
	void Entity::OnDestroy()
	{

	}


	/*----------------------------------------------------------------------
		Entity::OnSpawn [Hook] 
	----------------------------------------------------------------------*/
	void Entity::OnSpawn( const HypPtr< World >& inWorld )
	{

	}


	/*----------------------------------------------------------------------
		Entity::OnDespawn [Hook] 
	----------------------------------------------------------------------*/
	void Entity::OnDespawn( const HypPtr< World >& oldWorld )
	{

	}


	/*----------------------------------------------------------------------
		Entity::OnChildAdded [Hook] 
	----------------------------------------------------------------------*/
	void Entity::OnChildAdded( const HypPtr< Entity >& inChild )
	{

	}


	/*----------------------------------------------------------------------
		Entity::OnChildRemoved [Hook] 
	----------------------------------------------------------------------*/
	void Entity::OnChildRemoved( const HypPtr< Entity >& inChild )
	{

	}


	/*----------------------------------------------------------------------
		Entity::OnParentChanged [Hook] 
	----------------------------------------------------------------------*/
	void Entity::OnParentChanged( const HypPtr< Entity >& newParent, const HypPtr< Entity >& oldParent )
	{

	}


	/*----------------------------------------------------------------------
		Entity::OnComponentAdded [Hook] 
	----------------------------------------------------------------------*/
	void Entity::OnComponentAdded( const HypPtr< Component >& inComponent )
	{

	}


	/*----------------------------------------------------------------------
		Entity::OnComponentRemoved [Hook] 
	----------------------------------------------------------------------*/
	void Entity::OnComponentRemoved( const HypPtr< Component >& inComponent )
	{

	}


	/*----------------------------------------------------------------------
		Entity::OnLocalTransformChanged
	----------------------------------------------------------------------*/
	void Entity::OnLocalTransformChanged()
	{

	}


	/*----------------------------------------------------------------------
		Entity::OnWorldTransformChanged
	----------------------------------------------------------------------*/
	void Entity::OnWorldTransformChanged()
	{

	}


	/*----------------------------------------------------------------------
		Entity::BindUserInput
	----------------------------------------------------------------------*/
	void Entity::BindUserInput( InputManager& inManager )
	{

	}


}