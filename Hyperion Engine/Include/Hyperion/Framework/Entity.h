/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/Entity.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"
#include "Hyperion/Framework/Component.h"
#include "Hyperion/Library/Geometry.h"

#include <vector>
#include <memory>
#include <map>

namespace Hyperion
{
	class World;

	class Entity : public Object
	{

	private:

		/*-------------------------------------------------------
			Private Members
		-------------------------------------------------------*/

		HypPtr< Entity > m_Parent;
		HypPtr< World > m_World;

		std::vector< HypPtr< Entity > > m_Children;
		std::map< String, HypPtr< Component > > m_Components;
		std::map< String, HypPtr< Component > > m_AllComponents;

		Transform m_Transform;

		bool m_bIsSpawned;

	protected:


		/*-------------------------------------------------------
			Object Overrides
		-------------------------------------------------------*/

		void Initialize() final;
		void Shutdown() final;

		void SetWorld( const HypPtr< World >& inWorld );
		void PerformSpawn();
		void PerformDespawn();


		/*-------------------------------------------------------
			Hooks
		-------------------------------------------------------*/
		
		virtual void OnCreate();
		virtual void OnDestroy();

		virtual void OnSpawn( const HypPtr< World >& inNewWorld );
		virtual void OnDespawn( const HypPtr< World >& inOldWorld );

		virtual void OnChildAdded( const HypPtr< Entity >& inChild );
		virtual void OnChildRemoved( const HypPtr< Entity >& inChild );

		virtual void OnParentChanged( const HypPtr< Entity >& newParent, const HypPtr< Entity >& oldParent );

		virtual void OnComponentAdded( const HypPtr< Component >& newComponent );
		virtual void OnComponentRemoved( const HypPtr< Component >& oldComponent );

		virtual void OnLocalTransformChanged();
		virtual void OnWorldTransformChanged();

		virtual void BindUserInput( InputManager& inManager );

	public:

		/*-------------------------------------------------------
			Constructor/Destructor
		-------------------------------------------------------*/

		Entity();
		virtual ~Entity();

		/*-------------------------------------------------------
			Public Functions
		-------------------------------------------------------*/

		bool AddChild( const HypPtr< Entity >& inChild );
		bool RemoveChild( const HypPtr< Entity >& inChild );
		bool RemoveChild( uint32 inIdentifier );

		bool AddComponent( const HypPtr< Component >& inComponent, const String& inIdentifier, const HypPtr< Component >& inParent = nullptr );
		bool RemoveComponent( const HypPtr< Component >& inComponent );
		bool RemoveComponent( const String& inIdentifier );
		bool RemoveComponent( uint32 inIdentifier );

		bool RemoveFromParent();

		void GetRootComponents( std::vector< HypPtr< Component > >& outComponents, bool bIncludeChildEnts = false ) const;
		void GetAllComponents( std::vector< HypPtr< Component > >& outComponents, bool bIncludeChildEnts = false ) const;
		void GetComponentIdentifiers( std::vector< String >& outIdentifiers, bool bOnlyRoot = false ) const;

		void GetChildren( std::vector< HypPtr< Entity > >& outChildren, bool bOnlyRoot = false ) const;

		bool IsSpawned() const;
		bool IsRoot() const;

		inline HypPtr< World > GetWorld() const { return m_World; }

		HypPtr< Entity > GetParent() const;

		inline Vector3D GetPosition() const			{ return m_Transform.Position; }
		inline Angle3D GetRotation() const			{ return m_Transform.Rotation.GetEulerAngles(); }
		inline Quaternion GetQuaternion() const		{ return m_Transform.Rotation; }
		inline Vector3D GetScale() const			{ return m_Transform.Scale; }
		inline Transform GetTransform() const		{ return m_Transform; }

		Vector3D GetWorldPosition() const;
		Angle3D GetWorldRotation() const;
		Quaternion GetWorldQuaternion() const;
		Vector3D GetWorldScale() const;
		Transform GetWorldTransform() const;

		void SetPosition( const Vector3D& inPos );
		void SetRotation( const Angle3D& inRot );
		void SetQuaternion( const Quaternion& inQuat );
		void SetScale( const Vector3D& inScale );
		void SetTransform( const Transform& inTrans );

		void Translate( const Vector3D& inPos );
		void Rotate( const Quaternion& inQuat );
		void Rotate( const Angle3D& inEuler );

		friend class World;

		/*-------------------------------------------------------
			Hook Transmission
		-------------------------------------------------------*/
		void TransmitComponentFunction( bool bIncludeChildEnts, std::function< void( Component* ) > inFunc )
		{
			if( inFunc )
			{
				// Transmit to components of child entities
				if( bIncludeChildEnts )
				{
					for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
					{
						if( *It ) (*It)->TransmitComponentFunction( true, inFunc );
					}
				}

				// Transmit to our components
				for( auto It = m_Components.begin(); It != m_Components.end(); It++ )
				{
					if( It->second ) It->second->TransmitFunction( inFunc );
				}
			}
		}

		template< typename _Arg1 >
		void TransmitComponentFunction( bool bIncludeChildEnts, std::function< void( Component*, const _Arg1& ) > inFunc, const _Arg1& inArg )
		{
			if( inFunc )
			{
				// Transmit to components of child entities
				if( bIncludeChildEnts )
				{
					for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
					{
						if( *It ) (*It)->TransmitComponentFunction( true, inFunc, inArg );
					}
				}

				// Transmit to our components
				for( auto It = m_Components.begin(); It != m_Components.end(); It++ )
				{
					if( It->second ) It->second->TransmitFunction( inFunc, inArg );
				}
			}
		}

		template< typename _Arg1, typename _Arg2 >
		void TransmitComponentFunction( bool bIncludeChildEnts, std::function< void( Component*, const _Arg1&, const _Arg2* ) > inFunc, const _Arg1& inArg1, const _Arg2& inArg2 )
		{
			if( inFunc )
			{
				if( bIncludeChildEnts )
				{
					for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
					{
						if( *It ) (*It)->TransmitComponentFunction( true, inFunc, inArg1, inArg2 );
					}
				}

				for( auto It = m_Components.begin(); It != m_Components.end(); It++ )
				{
					if( It->second ) It->second->TransmitFunction( inFunc, inArg1, inArg2 );
				}
			}
		}

		void TransmitEntityFunction( std::function< void( Entity* ) > inFunc )
		{
			if( inFunc )
			{
				for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
				{
					if( *It )
					{
						inFunc( It->GetAddress() );
					}
				}

				inFunc( this );
			}
		}

		template< typename _Arg1 >
		void TransmitEntityFunction( std::function< void( Entity*, const _Arg1& ) > inFunc, const _Arg1& inArg1 )
		{
			if( inFunc )
			{
				for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
				{
					if( *It )
					{
						inFunc( It->GetAddress(), inArg1 );
					}
				}

				inFunc( this, inArg1 );
			}
		}

		template< typename _Arg1, typename _Arg2 >
		void TransmitEntityFunction( std::function< void( Entity*, const _Arg1&, const _Arg2& ) > inFunc, const _Arg1& inArg1, const _Arg2& inArg2 )
		{
			if( inFunc )
			{
				for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
				{
					if( *It )
					{
						inFunc( It->GetAddress(), inArg1, inArg2 );
					}
				}

				inFunc( this, inArg1, inArg2 );
			}
		}

	};

}