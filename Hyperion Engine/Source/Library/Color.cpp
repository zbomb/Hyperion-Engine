/*==================================================================================================
	Hyperion Engine
	Source/Library/Color.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Library/Color.h"


namespace Hyperion
{

	/*
	*	Color3F
	*/
	Color3F::Color3F()
		: Color3F( 0.f, 0.f, 0.f )
	{}

	Color3F::Color3F( float inR, float inG, float inB )
		: r( Math::Clamp( inR, 0.f, 1.f ) ), g( Math::Clamp( inG, 0.f, 1.f ) ), b( Math::Clamp( inB, 0.f, 1.f ) )
	{}

	Color3F::Color3F( const Color4F& in )
		: r( in.r ), g( in.g ), b( in.b )
	{}

	Color3F::Color3F( const Color3B& in )
		: r( (float) in.r / 255.f ), g( (float) in.g / 255.f ), b( (float) in.b / 255.f )
	{}

	Color3F::Color3F( const Color4B& in )
		: r( (float) in.r / 255.f ), g( (float) in.g / 255.f ), b( (float) in.b / 255.f )
	{}


	/*
	*	Color4F
	*/
	Color4F::Color4F()
		: Color4F( 0.f, 0.f, 0.f, 255.f )
	{}

	Color4F::Color4F( float inR, float inG, float inB, float inA )
		: r( Math::Clamp( inR, 0.f, 1.f ) ), g( Math::Clamp( inG, 0.f, 1.f ) ), b( Math::Clamp( inB, 0.f, 1.f ) ), a( Math::Clamp( inA, 0.f, 1.f ) )
	{}

	Color4F::Color4F( const Color3F& in )
		: r( in.r ), g( in.g ), b( in.b ), a( 255.f )
	{}

	Color4F::Color4F( const Color3B& in )
		: r( (float) in.r / 255.f ), g( (float) in.g / 255.f ), b( (float) in.b / 255.f ), a( 255.f )
	{}

	Color4F::Color4F( const Color4B& in )
		: r( (float) in.r / 255.f ), g( (float) in.g / 255.f ), b( (float) in.b / 255.f ), a( (float) in.a / 255.f )
	{}


	/*
	*	Color3B
	*/
	Color3B::Color3B()
		: Color3B( 0, 0, 0 )
	{}

	Color3B::Color3B( uint8 inR, uint8 inG, uint8 inB )
		: r( inR ), g( inG ), b( inB )
	{}

	Color3B::Color3B( const Color4F& in )
		: r( (byte)( in.r * 255.f ) ), g( (byte)( in.g * 255.f ) ), b( (byte)( in.b * 255.f ) )
	{}

	Color3B::Color3B( const Color3F& in )
		: r( (byte)( in.r * 255.f ) ), g( (byte)( in.g * 255.f ) ), b( (byte)( in.b * 255.f ) )
	{}

	Color3B::Color3B( const Color4B& in )
		: r( in.r ), g( in.g ), b( in.b )
	{}


	/*
	*	Color4B
	*/
	Color4B::Color4B()
		: Color4B( 0, 0, 0, 255 )
	{}

	Color4B::Color4B( uint8 inR, uint8 inG, uint8 inB, uint8 inA )
		: r( inR ), g( inG ), b( inB ), a( inA )
	{}

	Color4B::Color4B( const Color3F& in )
		: r( (byte)( in.r * 255.f ) ), g( (byte)( in.g * 255.f ) ), b( (byte)( in.b * 255.f ) ), a( 255 )
	{}

	Color4B::Color4B( const Color3B& in )
		: r( in.r ), g( in.g ), b( in.b ), a( 255 )
	{}

	Color4B::Color4B( const Color4F& in )
		: r( (byte)( in.r * 255.f ) ), g( (byte)( in.g * 255.f ) ), b( (byte)( in.b * 255.f ) ), a( (byte)( in.a * 255.f ) )
	{}

}