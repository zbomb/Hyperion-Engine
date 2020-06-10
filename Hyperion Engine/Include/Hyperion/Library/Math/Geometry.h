/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Library/Math/Geometry.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Library/Math/Vector.h"
#include "Hyperion/Framework/ViewState.h"

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


	class Geometry
	{

	public:

		Geometry() = delete;

		/*
			CalculateScreenSizeInPixels
			* Returns the DIAMETER of the sphere in screen space pixels
		*/
		static float CalculateScreenSizeInPixels( const Vector3D& inViewPos, float inFOV, const BoundingSphere& inBounds, uint32 ScreenHeight );
		static float CalculateScreenSizeInPixels( const ViewState& inView, const BoundingSphere& inBounds, uint32 ScreenHeight );

	};

}
