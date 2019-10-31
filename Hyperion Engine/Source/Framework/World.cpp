/*==================================================================================================
	Hyperion Engine
	Source/Framework/World.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/World.h"


namespace Hyperion
{

	void World::Initialize()
	{
		std::cout << "[DEBUG] World: Initialize...\n";
	}

	void World::Shutdown()
	{
		std::cout << "[DEBUG] World: Shutdown...\n";
	}



	/*-----------------------------------------------------------
		Entity List Methods
	-----------------------------------------------------------*/

	/*
		World::GetEntityList()
		* Returns a vector of weak_ptrs of all valid entities in the world
	*/
	std::vector< std::weak_ptr< Entity > > World::GetEntityList()
	{
		std::vector< std::weak_ptr< Entity > > Output;
		
		for( auto cIt = m_EntityCache.begin(); cIt != m_EntityCache.end(); cIt++ )
		{
			for( auto oIt = cIt->second.begin(); oIt != cIt->second.end(); oIt++ )
			{
				// Ensure the entity is valid before adding to list
				if( oIt->second && oIt->second->IsValid() )
				{
					Output.push_back( std::weak_ptr< Entity >( oIt->second ) );
				}
			}
		}

		return Output;
	}

	/*
		World::GetEntityMap()
		* Returns a map where entity weak_ptr's are mapped to the ObjectIDs
	*/
	std::map< ObjectID, std::weak_ptr< Entity > > World::GetEntityMap()
	{
		std::map< ObjectID, std::weak_ptr< Entity > > Output;

		for( auto cIt = m_EntityCache.begin(); cIt != m_EntityCache.end(); cIt++ )
		{
			for( auto oIt = cIt->second.begin(); oIt != cIt->second.end(); oIt++ )
			{
				if( oIt->second && oIt->second->IsValid() )
				{
					Output[ oIt->first ] = std::weak_ptr< Entity >( oIt->second );
				}
			}
		}

		return Output;
	}

	/*
		World::GetEntityCache( ObjectCacheID )
		* Returns the entity cache with the specified ID
	*/
	EntityCache* World::GetEntityCache( ObjectCacheID Identifier )
	{
		auto target = m_EntityCache.find( Identifier );
		return target == m_EntityCache.end() ? nullptr : std::addressof( target->second );
	}

	/*
		World::IsEntityValid( ObjectID, ObjectCacheID )
		* Checks if an object is valid, quicker method, since we can directly look up the cache
	*/
	bool World::IsEntityValid( ObjectID Identifier, ObjectCacheID CacheIdentifier )
	{
		auto Target = GetEntity( Identifier, CacheIdentifier ).lock();
		return Target && Target->IsAlive();
	}

	/*
		World::IsEntityValid( ObjectID )
		* Checks if an object is valid, this is the slowest method
	*/
	bool World::IsEntityValid( ObjectID Identifier )
	{
		auto Target = GetEntity( Identifier ).lock();
		return Target && Target->IsAlive();
	}

	/*
		World::GetEntity( ObjectID, ObjectCacheID )
		* Finds an entity quickly using the objectID and cacheID
	*/
	std::weak_ptr< Entity > World::GetEntity( ObjectID Identifier, ObjectCacheID CacheIdentifier )
	{
		if( Identifier == OBJECT_INVALID )
			return std::weak_ptr< Entity >();
		else if( CacheIdentifier == CACHE_INVALID )
			return std::weak_ptr< Entity >();

		auto* TargetCache = GetEntityCache( CacheIdentifier );
		if( TargetCache == nullptr )
			return std::weak_ptr< Entity >();

		auto TargetObject = TargetCache->find( Identifier );
		if( TargetObject == TargetCache->end() )
			return std::weak_ptr< Entity >();

		// Check if the entity is valid before returning
		return TargetObject->second && TargetObject->second->IsValid() ? std::weak_ptr< Entity >( TargetObject->second ) : std::weak_ptr< Entity >();
	}

	/*
		World::GetEntity( ObjectID )
		* Slow entity lookup using the objectID alone
	*/
	std::weak_ptr< Entity > World::GetEntity( ObjectID Identifier )
	{
		if( Identifier == OBJECT_INVALID )
			return std::weak_ptr< Entity >();

		for( auto cIt = m_EntityCache.begin(); cIt != m_EntityCache.end(); cIt++ )
		{
			for( auto oIt = cIt->second.begin(); oIt != cIt->second.end(); oIt++ )
			{
				if( oIt->first == Identifier )
				{
					// Check if the entity is valid
					return oIt->second && oIt->second->IsValid() ? std::weak_ptr< Entity >( oIt->second ) : std::weak_ptr< Entity >();
				}
			}
		}

		return std::weak_ptr< Entity >();
	}

	/*
		World::DestroyEntity( Entity* )
		* Destroys an entity and removes it from the world
	*/
	bool World::DestroyEntity( std::weak_ptr< Entity > Target )
	{
		// Ensure target is valid, and within this world
		if( Target.expired() )
		{
			std::cout << "[WARNING] World: DestroyEntity was passed an expired weak_ptr!\n";
			return false;
		}

		auto entPtr = Target.lock();
		if( !entPtr || !entPtr->IsAlive() )
		{
			std::cout << "[WARNING] World: DestroyEntity was passed a null/dead entity!\n";
			return false;
		}

		auto entWorld = entPtr->GetWorld();
		if( entWorld.lock()->GetID() != GetID() )
		{
			std::cout << "[WARNING] World: DestroyEntity was passed an entity that didnt belong to this world!\n";
			return false;
		}

		// First, loop through and destroy children first
		for( auto It = entPtr->m_Children.begin(); It != entPtr->m_Children.end(); It++ )
		{
			if( !DestroyEntity( *It ) )
			{
				std::cout << "[WARNING] World: DestroyEntity failed to destroy the children of target entityt!\n";
			}
		}

		// Call PerformDestroy method
		entPtr->PerformDestroy();

		// Remove this entity from the cache
		auto entId = entPtr->GetID();

		for( auto cIt = m_EntityCache.begin(); cIt != m_EntityCache.end(); cIt++ )
		{
			for( auto oIt = cIt->second.begin(); oIt != cIt->second.end(); )
			{
				if( oIt->first == entId )
				{
					oIt = cIt->second.erase( oIt );
				}
				else
				{
					oIt++;
				}
			}
		}

		return true;
	}



}