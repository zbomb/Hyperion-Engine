/*==================================================================================================
	Hyperion Engine
	Source/Framework/World.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/World.h"
#include "Hyperion/Core/RenderManager.h"
#include "Hyperion/Framework/CameraComponent.h"


namespace Hyperion
{
	
	World::World()
		: m_bSpawned( false ), m_bActive( false )
	{
	}

	World::~World()
	{
	}

	void World::Initialize()
	{
		Console::WriteLine( "[DEBUG] World: Initialize..." );
	}

	void World::Shutdown()
	{
		Console::WriteLine( "[DEBUG] World: Shutdown..." );

		// If were active
		if( m_bSpawned )
		{
			if( !DespawnWorld() )
			{
				Console::WriteLine( "[ERROR] World: Failed to automatically despawn world on destruction!" );
			}
		}
	}


	bool World::SpawnWorld()
	{
		// Check if were already spawned
		if( m_bSpawned || !m_bActive )
		{
			return false;
		}

		m_bSpawned = true;
		OnSpawn();

		// Setup this world in the renderer
		

		// Spawn all entities
		for( auto It = m_ActiveEnts.begin(); It != m_ActiveEnts.end(); It++ )
		{
			if( It->second && It->second->IsValid() )
			{
				It->second->PerformSpawn();
			}
		}

		return true;
	}

	bool World::DespawnWorld()
	{
		if( !m_bSpawned )
		{
			return false;
		}

		// Despawn all entities
		for( auto It = m_ActiveEnts.begin(); It != m_ActiveEnts.end(); It++ )
		{
			if( It->second && It->second->IsValid() )
			{
				It->second->PerformDespawn();
			}
		}

		m_bSpawned = false;
		OnDespawn();

		return true;
	}

	/*-----------------------------------------------------------
		Entity List Methods
	-----------------------------------------------------------*/

	bool World::AddEntity( const HypPtr< Entity >& inEnt )
	{
		// Validate the entity
		if( !inEnt || !inEnt->IsValid() || inEnt->GetWorld() )
		{
			Console::WriteLine( "[WARNING] World: Attempt to add invalid entity (or already added to a world)" );
			return false;
		}

		m_ActiveEnts[ inEnt->GetIdentifier() ] = inEnt;
		inEnt->SetWorld( AquirePointer< World >() );

		OnEntityAdded( inEnt );
		
		// If this world is spawned.. and the target entity is not.. then we need to spawn this entity
		if( IsSpawned() && !inEnt->IsSpawned() )
		{
			inEnt->PerformSpawn();
		}

		return true;
	}

	bool World::AddEntityAt( const HypPtr< Entity >& inEnt, Transform3D inTransform )
	{
		// Validate Entity
		if( !inEnt || !inEnt->IsValid() || inEnt->GetWorld() )
		{
			Console::WriteLine( "[Warning] World: Attempt to add an invalid/existing entity to this world!" );
			return false;
		}

		// Set new transform
		inEnt->SetTransform( inTransform );

		// Add to entity list, and update entity state
		m_ActiveEnts[ inEnt->GetIdentifier() ] = inEnt;
		inEnt->SetWorld( AquirePointer< World >() );

		// Call hook(s)
		OnEntityAdded( inEnt );

		if( IsSpawned() && !inEnt->IsSpawned() )
		{
			inEnt->PerformSpawn();
		}

		return true;
	}

	bool World::RemoveEntity( const HypPtr< Entity >& inEnt )
	{
		// Validate the entity
		if( !inEnt || !inEnt->IsValid() )
		{
			Console::WriteLine( "[Warning] World: Attempt to remove an invalid entity from this world" );
			return false;
		}

		auto listEntry = m_ActiveEnts.find( inEnt->GetIdentifier() );
		if( listEntry == m_ActiveEnts.end() )
		{
			Console::WriteLine( "[Warning] World: Attempt to remove an entity that doesnt belong to this world!" );
			return false;
		}

		// Despawn the entity if needed
		if( inEnt->IsSpawned() )
		{
			inEnt->PerformDespawn();
		}

		// Next, call hook before actually removing from this world
		OnEntityRemoved( inEnt );

		inEnt->SetWorld( nullptr );
		m_ActiveEnts.erase( listEntry );

		return true;
	}

	HypPtr< Entity > World::GetEntity( uint32 inIdentifier )
	{
		auto listEntry = m_ActiveEnts.find( inIdentifier );
		if( listEntry == m_ActiveEnts.end() )
		{
			return nullptr;
		}

		return listEntry->second;
	}


	void World::OnSpawn()
	{

	}

	void World::OnDespawn()
	{

	}

	void World::OnSetActive()
	{

	}

	void World::OnSetDeactive()
	{

	}

	void World::OnEntityAdded( const HypPtr< Entity >& inEnt )
	{

	}

	void World::OnEntityRemoved( const HypPtr< Entity >& inEnt )
	{

	}

}


/*
*	Register Type
*/
HYPERION_REGISTER_OBJECT_TYPE( World, Object );