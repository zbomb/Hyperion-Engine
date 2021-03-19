/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Simulation/Game/Components/NoClipMovementComponent.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Simulation/Game/Components/MovementComponent.h"


namespace Hyperion
{

	class NoClipMovementComponent : public MovementComponent
	{

	public:

		NoClipMovementComponent();

		// New input system
		void Move( const Vector3D& inVec ) override;
		void Look( const Vector2D& inVec ) override;

	};

}