/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/World.h
	� 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"
#include "Hyperion/Framework/Entity.h"
#include "Hyperion/Core/Engine.h"
#include <map>
#include <memory>

namespace Hyperion
{

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
		bool AddEntityAt( const HypPtr< Entity >& inTarget, Transform3D inTransform );
		bool RemoveEntity( const HypPtr< Entity >& inTarget );

		HypPtr< Entity > GetEntity( uint32 inIdentifier );

	protected:

		virtual void OnSpawn();
		virtual void OnDespawn();
		virtual void OnSetActive();
		virtual void OnSetDeactive();
		virtual void OnEntityAdded( const HypPtr< Entity >& inEnt );
		virtual void OnEntityRemoved( const HypPtr< Entity >& inEnt );





		friend class Engine;

	};

}
