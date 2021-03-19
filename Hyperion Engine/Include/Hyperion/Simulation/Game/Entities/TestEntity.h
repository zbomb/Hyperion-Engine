/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Simulation/Game/Entities/TestEntity.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Simulation/Game/Entity.h"
#include "Hyperion/Simulation/Game/Components/StaticModelComponent.h"


namespace Hyperion
{

	class TestEntity : public Entity
	{

	protected:

		HypPtr< StaticModelComponent > m_Comp;

		void OnCreate() final;
		void OnDestroy() final;

		void OnSpawn( const HypPtr< World >& inWorld ) override;
		void OnDespawn( const HypPtr< World >& inWorld ) override;

		void Tick( double delta ) override;

	};

}