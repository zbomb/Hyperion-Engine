/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/MovementComponent.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/Component.h"


namespace Hyperion
{

	class MovementComponent : public Component
	{

	public:

		virtual bool HandleKeyBinding( const String& inBind );
		virtual bool HandleAxisBinding( const String& inBind, float inValue );

	};

}