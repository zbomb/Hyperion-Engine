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

		// New input system
		virtual void MoveForward( float inScalar );
		virtual void MoveRight( float inScalar );
		virtual void LookUp( float inScalar );
		virtual void LookRight( float inScalar );

		// Old input system
		virtual bool HandleKeyBinding( const String& inBind );
		virtual bool HandleAxisBinding( const String& inBind, float inValue );

	};

}