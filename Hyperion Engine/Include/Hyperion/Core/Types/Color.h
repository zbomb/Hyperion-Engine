/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/DataTypes/Color.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


namespace Hyperion
{

	class Color
	{

	public:

		float R, G, B, A;

		Color();
		Color( float inR, float inG, float inB );
		Color( float inR, float inG, float inB, float inA );

		/*
			Basic Color Definitions
		*/
		const static Color Red;
		const static Color Green;
		const static Color Blue;
		const static Color White;
		const static Color Black;
		const static Color Gray;

	};

}