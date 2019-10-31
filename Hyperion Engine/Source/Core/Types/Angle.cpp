/*==================================================================================================
	Hyperion Engine
	Source/Core/Types/Angle.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/Types/Angle.h"


namespace Hyperion
{

	Angle3D::Angle3D( float inPitch /* = 0.f */, float inYaw /* = 0.f */, float inRoll /* = 0.f */ )
		: Pitch( inPitch ), Yaw( inYaw ), Roll( inRoll )
	{
	}


	Angle3D Angle3D::operator+( const Angle3D& Other )
	{
		return Angle3D( Pitch + Other.Pitch, Yaw + Other.Yaw, Roll + Other.Roll );
	}

	Angle3D Angle3D::operator-( const Angle3D& Other )
	{
		return Angle3D( Pitch - Other.Pitch, Yaw - Other.Yaw, Roll - Other.Roll );
	}

	void Angle3D::operator=( const Angle3D& Other )
	{
		Pitch	= Other.Pitch;
		Yaw		= Other.Yaw;
		Roll	= Other.Roll;
	}


}