/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/ViewState.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

// Hyperion Includes
#include "Hyperion/Hyperion.h"

namespace Hyperion
{

	struct ViewState
	{
		Vector3D Position;
		Angle3D Rotation;
		float FOV;
		float AspectRatio;

		ViewState()
		{
			Position		= Vector3D( 0.f, 0.f, 0.f );
			Rotation		= Angle3D( 0.f, 0.f, 0.f );
			FOV				= 0.f;
			AspectRatio		= 0.f;
		}
	};

}