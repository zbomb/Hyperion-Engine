/*==================================================================================================
	Hyperion Engine
	Source/Framework/NoClipMovementComponent.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/NoClipMovementComponent.h"
#include "Hyperion/Framework/Entity.h"
#include "Hyperion/Library/Geometry.h"



namespace Hyperion
{

	NoClipMovementComponent::NoClipMovementComponent()
	{
		bRequiresTick = true;
		m_bFwd = false;
		m_bLeft = false;
		m_bRight = false;
		m_bBwd = false;
	}


	// New Input System
	void NoClipMovementComponent::MoveForward( float inScalar )
	{

	}


	void NoClipMovementComponent::MoveRight( float inScalar )
	{

	}


	void NoClipMovementComponent::LookUp( float inScalar )
	{

	}


	void NoClipMovementComponent::LookRight( float inScalar )
	{

	}


	// Old Input System
	bool NoClipMovementComponent::HandleKeyBinding( const String& inCommand )
	{
		if( inCommand == "+forward" )
		{
			m_bFwd = true;
		}
		else if( inCommand == "-forward" )
		{
			m_bFwd = false;
		}
		else if( inCommand == "+left" )
		{
			m_bLeft = true;
		}
		else if( inCommand == "-left" )
		{
			m_bLeft = false;
		}
		else if( inCommand == "+right" )
		{
			m_bRight = true;
		}
		else if( inCommand == "-right" )
		{
			m_bRight = false;
		}
		else if( inCommand == "+back" )
		{
			m_bBwd = true;
		}
		else if( inCommand == "-back" )
		{
			m_bBwd = false;
		}
		else
		{
			return false;
		}

		return true;
	}


	bool NoClipMovementComponent::HandleAxisBinding( const String& inCommand, float inValue )
	{
		auto owner = GetOwner();

		if( owner && owner->IsSpawned() )
		{
			if( inCommand == "view_x" )
			{
				auto currentRotation = owner->GetRotation();
				owner->SetRotation( Angle3D( currentRotation.Pitch, currentRotation.Yaw + inValue, 0.f ) );
			}
			else if( inCommand == "view_y" )
			{
				auto currentRotation = owner->GetRotation();
				owner->SetRotation( Angle3D( currentRotation.Pitch + inValue, currentRotation.Yaw, 0.f ) );
			}
			else return false;
		}
		else return false;

		return true;
	}


	void NoClipMovementComponent::Tick( double delta )
	{
		float movementX = 0.f;
		float movementZ = 0.f;

		if( m_bRight )
		{
			movementX += 1.f;
		}

		if( m_bLeft )
		{
			movementX -= 1.f;
		}

		if( m_bFwd )
		{
			movementZ += 1.f;
		}

		if( m_bBwd )
		{
			movementZ -= 1.f;
		}

		// We need to get a uint vector aimed in the direction were currently facing
		auto owner			= GetOwner();

		if( owner && owner->IsSpawned() && ( movementX != 0.f || movementZ != 0.f ) )
		{
			auto ownerEuler = owner->GetQuaternion().GetEulerAngles();
			auto ownerFwd		= ownerEuler.GetDirectionVector();
			auto ownerRight = Angle3D( 0.f, ownerEuler.Yaw + 90.f, 0.f ).GetDirectionVector();

			owner->Translate( ( ownerFwd * movementZ + ownerRight * movementX ) * 0.25f );
		}
	}

}

