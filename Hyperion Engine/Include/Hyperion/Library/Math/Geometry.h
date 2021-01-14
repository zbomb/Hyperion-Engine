/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Library/Math/Geometry.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Library/Math/Vector.h"
#include "Hyperion/Library/Math/Vertex.h"
#include "Hyperion/Framework/ViewState.h"
#include "Hyperion/Library/Math/Transform.h"

namespace Hyperion
{

	struct AABB
	{
		AABB()
			: Min(), Max()
		{}

		AABB( const AABB& Other )
			: Min( Other.Min ), Max( Other.Max )
		{}

		AABB( AABB&& Other ) noexcept
			: Min( std::move( Other.Min ) ), Max( std::move( Other.Max ) )
		{}

		AABB& operator=( const AABB& Other )
		{
			Max = Other.Max;
			Min = Other.Min;

			return *this;
		}

		AABB& operator=( AABB&& Other ) noexcept
		{
			Max = std::move( Other.Max );
			Min = std::move( Other.Min );

			return *this;
		}

		Vector3D Min;
		Vector3D Max;
	};

	struct BoundingSphere
	{
		BoundingSphere()
			: Center(), Radius( 0.f )
		{}

		BoundingSphere( const BoundingSphere& Other )
			: Center( Other.Center ), Radius( Other.Radius )
		{}

		BoundingSphere( BoundingSphere&& Other ) noexcept
			: Center( std::move( Other.Center ) ), Radius( std::move( Other.Radius ) )
		{}

		BoundingSphere& operator=( const BoundingSphere& Other )
		{
			Center = Other.Center;
			Radius = Other.Radius;

			return *this;
		}

		BoundingSphere& operator=( BoundingSphere&& Other ) noexcept
		{
			Center = std::move( Other.Center );
			Radius = std::move( Other.Radius );

			return *this;
		}

		Vector3D Center;
		float Radius;
	};


	#pragma pack( push, 1 )
	struct Matrix
	{
		float data[ 16 ];

		Matrix()
			: data{ 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f }
		{}

		Matrix( float ia0, float ia1, float ia2, float ia3,
				 float ib0, float ib1, float ib2, float ib3,
				 float ic0, float ic1, float ic2, float ic3,
				 float id0, float id1, float id2, float id3 )
		{
			data[ 0 ] = ia0;
			data[ 1 ] = ia1;
			data[ 2 ] = ia2;
			data[ 3 ] = ia3;
			data[ 4 ] = ib0;
			data[ 5 ] = ib1;
			data[ 6 ] = ib2;
			data[ 7 ] = ib3;
			data[ 8 ] = ic0;
			data[ 9 ] = ic1;
			data[ 10 ] = ic2;
			data[ 11 ] = ic3;
			data[ 12 ] = id0;
			data[ 13 ] = id1;
			data[ 14 ] = id2;
			data[ 15 ] = id3;
		}

		Matrix( const float* in )
		{
			memcpy_s( data, 16 * sizeof( float ), in, 16 * sizeof( float ) );
		}

		const float* GetData() const { return data; }

		void operator=( const float* inData )
		{
			memcpy_s( data, 16 * sizeof( float ), inData, 16 * sizeof( float ) );
		}

		void AssignData( const float* in ) { this->operator=( in ); }
	};
	#pragma pack( pop )


	class GeometryLibrary
	{

	public:

		GeometryLibrary() = delete;

		/*
			CalculateScreenSizeInPixels
			* Returns the DIAMETER of the sphere in screen space pixels
		*/
		static float CalculateScreenSizeInPixels( const Vector3D& inViewPos, float inFOV, const BoundingSphere& inBounds, uint32 ScreenHeight );
		static float CalculateScreenSizeInPixels( const ViewState& inView, const BoundingSphere& inBounds, uint32 ScreenHeight );

		static Vector3D GetDirectionVectorFromAngle( const Angle3D& inAngle );

	};

}
