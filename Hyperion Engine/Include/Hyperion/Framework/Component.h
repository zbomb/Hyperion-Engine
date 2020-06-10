/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/Component.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"
#include "Hyperion/Library/Math/Transform.h"
#include "Hyperion/Core/String.h"

#include <vector>
#include <string>
#include <map>
#include <functional>


namespace Hyperion
{

	class Entity;
	class World;

	class Component : public Object
	{

	private:

		/*-------------------------------------------------------
			Private Members
		-------------------------------------------------------*/

		HypPtr< Entity > m_Owner;
		HypPtr< World > m_World;
		HypPtr< Component > m_Parent;

		std::map< String, HypPtr< Component > > m_Children;

		Transform3D m_Transform;

		String m_ComponentIdentifier;

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

		virtual void OnSpawn( const HypPtr< World >& inWorld );
		virtual void OnDespawn( const HypPtr< World >& oldWorld );

		virtual void OnAttach( const HypPtr< Entity >& inOwner, const HypPtr< Component >& inParent );
		virtual void OnDetach( const HypPtr< Entity >& oldOwner, const HypPtr< Component >& oldParent );

		virtual void OnChildAdded( const HypPtr< Component >& newChild );
		virtual void OnChildRemoved( const HypPtr< Component >& oldChild );

		virtual void OnLocalTransformChanged();
		virtual void OnWorldTransformChanged();


		/*-------------------------------------------------------
			Helper Functions
		-------------------------------------------------------*/


	public:

		/*-------------------------------------------------------
			Constructor/Destructor
		-------------------------------------------------------*/

		Component();
		virtual ~Component();


		/*-------------------------------------------------------
			Public Functions
		-------------------------------------------------------*/

		bool RemoveFromParent();

		HypPtr< Component > GetParent() const;
		HypPtr< Entity > GetOwner() const;
		HypPtr< World > GetWorld() const;

		void GetComponents( std::vector< HypPtr< Component > >& outComponents, bool bOnlyRoot /* = true */ ) const;

		inline std::map< String, HypPtr< Component > >::const_iterator ComponentsBegin() const		{ return m_Children.begin(); }
		inline std::map< String, HypPtr< Component > >::const_iterator ComponentsEnd()	const		{ return m_Children.end(); }

		bool IsActive() const;
		bool IsRoot() const;
		bool IsAttached() const;

		inline String GetComponentIdentifier() const { return m_ComponentIdentifier; }

		inline Vector3D GetPosition() const			{ return m_Transform.Position; }
		inline Angle3D GetRotation() const			{ return m_Transform.Rotation; }
		inline Vector3D GetScale() const			{ return m_Transform.Scale; }
		inline Transform3D GetTransform() const		{ return m_Transform; }

		Vector3D GetWorldPosition() const;
		Angle3D GetWorldRotation() const;
		Vector3D GetWorldScale() const;
		Transform3D GetWorldTransform() const;

		void SetPosition( const Vector3D& inPosition );
		void SetRotation( const Angle3D& inRotation );
		void SetScale( const Vector3D& inScale );
		void SetTransform( const Transform3D& inTransform );

		/*-------------------------------------------------------
			Hook Transmission
		-------------------------------------------------------*/
		void TransmitFunction( std::function< void( Component* ) > inFunc )
		{
			if( inFunc )
			{
				for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
				{
					if( It->second ) It->second->TransmitFunction( inFunc );
				}

				inFunc( this );
			}
		}

		template< typename _Arg1 >
		void TransmitFunction( std::function< void( Component*, const _Arg1& ) > inFunc, const _Arg1& inArg )
		{
			if( inFunc )
			{
				for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
				{
					if( It->second ) It->second->TransmitFunction( inFunc, inArg );
				}

				inFunc( this, inArg );
			}
		}

		template< typename _Arg1, typename _Arg2 >
		void TransmitFunction( std::function< void( Component*, const _Arg1&, const _Arg2& ) > inFunc, const _Arg1& inArg1, const _Arg2& inArg2 )
		{
			if( inFunc )
			{
				for( auto It = m_Children.begin(); It != m_Children.end(); It++ )
				{
					if( It->second ) It->second->TransmitFunction( inFunc, inArg1, inArg2 );
				}

				inFunc( this, inArg1, inArg2 );
			}
		}

		/*-------------------------------------------------------
			Friends
		-------------------------------------------------------*/
		friend class Entity;
	};

}