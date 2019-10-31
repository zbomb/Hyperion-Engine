/*==================================================================================================
	Hyperion Engine
	Source/Core/Types/Color.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/Types/Color.h"


namespace Hyperion
{

	Color::Color()
		: Color( 0.f, 0.f, 0.f, 1.f )
	{
	}

	Color::Color( float inR, float inG, float inB )
		: Color( inR, inG, inB, 1.f )
	{
	}

	Color::Color( float inR, float inG, float inB, float inA )
		: R( inR ), G( inG ), B( inB ), A( inA )
	{
	}


	/*
		Basic Color Definitions
	*/
	const Color Color::Red( 1.f, 0.f, 0.f );
	const Color Color::Green( 0.f, 1.f, 0.f );
	const Color Color::Blue( 0.f, 0.f, 1.f );
	const Color Color::White( 1.f, 1.f, 1.f );
	const Color Color::Black( 0.f, 0.f, 0.f );
	const Color Color::Gray( 0.3f, 0.3f, 0.3f );



}