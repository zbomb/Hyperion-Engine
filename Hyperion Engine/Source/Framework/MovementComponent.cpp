/*==================================================================================================
	Hyperion Engine
	Source/Framework/MovementComponent.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/MovementComponent.h"
#include "Hyperion/Framework/Character.h"


namespace Hyperion
{

	bool MovementComponent::HandleKeyBinding( const String& inBind )
	{
		return false;
	}


	bool MovementComponent::HandleAxisBinding( const String& inBind, float inValue )
	{
		return false;
	}

}

HYPERION_REGISTER_OBJECT_TYPE( MovementComponent, Component );