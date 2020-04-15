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

		ViewState()
		{
			Position		= Vector3D( 0.f, 0.f, 0.f );
			Rotation		= Angle3D( 0.f, 0.f, 0.f );
			FOV				= 0.f;
		}

		bool operator==( const ViewState& Other ) const
		{
			return
				Position == Other.Position &&
				Rotation == Other.Rotation &&
				FOV == Other.FOV;
		}

		bool operator!=( const ViewState& Other ) const
		{
			return
				Position != Other.Position ||
				Rotation != Other.Rotation ||
				FOV != Other.FOV;
		}

		ViewState& operator=( const ViewState& Other )
		{
			Position = Other.Position;
			Rotation = Other.Rotation;
			FOV = Other.FOV;

			return *this;
		}
	};

}