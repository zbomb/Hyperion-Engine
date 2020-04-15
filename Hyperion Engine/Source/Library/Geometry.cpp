/*==================================================================================================
	Hyperion Engine
	Source/Library/Geometry.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Library/Geometry.h"
#include "Hyperion/Framework/ViewState.h"



namespace Hyperion
{

	float Geometry::CalculateScreenSizeInPixels( const Vector3D& inViewPos, float inFOV, const BoundingSphere& inBounds, uint32 ScreenHeight )
	{
		// Calculate distance from the screen origin to the center of the boudning sphere
		float fovHalf = inFOV / 2.f;
		float dist = Vector3D::Distance( inViewPos, inBounds.Center );
		float num = dist == 0.f ? 100000000.f : ( inBounds.Radius * ( cosf( fovHalf ) / sinf( fovHalf ) ) ) / dist;

		// If we wanted the radius instead of the diameter, we would half this result
		return num * (float) ( ScreenHeight );
	}


	float Geometry::CalculateScreenSizeInPixels( const ViewState& inView, const BoundingSphere& inBounds, uint32 ScreenHeight )
	{
		return CalculateScreenSizeInPixels( inView.Position, inView.FOV, inBounds, ScreenHeight );
	}

}