/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/DataTypes/Transform.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Library/Math/Vector.h"
#include "Hyperion/Library/Math/Angle.h"

#include <memory>

namespace Hyperion
{

	struct Transform3D
	{

	public:

		/*
			Data Members
		*/
		Vector3D Position;
		Angle3D Rotation;
		Vector3D Scale;

		/*
			Constructors
		*/
		Transform3D( const Vector3D& inPosition = Vector3D(), const Angle3D& inRotation = Angle3D(), const Vector3D& inScale = Vector3D() )
			: Position( inPosition ), Rotation( inRotation ), Scale( inScale )
		{
		}

		Transform3D( const Transform3D& inOther )
			: Position( inOther.Position ), Rotation( inOther.Rotation ), Scale( inOther.Scale )
		{
		}

		Transform3D( Transform3D&& inOther ) noexcept
			: Position( std::move( inOther.Position ) ), Rotation( std::move( inOther.Rotation ) ), Scale( std::move( inOther.Scale ) )
		{
		}

		/*
			Assignment Operators
		*/
		Transform3D& operator=( const Transform3D& inOther )
		{
			Position	= inOther.Position;
			Rotation	= inOther.Rotation;
			Scale		= inOther.Scale;

			return *this;
		}

		Transform3D& operator=( Transform3D&& inOther ) noexcept
		{
			Position	= std::move( inOther.Position );
			Rotation	= std::move( inOther.Rotation );
			Scale		= std::move( inOther.Scale );

			return *this;
		}

		/*
			Arithmatic Operators
		*/
		Transform3D operator+( const Transform3D& inOther ) const
		{
			return Transform3D(
				Position + inOther.Position,
				Rotation + inOther.Rotation,
				Scale + inOther.Scale
			);
		}

		Transform3D operator-( const Transform3D& inOther ) const
		{
			return Transform3D(
				Position - inOther.Position,
				Rotation - inOther.Rotation,
				Scale - inOther.Scale
			);
		}

		/*
			Comparison Operators
		*/
		bool operator==( const Transform3D& inOther ) const
		{
			return( Position == inOther.Position &&
					Rotation == inOther.Rotation &&
					Scale == inOther.Scale );
		}

		bool operator!=( const Transform3D& inOther ) const
		{
			return( Position != inOther.Position ||
					Rotation != inOther.Rotation ||
					Scale != inOther.Scale );
		}

		/*
			Member Functions
		*/
		void Clear()
		{
			Position.Clear();
			Rotation.Clear();
			Scale.Clear();
		}

	};

}