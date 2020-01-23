/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/TestEntity.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/Entity.h"


namespace Hyperion
{

	class TestEntity : public Entity
	{

	protected:

		void OnSpawn( const HypPtr< World >& inWorld ) override;
		void OnDespawn( const HypPtr< World >& inWorld ) override;

	};

}