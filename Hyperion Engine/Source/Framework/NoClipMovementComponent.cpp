/*==================================================================================================
	Hyperion Engine
	Source/Framework/NoClipMovementComponent.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/NoClipMovementComponent.h"
#include "Hyperion/Framework/Entity.h"
#include "Hyperion/Library/Geometry.h"
#include "Hyperion/Framework/Character.h"
#include "Hyperion/Framework/CameraComponent.h"
#include "Hyperion/Library/Math.h"


namespace Hyperion
{

	NoClipMovementComponent::NoClipMovementComponent()
	{

	}


	// New Input System
	void NoClipMovementComponent::Move( const Vector3D& inVec )
	{
		auto owner = GetOwner();
		if( owner && owner->IsSpawned() )
		{
			auto charPtr = CastObject< Character >( owner );
			if( charPtr )
			{
				// We want to build a plane from the eye direction vector, and calculate a perpendicular unit vector
				// that will point to the right, the plane will be perpendicular to the X,Z plane itself
				auto eyeDir		= charPtr->GetEyeDirection().GetDirectionVector();
				auto rightDir	= -( eyeDir ^ Vector3D::GetWorldUp() ).GetNormalized();
				auto upDir		= Vector3D::GetWorldUp();

				owner->Translate( ( eyeDir * inVec.X  + rightDir * inVec.Y + upDir * inVec.Z ) * 0.25f );

			}
			else
			{
				// Fall back on using the rotation of the entity itself...
				auto ownerEuler		= owner->GetQuaternion().GetEulerAngles();
				auto ownerFwd		= ownerEuler.GetDirectionVector();
				auto ownerRight		= Angle3D( 0.f, ownerEuler.Yaw + 90.f, 0.f ).GetDirectionVector();
				auto upDir			= Vector3D::GetWorldUp();

				owner->Translate( ( ownerFwd * inVec.X + ownerRight * inVec.Y + upDir * inVec.Z ) * 0.25f );
			}
		}
	}

	
	void NoClipMovementComponent::Look( const Vector2D& inVec )
	{
		auto owner = GetOwner();
		if( owner && owner->IsSpawned() )
		{
			auto characterPtr = CastObject< Character >( owner );
			if( characterPtr )
			{
				// A character may (probably) have special handling for look direction
				auto eyeDirection = characterPtr->GetEyeDirection();
				bool bClampDown = eyeDirection.Pitch > 85.f;
				eyeDirection.Pitch	+= inVec.X;
				eyeDirection.Yaw	+= inVec.Y;

				// There is a slight issue.. angles get pulled into the range of [0,360) 
				// So, we cant use a normal clamp function to constrain camrea pitch to the desired range
				if( bClampDown )
				{
					// If were looking down, the min pitch would be -85, or.. 360 - 85 => 275 degrees
					if( eyeDirection.Pitch < 275.f ) { eyeDirection.Pitch = 275.f; }
				}
				else
				{
					// If were looking up, the max pitch is 85
					if( eyeDirection.Pitch > 85.f ) { eyeDirection.Pitch = 85.f; }
				}

				characterPtr->SetEyeDirection( eyeDirection.Pitch, eyeDirection.Yaw );
			}
			else
			{
				auto ownerRotation = owner->GetRotation();
				bool bClampDown = ownerRotation.Pitch > 85.f;
				ownerRotation.Pitch += inVec.X;
				ownerRotation.Yaw += inVec.Y;

				if( bClampDown )
				{
					if( ownerRotation.Pitch < 275.f ) { ownerRotation.Pitch = 275.f; }
				}
				else
				{
					if( ownerRotation.Pitch > 85.f ) { ownerRotation.Pitch = 85.f; }
				}

				owner->SetRotation( Angle3D( ownerRotation.Pitch, ownerRotation.Yaw, 0.f ) );
			}
		}
	}

}

