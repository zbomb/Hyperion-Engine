/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/DataTypes/Vector.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <memory>

namespace Hyperion
{

	class Vector2D
	{

	public:

		/*
			Data Members
		*/
		float X, Y;

		/*
			Constructors
		*/
		Vector2D( float inX = 0.f, float inY = 0.f )
			: X( inX ), Y( inY )
		{
		}

		Vector2D( const Vector2D& inOther )
			: X( inOther.X ), Y( inOther.Y )
		{
		}

		Vector2D( Vector2D&& inOther ) noexcept
			: X( std::move( inOther.X ) ), Y( std::move( inOther.Y ) )
		{
		}

		/*
			Operators
		*/
		Vector2D operator+( const Vector2D& Other ) const
		{
			return Vector2D(
				X + Other.X,
				Y + Other.Y
			);
		}

		Vector2D operator-( const Vector2D& Other ) const
		{
			return Vector2D(
				X - Other.X,
				Y - Other.Y
			);
		}

		Vector2D operator*( float inScale ) const
		{
			return Vector2D(
				X * inScale,
				Y * inScale
			);
		}

		Vector2D operator/( float inScale ) const
		{
			return Vector2D(
				X / inScale,
				Y / inScale
			);
		}

		/*
			Assignment
		*/
		Vector2D& operator=( const Vector2D& Other )
		{
			X = Other.X;
			Y = Other.Y;

			return *this;
		}

		Vector2D& operator=( Vector2D&& Other ) noexcept
		{
			X = std::move( Other.X );
			Y = std::move( Other.Y );

			return *this;
		}

		/*
			Comparison Operators
		*/
		bool operator==( const Vector2D& inOther ) const
		{
			return( X == inOther.X && Y == inOther.Y );
		}

		bool operator!=( const Vector2D& inOther ) const
		{
			return( X != inOther.X || Y != inOther.Y );
		}

		/*
			Static Functions
		*/
		static float Distance( const Vector2D& First, const Vector2D& Second )
		{
			float sqX = powf( Second.X - First.X, 2.f );
			float sqY = powf( Second.Y - First.Y, 2.f );

			return sqrtf( sqX + sqY );
		}

		/*
			Member Functions
		*/
		void Clear()
		{
			X = 0.f;
			Y = 0.f;
		}

		inline float Distance( const Vector2D& Other )
		{
			return Vector2D::Distance( *this, Other );
		}

	};

	class Vector3D
	{

	public:

		/*
			Data Members
		*/
		float X, Y, Z;

		/*
			Constructors
		*/
		Vector3D( float inX = 0.f, float inY = 0.f, float inZ = 0.f )
			: X( inX ), Y( inY ), Z( inZ )
		{
		}

		Vector3D( const Vector3D& inOther )
			: X( inOther.X ), Y( inOther.Y ), Z( inOther.Z )
		{
		}

		Vector3D( Vector3D&& inOther ) noexcept
			: X( std::move( inOther.X ) ), Y( std::move( inOther.Y ) ), Z( std::move( inOther.Z ) )
		{
		}

		/*
			Operators
		*/
		Vector3D operator+( const Vector3D& Other ) const
		{
			return Vector3D(
				X + Other.X,
				Y + Other.Y,
				Z + Other.Z
			);
		}

		Vector3D operator-( const Vector3D& Other ) const
		{
			return Vector3D(
				X - Other.X,
				Y - Other.Y,
				Z - Other.Z
			);
		}

		Vector3D operator*( float inScale ) const
		{
			return Vector3D(
				X * inScale,
				Y * inScale,
				Z * inScale
			);
		}

		Vector3D operator/( float inScale ) const
		{
			return Vector3D(
				X / inScale,
				Y / inScale,
				Z / inScale
			);
		}

		/*
			Assignment
		*/
		Vector3D& operator=( const Vector3D& Other )
		{
			X = Other.X;
			Y = Other.Y;
			Z = Other.Z;

			return *this;
		}

		Vector3D& operator=( Vector3D&& Other ) noexcept
		{
			X = std::move( Other.X );
			Y = std::move( Other.Y );
			Z = std::move( Other.Z );

			return *this;
		}

		/*
			Comparison Operators
		*/
		bool operator==( const Vector3D& inOther ) const
		{
			return( X == inOther.X &&
					Y == inOther.Y &&
					Z == inOther.Z );
		}

		bool operator!=( const Vector3D& inOther ) const
		{
			return( X != inOther.X ||
					Y != inOther.Y ||
					Z != inOther.Z );
		}

		/*
			Static Functions
		*/
		static float Distance( const Vector3D& First, const Vector3D& Second )
		{
			float sqX = powf( Second.X - First.X, 2.f );
			float sqY = powf( Second.Y - First.Y, 2.f );
			float sqZ = powf( Second.Z - First.Z, 2.f );

			return sqrtf( sqX + sqY + sqZ );
		}

		/*
			Member Function
		*/
		void Clear()
		{
			X = 0.f;
			Y = 0.f;
			Z = 0.f;
		}

		inline float Distance( const Vector3D& Other )
		{
			return Vector3D::Distance( *this, Other );
		}
	};

	class Vector4D
	{

	public:

		/*
			Data Members
		*/
		float X, Y, Z, W;

		/*
			Constructors
		*/
		Vector4D( float inX = 0.f, float inY = 0.f, float inZ = 0.f, float inW = 0.f )
			: X( inX ), Y( inY ), Z( inZ ), W( inW )
		{
		}

		Vector4D( const Vector4D& inOther )
			: X( inOther.X ), Y( inOther.Y ), Z( inOther.Z ), W( inOther.W )
		{
		}

		Vector4D( Vector4D&& inOther ) noexcept
			: X( std::move( inOther.X ) ), Y( std::move( inOther.Y ) ), Z( std::move( inOther.Z ) ), W( std::move( inOther.W ) )
		{
		}

		/*
			Operators
		*/
		Vector4D operator+( const Vector4D& Other ) const
		{
			return Vector4D(
				X + Other.X,
				Y + Other.Y,
				Z + Other.Z,
				W + Other.W
			);
		}

		Vector4D operator-( const Vector4D& Other ) const
		{
			return Vector4D(
				X - Other.X,
				Y - Other.Y,
				Z - Other.Z,
				W - Other.W
			);
		}

		Vector4D operator*( float inScale ) const
		{
			return Vector4D(
				X * inScale,
				Y * inScale,
				Z * inScale,
				W * inScale
			);
		}

		Vector4D operator/( float inScale ) const
		{
			return Vector4D(
				X / inScale,
				Y / inScale,
				Z / inScale,
				W / inScale
			);
		}

		/*
			Assignment
		*/
		Vector4D& operator=( Vector4D&& Other ) noexcept
		{
			X = std::move( Other.X );
			Y = std::move( Other.Y );
			Z = std::move( Other.Z );
			W = std::move( Other.W );

			return *this;
		}

		Vector4D& operator=( const Vector4D& Other )
		{
			X = Other.X;
			Y = Other.Y;
			Z = Other.Z;
			W = Other.W;

			return *this;
		}

		/*
			Comparison Operators
		*/
		bool operator==( const Vector4D& Other ) const
		{
			return( X == Other.X &&
					Y == Other.Y &&
					Z == Other.Z &&
					W == Other.W );
		}

		bool operator!=( const Vector4D& Other ) const
		{
			return( X != Other.X ||
					Y != Other.Y ||
					Z != Other.Z ||
					W != Other.W );
		}

		/*
			Static Functions
		*/
		static float Distance( const Vector4D& First, const Vector4D& Second )
		{
			float sqX = powf( Second.X - First.X, 2.f );
			float sqY = powf( Second.Y - First.Y, 2.f );
			float sqZ = powf( Second.Z - First.Z, 2.f );
			float sqW = powf( Second.W - First.W, 2.f );

			return sqrtf( sqX + sqY + sqZ + sqW );
		}

		/*
			Member Functions
		*/
		void Clear()
		{
			X = 0.f;
			Y = 0.f;
			Z = 0.f;
			W = 0.f;
		}

		float Distance( const Vector4D& Other )
		{
			return Vector4D::Distance( *this, Other );
		}

	};

}