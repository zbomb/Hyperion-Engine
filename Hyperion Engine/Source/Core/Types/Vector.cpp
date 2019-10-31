/*==================================================================================================
	Hyperion Engine
	Source/Core/Types/Vector.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/Types/Vector.h"


namespace Hyperion
{

	/*-----------------------------------------------------------------------------
		Vector 2D
	-----------------------------------------------------------------------------*/
	Vector2D::Vector2D( float inX /* = 0.f */, float inY /* = 0.f */ )
		: X( inX ), Y( inY )
	{

	}

	Vector2D Vector2D::operator+( const Vector2D& Other )
	{
		return Vector2D( X + Other.X, Y + Other.Y );
	}

	Vector2D Vector2D::operator-( const Vector2D& Other )
	{
		return Vector2D( X - Other.X, Y - Other.Y );
	}

	void Vector2D::operator=( const Vector2D& Other )
	{
		X = Other.X;
		Y = Other.Y;
	}
	

	/*-----------------------------------------------------------------------------
		Vector 3D
	-----------------------------------------------------------------------------*/
	Vector3D::Vector3D( float inX /* = 0.f */, float inY /* = 0.f */, float inZ /* = 0.f */ )
		: X( inX ), Y( inY ), Z( inZ )
	{

	}

	Vector3D Vector3D::operator+( const Vector3D& Other )
	{
		return Vector3D( X + Other.X, Y + Other.Y, Z + Other.Z );
	}

	Vector3D Vector3D::operator-( const Vector3D& Other )
	{
		return Vector3D( X - Other.X, Y - Other.Y, Z - Other.Z );
	}

	void Vector3D::operator=( const Vector3D& Other )
	{
		X = Other.X;
		Y = Other.Y;
		Z = Other.Z;
	}


	/*-----------------------------------------------------------------------------
		Vector 4D
	-----------------------------------------------------------------------------*/
	Vector4D::Vector4D( float inX /* = 0.f */, float inY /* = 0.f */, float inZ /* = 0.f */, float inW /* = 0.f */ )
		: X( inX ), Y( inY ), Z( inZ ), W( inW )
	{

	}

	Vector4D Vector4D::operator+( const Vector4D& Other )
	{
		return Vector4D( X + Other.X, Y + Other.Y, Z + Other.Z, W + Other.W );
	}

	Vector4D Vector4D::operator-( const Vector4D& Other )
	{
		return Vector4D( X - Other.X, Y - Other.Y, Z - Other.Z, W - Other.W );
	}

	void Vector4D::operator=( const Vector4D& Other )
	{
		X = Other.X;
		Y = Other.Y;
		Z = Other.Z;
		W = Other.W;
	}
}