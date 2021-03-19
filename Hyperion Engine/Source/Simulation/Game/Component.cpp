/*==================================================================================================
	Hyperion Engine
	Source/Simulation/Game/Component.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Simulation/Game/Component.h"
#include "Hyperion/Simulation/Game/Entity.h"
#include "Hyperion/Simulation/Game/World.h"

namespace Hyperion
{

	/*----------------------------------------------------------------------
		Component Constructor
	----------------------------------------------------------------------*/
	Component::Component()
		: m_bIsSpawned( false )
	{
	}


	/*----------------------------------------------------------------------
		Component Destructor
	----------------------------------------------------------------------*/
	Component::~Component()
	{
		m_bIsSpawned = false;
	}


	/*----------------------------------------------------------------------
		Component::Initialize
	----------------------------------------------------------------------*/
	void Component::Initialize()
	{
		OnCreate();
	}


	/*----------------------------------------------------------------------
		Component::Shutdown
	----------------------------------------------------------------------*/
	void Component::Shutdown()
	{
		// Destroy any child components
		auto childCopy = m_Children;
		for( auto It = childCopy.begin(); It != childCopy.end(); It++ )
		{
			if( It->second )
			{
				DestroyObject( It->second );
			}
		}

		childCopy.clear();
		m_Children.clear();

		// Remove ourself from the parent entity
		if( IsAttached() )
		{
			if( !RemoveFromParent() )
			{
				Console::WriteLine( "[ERROR] Component: Failed to remove self from parent during shutdown!" );
			}
		}

		OnDestroy();
		m_bIsSpawned = false;
	}

	void Component::SetWorld( const HypPtr< World >& inWorld )
	{
		m_World = inWorld;

		for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
		{
			if( It->second ) It->second->SetWorld( inWorld );
		}
	}

	void Component::PerformSpawn()
	{
		// Check if component is already spawned
		if( m_bIsSpawned )
		{
			return;
		}

		// Ensure a valid world is set and we have a parent
		if( !m_World || !m_World->IsValid() || !m_Owner || !m_Owner->IsValid() )
		{
			Console::WriteLine( "[ERROR] Component: Attempt to spawn component without a valid world and entity set!" );
			return;
		}

		m_bIsSpawned = true;
		OnSpawn( m_World );

		for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
		{
			if( It->second ) It->second->PerformSpawn();
		}
	}

	void Component::PerformDespawn()
	{
		// Check if component is spawned
		if( !m_bIsSpawned )
		{
			return;
		}

		for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
		{
			if( It->second ) It->second->PerformDespawn();
		}

		OnDespawn( m_World );
	}

	/*----------------------------------------------------------------------
		Component::RemoveFromParent
	----------------------------------------------------------------------*/
	bool Component::RemoveFromParent()
	{
		auto thisOwner = GetOwner();
		if( !thisOwner || !thisOwner->IsValid() )
		{
			Console::WriteLine( "[ERROR] Component: Attemp to remove this component from its owner.. but the owner was not valid!" );
			return false;
		}

		return thisOwner->RemoveComponent( AquirePointer< Component >() );
	}


	/*----------------------------------------------------------------------
		Component::GetParent
	----------------------------------------------------------------------*/
	HypPtr< Component > Component::GetParent() const
	{
		return m_Parent;
	}


	/*----------------------------------------------------------------------
		Component::GetOwner
	----------------------------------------------------------------------*/
	HypPtr< Entity > Component::GetOwner() const
	{
		return m_Owner;
	}


	/*----------------------------------------------------------------------
		Component::GetWorld
	----------------------------------------------------------------------*/
	HypPtr< World > Component::GetWorld() const
	{
		return m_World;
	}


	/*----------------------------------------------------------------------
		Component::IsActive
	----------------------------------------------------------------------*/
	bool Component::IsActive() const
	{
		return IsValid() && m_World.IsValid() && m_bIsSpawned;
	}


	/*----------------------------------------------------------------------
		Component::IsRoot
	----------------------------------------------------------------------*/
	bool Component::IsRoot() const
	{
		return m_Owner.IsValid() && !m_Parent.IsValid();
	}

	/*----------------------------------------------------------------------
		Component::IsAttached
	----------------------------------------------------------------------*/
	bool Component::IsAttached() const
	{
		return m_Owner.IsValid();
	}


	/*----------------------------------------------------------------------
		Component::GetComponents
	----------------------------------------------------------------------*/
	void Component::GetComponents( std::vector< HypPtr< Component > >& outComponents, bool bOnlyRoot /* = true */ ) const
	{
		for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
		{
			outComponents.push_back( It->second );

			if( !bOnlyRoot )
			{
				It->second->GetComponents( outComponents, false );
			}
		}
	}


	/*----------------------------------------------------------------------
		Component::GetWorldPosition
	----------------------------------------------------------------------*/
	Vector3D Component::GetWorldPosition() const
	{
		if( m_Parent )
		{
			return( m_Parent->GetWorldPosition() + m_Transform.Position );
		}
		else if( m_Owner )
		{
			return( m_Owner->GetWorldPosition() + m_Transform.Position );
		}
		else
		{
			return m_Transform.Position;
		}
	}


	/*----------------------------------------------------------------------
		Component::GetWorldRotation
	----------------------------------------------------------------------*/
	Angle3D Component::GetWorldRotation() const
	{
		return GetWorldQuaternion().GetEulerAngles();
	}

	Quaternion Component::GetWorldQuaternion() const
	{
		if( m_Parent )
		{
			return( m_Transform.Rotation * m_Parent->GetWorldQuaternion() );
		}
		else if( m_Owner )
		{
			return( m_Transform.Rotation * m_Owner->GetWorldQuaternion() );
		}
		else
		{
			return m_Transform.Rotation;
		}
	}

	/*----------------------------------------------------------------------
		Component::GetWorldScale
	----------------------------------------------------------------------*/
	Vector3D Component::GetWorldScale() const 
	{
		if( m_Parent )
		{
			return( m_Parent->GetWorldScale() + m_Transform.Scale );
		}
		else if( m_Owner )
		{
			return( m_Owner->GetWorldScale() + m_Transform.Scale );
		}
		else
		{
			return m_Transform.Scale;
		}
	}


	/*----------------------------------------------------------------------
		Component::GetWorldTransform
	----------------------------------------------------------------------*/
	Transform Component::GetWorldTransform() const
	{
		Transform Output;

		if( m_Parent )
		{
			// The order of additives matters when combining transforms!
			// This is because, the quaternions are being multiplied, and its not a communitive operation
			Output = m_Transform + m_Parent->GetWorldTransform();
		}
		else if( m_Owner )
		{
			Output = m_Transform + m_Owner->GetWorldTransform();
		}
		else
		{
			Output = m_Transform;
		}

		return Output;
	}


	/*----------------------------------------------------------------------
		Component::SetPosition
	----------------------------------------------------------------------*/
	void Component::SetPosition( const Vector3D& inPosition )
	{
		m_Transform.Position = inPosition;
		OnLocalTransformChanged();

		TransmitFunction( &Component::OnWorldTransformChanged );
	}


	/*----------------------------------------------------------------------
		Component::SetRotation
	----------------------------------------------------------------------*/
	void Component::SetRotation( const Angle3D& inRotation )
	{
		SetQuaternion( Quaternion( inRotation ) );
	}


	void Component::SetQuaternion( const Quaternion& inQuat )
	{
		m_Transform.Rotation = inQuat;
		OnLocalTransformChanged();
		TransmitFunction( &Component::OnWorldTransformChanged );
	}


	/*----------------------------------------------------------------------
		Component::SetScale
	----------------------------------------------------------------------*/
	void Component::SetScale( const Vector3D& inScale )
	{
		m_Transform.Scale = inScale;
		OnLocalTransformChanged();

		TransmitFunction( &Component::OnWorldTransformChanged );
	}


	/*----------------------------------------------------------------------
		Component::SetTransform
	----------------------------------------------------------------------*/
	void Component::SetTransform( const Transform& inTransform )
	{
		m_Transform = inTransform;

		OnLocalTransformChanged();
		TransmitFunction( &Component::OnWorldTransformChanged );
	}


	void Component::Translate( const Vector3D& inVec )
	{
		m_Transform = m_Transform.TranslateTransform( inVec );
		OnLocalTransformChanged();
		TransmitFunction( &Component::OnWorldTransformChanged );
	}

	void Component::Rotate( const Quaternion& inQuat )
	{
		m_Transform = m_Transform.RotateTransform( inQuat );
		OnLocalTransformChanged();
		TransmitFunction( &Component::OnWorldTransformChanged );
	}


	void Component::Rotate( const Angle3D& inEuler )
	{
		m_Transform = m_Transform.RotateTransform( inEuler );
		OnLocalTransformChanged();
		TransmitFunction( &Component::OnWorldTransformChanged );
	}

	
	/*----------------------------------------------------------------------
		Component::OnCreate
	----------------------------------------------------------------------*/
	void Component::OnCreate()
	{

	}


	/*----------------------------------------------------------------------
		Component::OnDestroy
	----------------------------------------------------------------------*/
	void Component::OnDestroy()
	{

	}


	/*----------------------------------------------------------------------
		Component::OnSpawn
	----------------------------------------------------------------------*/
	void Component::OnSpawn( const HypPtr< World >& inWorld )
	{

	}


	/*----------------------------------------------------------------------
		Component::OnDespawn
	----------------------------------------------------------------------*/
	void Component::OnDespawn( const HypPtr< World >& oldWorld )
	{

	}


	/*----------------------------------------------------------------------
		Component::OnAttach
	----------------------------------------------------------------------*/
	void Component::OnAttach( const HypPtr< Entity >& inOwner, const HypPtr< Component >& inParent )
	{

	}


	/*----------------------------------------------------------------------
		Component::OnDetach
	----------------------------------------------------------------------*/
	void Component::OnDetach( const HypPtr< Entity >& oldOwner, const HypPtr< Component >& oldParent )
	{

	}


	/*----------------------------------------------------------------------
		Component::OnChildAdded
	----------------------------------------------------------------------*/
	void Component::OnChildAdded( const HypPtr< Component >& newChild )
	{

	}


	/*----------------------------------------------------------------------
		Component::OnChildRemoved
	----------------------------------------------------------------------*/
	void Component::OnChildRemoved( const HypPtr< Component >& oldChild )
	{

	}


	/*----------------------------------------------------------------------
		Component::OnLocalTransformChanged
	----------------------------------------------------------------------*/
	void Component::OnLocalTransformChanged()
	{

	}


	/*----------------------------------------------------------------------
		Component::OnWorldTransformChanged
	----------------------------------------------------------------------*/
	void Component::OnWorldTransformChanged()
	{

	}

}


/*
*	Register Type
*/
HYPERION_REGISTER_ABSTRACT_OBJECT_TYPE( Component, Object );