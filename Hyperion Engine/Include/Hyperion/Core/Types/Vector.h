/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/DataTypes/Vector.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


namespace Hyperion
{

	class Vector2D
	{

	public:

		float X, Y;

		Vector2D( float inX = 0.f, float inY = 0.f );

		/*
			Operators
		*/
		Vector2D operator+( const Vector2D& Other );
		Vector2D operator-( const Vector2D& Other );
		void operator=( const Vector2D& Other );

	};

	class Vector3D
	{

	public:

		float X, Y, Z;

		Vector3D( float inX = 0.f, float inY = 0.f, float inZ = 0.f );

		/*
			Operators
		*/
		Vector3D operator+( const Vector3D& Other );
		Vector3D operator-( const Vector3D& Other );
		void operator=( const Vector3D& Other );
	};

	class Vector4D
	{

	public:

		float X, Y, Z, W;

		Vector4D( float inX = 0.f, float inY = 0.f, float inZ = 0.f, float inW = 0.f );

		/*
			Operators
		*/
		Vector4D operator+( const Vector4D& Other );
		Vector4D operator-( const Vector4D& Other );
		void operator=( const Vector4D& Other );

	};

}