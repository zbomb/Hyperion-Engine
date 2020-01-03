/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/DataTypes/Color.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include <memory>

namespace Hyperion
{

	class Color
	{

	public:

		/*
			Data Members
		*/
		uint8 R, G, B, A;

		/*
			Constructors
		*/
		Color( uint8 inR = 0, uint8 inG = 0, uint8 inB = 0, uint8 inA = 255 )
			: R( inR ), G( inG ), B( inB ), A( inA )
		{
		}

		Color( const Color& Other )
			: R( Other.R ), G( Other.G ), B( Other.B ), A( Other.A )
		{
		}

		Color( Color&& Other ) noexcept
			: R( std::move( Other.R ) ), G( std::move( Other.G ) ), B( std::move( Other.B ) ), A( std::move( Other.A ) )
		{
		}

		/*
			Assignment Operators
		*/
		Color& operator=( const Color& Other )
		{
			R = Other.R;
			G = Other.G;
			B = Other.B;
			A = Other.A;

			return *this;
		}

		Color& operator=( Color&& Other ) noexcept
		{
			R = std::move( Other.R );
			G = std::move( Other.G );
			B = std::move( Other.B );
			A = std::move( Other.A );

			return *this;
		}

		/*
			Equality Operators
		*/
		bool operator==( const Color& Other ) const
		{
			return( R == Other.R &&
					G == Other.G &&
					B == Other.B &&
					A == Other.A );
		}

		bool operator!=( const Color& Other ) const
		{
			return( R != Other.R ||
					G != Other.G ||
					B != Other.B ||
					A != Other.A );
		}

		/*
			Scalar Getters
		*/
		float GetRScale() const
		{
			return( static_cast< float >( R ) / 255.f );
		}

		float GetGScale() const
		{
			return( static_cast< float >( G ) / 255.f );
		}

		float GetBScale() const
		{
			return( static_cast< float >( B ) / 255.f );
		}

		float GetAScale() const
		{
			return( static_cast< float >( A ) / 255.f );
		}

		void SetRScale( float inR )
		{
			if( inR < 0.f )			inR = 0.f;
			else if( inR > 1.f )	inR = 1.f;

			R = uint8( inR * 255.f );
		}

		void SetGScale( float inG )
		{
			if( inG < 0.f )			inG = 0.f;
			else if( inG > 1.f )	inG = 1.f;

			G = uint8( inG * 255.f );
		}

		void SetBScale( float inB )
		{
			if( inB < 0.f )			inB = 0.f;
			else if( inB > 1.f )	inB = 1.f;

			B = uint8( inB * 255.f );
		}

		void SetAScale( float inA )
		{
			if( inA < 0.f )			inA = 0.f;
			else if( inA > 1.f )	inA = 1.f;

			A = uint8( inA * 255.f );
		}

		void SetScales( float inR, float inG, float inB, float inA )
		{
			SetRScale( inR );
			SetGScale( inG );
			SetBScale( inB );
			SetAScale( inA );
		}

	};

}