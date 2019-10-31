/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/Entity.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"
#include "Hyperion/Framework/Component.h"
#include "Hyperion/Core/Engine.h"

#include <vector>
#include <memory>
#include <map>

namespace Hyperion
{
	class World;

	class Entity : public Object
	{

	private:

		std::weak_ptr< World > m_World;
		std::vector< std::weak_ptr< Entity > > m_Children;
		std::weak_ptr< Entity > m_Parent;

		bool m_IsAlive;

		Vector3D m_Position;
		Angle3D m_Rotation;
		Vector3D m_Scale;

		void PerformSpawn();
		void PerformDestroy();
		void PerformTick( float Delta );

	public:

		Entity();
		~Entity();

		virtual void OnSpawn();
		virtual void OnDestroy();
		virtual void Tick( float Delta );

		bool IsAlive() const;
		std::weak_ptr< World > GetWorld() const;
		inline bool IsRoot() const { return !m_Parent.expired(); }

		/*
			Position/Rotation/Scale Getters
		*/
		inline Vector3D GetPosition() const		{ return m_Position; }
		inline Angle3D GetRotation() const		{ return m_Rotation; }
		inline Vector3D GetScale() const		{ return m_Scale; }

		Vector3D GetWorldPosition() const;
		Angle3D GetWorldRotation() const;
		Vector3D GetWorldScale() const;

		void SetPosition( const Vector3D& inPosition );
		void SetRotation( const Angle3D& inRotation );
		void SetScale( const Vector3D& inScale );

		/*
			Component System
		*/
	private:

		std::vector< size_t > m_DisallowedComponentClasses;
		std::vector< std::string > m_DisallowedComponentNames;
		std::map< std::string, std::shared_ptr< Component > > m_Components;
		std::map< std::string, std::weak_ptr< Component > > m_FullComponentList;

	public:

		template< typename T >
		std::weak_ptr< T > CreateRootComponent( const std::string& ComponentName )
		{
			// First, check for valid name
			if( ComponentName.size() <= 0 )
			{
				std::cout << "[ERROR] Entity: Tried to create a component with no valid name!\n";
				return std::weak_ptr< T >();
			}

			// Check if there is a component with this name already
			auto nameCheck = m_Components.find( ComponentName );
			auto fullNameCheck = m_FullComponentList.find( ComponentName );
			if( nameCheck != m_Components.end() || fullNameCheck != m_FullComponentList.end() )
			{
				std::cout << "[ERROR] Entity: Tried to create a component with a name that already exists!\n";
				return std::weak_ptr< T >();
			}

			// Check if this component class or name is disallowed
			// TODO: Convert component name to lower case?
			auto componentType = typeid( T ).hash_code();

			for( auto It = m_DisallowedComponentClasses.begin(); It != m_DisallowedComponentClasses.end(); It++ )
			{
				if( *It == componentType )
					return std::weak_ptr< T >();
			}

			for( auto It = m_DisallowedComponentNames.begin(); It != m_DisallowedComponentNames.end(); It++ )
			{
				if( *It == ComponentName )
					return std::weak_ptr< T >();
			}

			// Looks like were good to create this root component, first, actually create it
			auto& Eng = Engine::GetInstance();

			std::shared_ptr< T > newComponent = Eng.CreateObject< T >();

			// Next we need to set up the component
			std::shared_ptr< Component > compBase = std::dynamic_pointer_cast< Component >( newComponent );
			if( !compBase )
			{
				std::cout << "[ERROR] Entity: Failed to CreateRootComponent.. failed to downcast new component to Component base class!\n";
				return std::weak_ptr< T >();
			}

			compBase->m_Owner	= GetWeakPtr< Entity >();
			compBase->m_World	= GetWorld();
			compBase->m_Name	= ComponentName;
			
			// Store component
			m_Components[ ComponentName ] = compBase;
			m_FullComponentList[ ComponentName ] = std::weak_ptr< Component >( compBase );

			// Perform Init
			compBase->PerformAttach();

			std::cout << "[DEBUG] Entity: Created root component named \"";
			std::cout << ComponentName;
			std::cout << "\"\n\t";
			std::cout << "Parent Entity: ";
			std::cout << GetID();
			std::cout << "\n";

			return newComponent;
		}

		template< typename T >
		std::weak_ptr< T > CreateChildComponent( std::weak_ptr< Component > WeakParent, const std::string& ComponentName )
		{
			// Ensure name is valid
			if( ComponentName.size() <= 0 )
			{
				std::cout << "[ERROR] Entity: Tried to create a component with no valid name!\n";
				return std::weak_ptr< T >();
			}

			// Ensure parent is valid
			auto Parent = WeakParent.lock();
			if( !Parent || !Parent->IsValid() )
			{
				std::cout << "[ERROR] Entity: Tried to create a component with an invalid parent!\n";
				return std::weak_ptr< T >();
			}

			// Check if this component name is already in use
			auto nameCheck = Parent->m_Children.find( ComponentName );
			auto fullNameCheck = m_FullComponentList.find( ComponentName );
			if( nameCheck != Parent->m_Children.end() || fullNameCheck != m_FullComponentList.end() )
			{
				std::cout << "[ERROR] Entity: Tried to create a component (child) with a name that is already in use!\n";
				return std::weak_ptr< T >();
			}

			// Ensure parent belongs to this entity
			auto parentOwner = Parent->GetOwner().lock();
			if( !parentOwner || !parentOwner->IsAlive() || parentOwner->GetID() != GetID() )
			{
				std::cout << "[ERROR] Entity: Tried to create a component, and attach to a component that doesnt belong to this entity!\n";
				return std::weak_ptr< T >();
			}

			// Check if this component name or class is disallowed
			auto compType = typeid( T ).hash_code();

			for( auto It = m_DisallowedComponentClasses.begin(); It != m_DisallowedComponentClasses.end(); It++ )
			{
				if( *It == compType )
					return std::weak_ptr< T >();
			}

			for( auto It = m_DisallowedComponentNames.begin(); It != m_DisallowedComponentNames.end(); It++ )
			{
				if( *It == ComponentName )
					return std::weak_ptr< T >();
			}

			// Create new component
			auto& Eng = Engine::GetInstance();
			std::shared_ptr< T > newComponent = Eng.CreateObject< T >();

			// Setup component
			std::shared_ptr< Component > compBase = std::dynamic_pointer_cast< Component >( newComponent );

			if( !compBase )
			{
				std::cout << "[ERROR] Entity: Failed to CreateComponent.. failed to downcast new component to Component base class!\n";
				return std::weak_ptr< T >();
			}

			compBase->m_Owner	= GetWeakPtr< Entity >();
			compBase->m_World	= GetWorld();
			compBase->m_Name	= ComponentName;
			compBase->m_Parent	= std::weak_ptr< Component >( Parent );

			// Store this component reference
			Parent->m_Children[ ComponentName ] = compBase;
			m_FullComponentList[ ComponentName ] = std::weak_ptr< Component >( compBase );

			// Perform init and return the new component
			compBase->PerformAttach();

			std::cout << "[DEBUG] Entity: Created child component named \"";
			std::cout << ComponentName;
			std::cout << "\"\n\t";
			std::cout << "Parent Entity: ";
			std::cout << parentOwner->GetID();
			std::cout << "\n\t";
			std::cout << "Parent Component: ";
			std::cout << Parent->GetName();
			std::cout << "\n";

			return newComponent;
		}

		void DoNotCreateBaseClassComponent( const std::string& Name )
		{
			m_DisallowedComponentNames.push_back( Name );
		}

		template< typename T >
		void DoNotCreateBaseClassComponentClass()
		{
			m_DisallowedComponentClasses.push_back( typeid( T ).hash_code() );
		}

		bool RemoveComponent( std::weak_ptr< Component > Target );

		friend class World;

		/*
			Rendering Stuff
		*/
	private:

		bool m_bRenderingEnabled;

	public:

		void DisableRendering();
		void EnableRendering();
		bool IsRenderingEnabled() const;

	};

}