/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/TransparencyTestEntity.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/Entity.h"
#include "Hyperion/Framework/StaticModelComponent.h"


namespace Hyperion
{

	class TransparencyTestEntity : public Entity
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