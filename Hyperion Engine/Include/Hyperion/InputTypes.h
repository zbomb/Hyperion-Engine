/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/InputTypes.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{
	// Special 'NONE' state
	constexpr uint32 INPUT_NONE					= 0;

	// Key binds for moving around
	constexpr uint32 INPUT_STATE_MOVE_FORWARD	= 1;
	constexpr uint32 INPUT_STATE_MOVE_BACKWARD	= 2;
	constexpr uint32 INPUT_STATE_MOVE_RIGHT		= 3;
	constexpr uint32 INPUT_STATE_MOVE_LEFT		= 4;

	// Input axis bindings
	constexpr uint32 INPUT_AXIS_LOOK_YAW		= 5;
	constexpr uint32 INPUT_AXIS_LOOK_PITCH		= 6;

}