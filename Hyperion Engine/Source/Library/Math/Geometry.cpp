/*==================================================================================================
	Hyperion Engine
	Source/Library/Geometry.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Library/Math/Geometry.h"
#include "Hyperion/Framework/ViewState.h"



namespace Hyperion
{

	float GeometryLibrary::CalculateScreenSizeInPixels( const Vector3D& inViewPos, float inFOV, const BoundingSphere& inBounds, uint32 ScreenHeight )
	{
		// Calculate distance from the screen origin to the center of the boudning sphere
		float fovHalf = inFOV / 2.f;
		float dist = Vector3D::Distance( inViewPos, inBounds.Center );
		float num = dist == 0.f ? 100000000.f : ( inBounds.Radius * ( cosf( fovHalf ) / sinf( fovHalf ) ) ) / dist;

		// If we wanted the radius instead of the diameter, we would half this result
		return num * (float) ( ScreenHeight );
	}


	float GeometryLibrary::CalculateScreenSizeInPixels( const ViewState& inView, const BoundingSphere& inBounds, uint32 ScreenHeight )
	{
		return CalculateScreenSizeInPixels( inView.Position, inView.FOV, inBounds, ScreenHeight );
	}


	Vector3D GeometryLibrary::GetDirectionVectorFromAngle( const Angle3D& inAngle )
	{
		float pitch		= inAngle.Pitch;
		float yaw		= inAngle.Yaw;

		// Clamp the angle into [0,360) degrees
		while( pitch >= 360.f ) { pitch -= 360.f; }
		while( yaw >= 360.f ) { yaw -= 360.f; }
		while( pitch < 0.f ) { pitch += 360.f; }
		while( yaw < 0.f ) { yaw += 360.f; }

		// Convert to radians
		pitch	= HYPERION_DEG_TO_RAD( pitch );
		yaw		= HYPERION_DEG_TO_RAD( yaw );

		return Vector3D(
			sinf( yaw ) * cosf( pitch ),
			-sinf( pitch ),
			cosf( yaw ) * cosf( pitch )
		);
	}

}