/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/World.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"
#include "Hyperion/Framework/Entity.h"
#include "Hyperion/Core/Engine.h"
#include <map>
#include <memory>


namespace Hyperion
{

	typedef std::map< ObjectID, std::shared_ptr< Entity > > EntityCache;

	enum class EventOrder
	{
		ParentFirst,
		ParentLast
	};

	class World : public Object
	{

	protected:

		bool bIsActive;

	public:

		void Initialize() override;
		void Shutdown() override;

		inline bool IsActiveWorld() const { return bIsActive; }

		/*
			Entity List
		*/
	private:

		std::map< ObjectCacheID, EntityCache > m_EntityCache;

	public:

		// Entity vector/map getters
		std::vector< std::weak_ptr< Entity > > GetEntityList();
		std::map< ObjectID, std::weak_ptr< Entity > > GetEntityMap();

		// Entity cache list getters
		EntityCache* GetEntityCache( ObjectCacheID CacheIdentifier );

		template< typename T >
		EntityCache* GetEntityCache()
		{
			auto& engine = Engine::GetInstance();
			return GetEntityCache( engine.GetCacheIdentifier< T >() );
		}

		// Entity Validity Check
		bool IsEntityValid( ObjectID Identifier, ObjectCacheID CacheIdentifier );
		bool IsEntityValid( ObjectID Identifier );

		template< typename T >
		bool IsEntityValid( ObjectID Identifier )
		{
			auto& engine = Engine::GetInstance();
			return IsEntityValid( Identifier, engine.GetCacheIdentifier< T >() );
		}

		// Entity Getters
		std::weak_ptr< Entity > GetEntity( ObjectID Identifier, ObjectCacheID CacheIdentifier );
		std::weak_ptr< Entity > GetEntity( ObjectID Identifier );

		template< typename T >
		std::weak_ptr< Entity > GetEntity( ObjectID Identifier )
		{
			auto& engine = Engine::GetInstance();
			return GetEntity( Identifier, engine.GetCacheIdentifier< T >() );
		}

		// Entity Creation
		template< typename T >
		std::weak_ptr< T > CreateEntity( std::shared_ptr< Entity > Parent = nullptr )
		{
			// Ensure we have a valid entity type
			static_assert( std::is_base_of< Entity, T >::value, "Invalid object type specified, must not be an Entity!" );
			static_assert( !std::is_same< Entity, T >::value, "Invalid object type specified, Entity baseclass specified?" );

			// If there is a parent, we need to do a validity check
			if( Parent )
			{
				if( !Parent->IsValid() || !Parent->IsAlive() )
				{
					std::cout << "[WARNING] World: Attempt to create entity.. but the target parent was not valid/alive!\n";
					return std::weak_ptr< T >();
				}
			}

			// Create the new object
			auto& engine = Engine::GetInstance();
			std::shared_ptr< T > newEntity = engine.CreateRawObject< T >();

			// Add the entity to the world, first we must perform dynamic pointer cast
			auto cacheGroup = engine.GetCacheIdentifier< T >();
			std::shared_ptr< Entity > baseEntity = std::dynamic_pointer_cast< Entity >( newEntity );
			if( !baseEntity )
			{
				std::cout << "[World] Failed to downcast new entity! Error??\n";
				return std::weak_ptr< T >();
			}

			// Store strong pointer in entity cache
			m_EntityCache[ cacheGroup ][ baseEntity->GetID() ] = baseEntity;

			// Set parent ptr, and in the parent, add child ptr
			if( Parent )
			{
				baseEntity->m_Parent = std::weak_ptr< Entity >( Parent );
				Parent->m_Children.push_back( std::weak_ptr< Entity >( baseEntity ) );
			}

			// Init the new Entity
			baseEntity->m_IsAlive	= true;
			baseEntity->m_World		= GetWeakPtr< World >();

			baseEntity->PerformSpawn();

			// Return entity
			return std::weak_ptr< T >( newEntity );
		}

		// Entity Destruction
		bool DestroyEntity( std::weak_ptr< Entity > Target );

		/*
			Event System
		*/
	private:

		// Function Helper for 0 parameter version
		void _Impl_CallEvent( std::shared_ptr< Entity > Target, void( *FuncPtr )( Entity* ), EventOrder Order )
		{
			// Ensure function pointer is valid, and entity target is valid
			if( !FuncPtr || !Target || !Target->IsAlive() )
				return;

			// If were doing parent first, then call the function right away
			if( Order == EventOrder::ParentFirst )
				FuncPtr( Target.get() );

			// Loop through children, call this function for each one
			for( auto It = Target->m_Children.begin(); It != Target->m_Children.end(); It++ )
			{
				if( !It->expired() )
				{
					_Impl_CallEvent( It->lock(), FuncPtr, Order );
				}
			}

			// If were doing parent last, call function now
			if( Order == EventOrder::ParentLast )
				FuncPtr( Target.get() );
		}

		// Function Helper for 1 parameter version
		template< typename T1 >
		void _Impl_CallEvent( std::shared_ptr< Entity > Target, void( *FuncPtr )( Entity*, T1 ), T1 Param1, EventOrder Order )
		{
			if( !FuncPtr || !Target || !Target->IsAlive() )
				return;

			if( Order == EventOrder::ParentFirst )
				FuncPtr( Target.get(), Param1 );

			for( auto It = Target->m_Children.begin(); It != Target->m_Children.end(); It++ )
			{
				if( !It->expired() )
					_Impl_CallEvent< T1 >( It->lock(), FuncPtr, Param1, Order );
			}

			if( Order == EventOrder::ParentLast )
				FuncPtr( Target.get(), Param1 );

		}

		// Function Helper for 2 parameter version
		template< typename T1, typename T2 >
		void _Impl_CallEvent( std::shared_ptr< Entity > Target, void( *FuncPtr )( Entity*, T1, T2 ), T1 Param1, T2 Param2, EventOrder Order )
		{
			if( !FuncPtr || !Target || !Target->IsAlive() )
				return;

			if( Order == EventOrder::ParentFirst )
				FuncPtr( Target.get(), Param1, Param2 );

			for( auto It = Target->m_Children.begin(); It != Target->m_Children.end(); It++ )
			{
				if( !It->expired() )
					_Impl_CallEvent< T1, T2 >( It->lock(), FuncPtr, Param1, Param2, Order );
			}

			if( Order == EventOrder::ParentLast )
				FuncPtr( Target.get(), Param1, Param2 );
		}

		// Function Helper for 3 parameter version
		template< typename T1, typename T2, typename T3 >
		void _Impl_CallEvent( std::shared_ptr< Entity > Target, void(*FuncPtr)(Entity*, T1, T2, T3), T1 Param1, T2 Param2, T3 Param3, EventOrder Order )
		{
			if( !FuncPtr || !Target || !Target->IsAlive() )
				return;

			if( Order == EventOrder::ParentFirst )
				FuncPtr( Target.get(), Param1, Param2, Param3 );

			for( auto It = Target->m_Children.begin(); It != Target->m_Children.end(); It++ )
			{
				if( !It->expired() )
					_Impl_CallEvent< T1, T2, T3 >( It->lock(), FuncPtr, Param1, Param2, Param3, Order );
			}

			if( Order == EventOrder::ParentLast )
				FuncPtr( Target.get(), Param1, Param2, Param3 );
		}

		// Function Helper for 4 parameter version
		template< typename T1, typename T2, typename T3, typename T4 >
		void _Impl_CallEvent( std::shared_ptr< Entity > Target, void(*FuncPtr)(Entity*, T1, T2, T3, T4), T1 Param1, T2 Param2, T3 Param3, T4 Param4, EventOrder Order )
		{
			if( !FuncPtr || !Target || !Target->IsAlive() )
				return;

			if( Order == EventOrder::ParentFirst )
				FuncPtr( Target.get(), Param1, Param2, Param3, Param4 );

			for( auto It = Target->m_Children.begin(); It != Target->m_Children.end(); It++ )
			{
				if( !It->expired() )
					_Impl_CallEvent< T1, T2, T3, T4 >( It->lock(), FuncPtr, Param1, Param2, Param3, Param4, Order );
			}

			if( Order == EventOrder::ParentLast )
				FuncPtr( Target.get(), Param1, Param2, Param3, Param4 );
		}

	public:
		
		// 0 Paramter Version
		void CallEvent( void( *FuncPtr )( Entity* ), EventOrder Order = EventOrder::ParentLast )
		{
			for( auto It = m_EntityCache.begin(); It != m_EntityCache.end(); It++ )
			{
				for( auto oIt = It->second.begin(); oIt != It->second.end(); oIt++ )
				{
					auto Target = oIt->second;
					if( Target && Target->IsRoot() )
					{
						// Call the recursive helper function on all root entities, which will dispatch the event
						// to all of its children, and in the correct order
						_Impl_CallEvent( Target, FuncPtr, Order );
					}
				}
			}
		}

		// 1 Parameter Version
		template< typename T1 >
		void CallEvent( void( *FuncPtr )( Entity*, T1 ), T1 Param1, EventOrder Order = EventOrder::ParentLast )
		{
			for( auto Outer = m_EntityCache.begin(); Outer != m_EntityCache.end(); Outer++ )
			{
				for( auto Inner = Outer->second.begin(); Inner != Outer->second.end(); Inner++ )
				{
					if( Inner->second && Inner->second->IsRoot() )
						_Impl_CallEvent< T1 >( Inner->second, FuncPtr, Param1, Order );
				}
			}
		}

		// 2 Parameter Version
		template< typename T1, typename T2 >
		void CallEvent( void( *FuncPtr )( Entity*, T1, T2 ), T1 Param1, T2 Param2, EventOrder Order = EventOrder::ParentLast )
		{
			for( auto Outer = m_EntityCache.begin(); Outer != m_EntityCache.end(); Outer++ )
			{
				for( auto Inner = Outer->second.begin(); Inner != Outer->second.end(); Inner++ )
				{
					if( Inner->second && Inner->second->IsRoot() )
						_Impl_CallEvent< T1, T2 >( Inner->second, FuncPtr, Param1, Param2, Order );
				}
			}
		}

		// 3 Parameter Version
		template< typename T1, typename T2, typename T3 >
		void CallEvent( void( *FuncPtr )( Entity*, T1, T2, T3 ), T1 Param1, T2 Param2, T3 Param3, EventOrder Order = EventOrder::ParentLast )
		{
			for( auto Outer = m_EntityCache.begin(); Outer != m_EntityCache.end(); Outer++ )
			{
				for( auto Inner = Outer->second.begin(); Inner != Outer->second.end(); Inner++ )
				{
					if( Inner->second && Inner->second->IsRoot() )
						_Impl_CallEvent< T1, T2, T3 >( Inner->second, FuncPtr, Param1, Param2, Param3, Order );
				}
			}
		}

		// 4 Parameter Version
		template< typename T1, typename T2, typename T3, typename T4 >
		void CallEvent( void( *FuncPtr )( Entity*, T1, T2, T3, T4 ), T1 Param1, T2 Param2, T3 Param3, T4 Param4, EventOrder Order = EventOrder::ParentLast )
		{
			for( auto Outer = m_EntityCache.begin(); Outer != m_EntityCache.end(); Outer++ )
			{
				for( auto Inner = Outer->second.begin(); Inner != Outer->second.end(); Inner++ )
				{
					if( Inner->second && Inner->second->IsRoot() )
						_Impl_CallEvent< T1, T2, T3, T4 >( Inner->second, FuncPtr, Param1, Param2, Param3, Param4, Order );
				}
			}
		}



		friend class Engine;

	};

}
