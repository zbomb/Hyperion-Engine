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
	constexpr uint32 INPUT_STATE_LOOK_UP		= 7;
	constexpr uint32 INPUT_STATE_LOOK_RIGHT		= 8;
	constexpr uint32 INPUT_STATE_LOOK_LEFT		= 9;
	constexpr uint32 INPUT_STATE_LOOK_DOWN		= 10;
	constexpr uint32 INPUT_STATE_SPRINT			= 11;
	constexpr uint32 INPUT_STATE_MOVE_UP		= 12;
	constexpr uint32 INPUT_STATE_MOVE_DOWN		= 13;

	// Input axis bindings
	constexpr uint32 INPUT_AXIS_LOOK_YAW		= 5;
	constexpr uint32 INPUT_AXIS_LOOK_PITCH		= 6;

}