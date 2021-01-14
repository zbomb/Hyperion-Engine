/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/DataTypes/Angle.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <memory>

namespace Hyperion
{

	class Angle3D
	{

	public:

		/*
			Data Members
		*/
		float Pitch, Yaw, Roll;

		/*
			Constructors
		*/
		Angle3D( float inPitch = 0.f, float inYaw = 0.f, float inRoll = 0.f )
			: Pitch( inPitch ), Yaw( inYaw ), Roll( inRoll )
		{
		}

		Angle3D( const Angle3D& inOther )
			: Pitch( inOther.Pitch ), Yaw( inOther.Yaw ), Roll( inOther.Roll )
		{
		}

		Angle3D( Angle3D&& inOther ) noexcept
			: Pitch( std::move( inOther.Pitch ) ), Yaw( std::move( inOther.Yaw ) ), Roll( std::move( inOther.Roll ) )
		{
		}

		/*
			Operators
		*/
		Angle3D operator+( const Angle3D& inOther ) const
		{
			return Angle3D(
				Pitch + inOther.Pitch,
				Yaw + inOther.Yaw,
				Roll + inOther.Roll
			);
		}

		Angle3D operator-( const Angle3D& inOther ) const
		{
			return Angle3D(
				Pitch - inOther.Pitch,
				Yaw - inOther.Yaw,
				Roll - inOther.Roll
			);
		}

		/*
			Assignment Operators
		*/
		Angle3D& operator=( const Angle3D& Other )
		{
			Pitch = Other.Pitch;
			Yaw = Other.Yaw;
			Roll = Other.Roll;

			return *this;
		}

		Angle3D& operator=( Angle3D&& Other ) noexcept
		{
			Pitch = std::move( Other.Pitch );
			Yaw = std::move( Other.Yaw );
			Roll = std::move( Other.Roll );

			return *this;
		}

		/*
			Comparison Operators
		*/
		bool operator==( const Angle3D& Other ) const
		{
			return( Pitch == Other.Pitch &&
					Yaw == Other.Yaw &&
					Roll == Other.Roll );
		}

		bool operator!=( const Angle3D& Other ) const
		{
			return( Pitch != Other.Pitch ||
					Yaw != Other.Yaw ||
					Roll != Other.Roll );
		}

		/*
			Member Functions
		*/
		void Clear()
		{
			Pitch = 0.f;
			Yaw = 0.f;
			Roll = 0.f;
		}


		void ClampContents()
		{
			// We want to get each component into the range of 0 to 360, where 0 is inclusive
			while( Pitch >= 360.f ) { Pitch -= 360.f; }
			while( Yaw >= 360.f ) { Yaw -= 360.f; }
			while( Roll >= 360.f ) { Roll -= 360.f; }
			while( Pitch < 0.f ) { Pitch += 360.f; }
			while( Yaw < 0.f ) { Yaw += 360.f; }
			while( Roll < 0.f ) { Roll += 360.f; }
		}

	};

}