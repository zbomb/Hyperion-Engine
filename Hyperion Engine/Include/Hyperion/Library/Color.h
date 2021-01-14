/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Library/Color.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Library/Math/MathCore.h"


namespace Hyperion
{
	/*
	*	Forward Declarations
	*/
	struct Color4F;
	struct Color3B;
	struct Color4B;


	struct Color3F
	{
		float r, g, b;

		Color3F();
		Color3F( float inR, float inG, float inB );
		Color3F( const Color4F& in );
		Color3F( const Color3B& in );
		Color3F( const Color4B& in );

	};


	struct Color4F
	{
		float r, g, b, a;

		Color4F();
		Color4F( float inR, float inG, float inB, float inA );
		Color4F( const Color3F& in );
		Color4F( const Color3B& in );
		Color4F( const Color4B& in );

	};

	struct Color3B
	{
		uint8 r, g, b;

		Color3B();
		Color3B( uint8 inR, uint8 inG, uint8 inB );
		Color3B( const Color4F& in );
		Color3B( const Color3F& in );
		Color3B( const Color4B& in );

	};

	struct Color4B
	{
		uint8 r, g, b, a;

		Color4B();
		Color4B( uint8 inR, uint8 inG, uint8 inB, uint8 inA );
		Color4B( const Color3F& in );
		Color4B( const Color3B& in );
		Color4B( const Color4F& in );

	};

}