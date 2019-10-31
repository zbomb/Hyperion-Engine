/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/DataTypes/Angle.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

namespace Hyperion
{

	class Angle3D
	{

	public:

		float Pitch, Yaw, Roll;

		Angle3D( float inPitch = 0.f, float inYaw = 0.f, float inRoll = 0.f );

		/*
			Operators
		*/
		Angle3D operator+( const Angle3D& Other );
		Angle3D operator-( const Angle3D& Other );
		void operator=( const Angle3D& Other );

	};

}