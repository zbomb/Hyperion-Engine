/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Simulation/Game/Components/MovementComponent.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Simulation/Game/Component.h"


namespace Hyperion
{

	class MovementComponent : public Component
	{

	public:

		// New input system
		virtual void Move( const Vector3D& inVec );
		virtual void Look( const Vector2D& inVec );

	};

}