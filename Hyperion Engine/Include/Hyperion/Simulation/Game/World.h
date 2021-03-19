/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Simulation/Game/World.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"
#include "Hyperion/Simulation/Game/ViewState.h"
#include "Hyperion/Simulation/Game/Entity.h"

#include <map>
#include <memory>


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class CameraComponent;


	class World : public Object
	{

	protected:

		/*
			Object Overrides
		*/
		void Initialize() final;
		void Shutdown() final;

	public:

		/*
			Entity Spawning
		*/

		World();
		~World();

	private:

		bool m_bSpawned;
		bool m_bActive;

		std::map< uint32, HypPtr< Entity > > m_ActiveEnts;

	public:

		inline bool IsSpawned() const	{ return m_bSpawned; }
		inline bool IsActive() const	{ return m_bActive; }

		bool SpawnWorld();
		bool DespawnWorld();

		bool AddEntity( const HypPtr< Entity >& inTarget );
		bool AddEntityAt( const HypPtr< Entity >& inTarget, Transform inTransform );
		bool RemoveEntity( const HypPtr< Entity >& inTarget );

		HypPtr< Entity > GetEntity( uint32 inIdentifier );

	protected:

		virtual void OnSpawn();
		virtual void OnDespawn();
		virtual void OnSetActive();
		virtual void OnSetDeactive();
		virtual void OnEntityAdded( const HypPtr< Entity >& inEnt );
		virtual void OnEntityRemoved( const HypPtr< Entity >& inEnt );

		friend class GameInstance;

	};

}
