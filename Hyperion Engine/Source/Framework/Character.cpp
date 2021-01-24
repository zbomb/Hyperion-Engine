/*==================================================================================================
	Hyperion Engine
	Source/Framework/Character.cpp
	© 2021, Zachary Berry
==================================================================================================*/


#include "Hyperion/Framework/Character.h"
#include "Hyperion/Framework/NoClipMovementComponent.h"
#include "Hyperion/Framework/CharacterController.h"


namespace Hyperion
{

	void Character::OnCreate()
	{
		m_Camera	= CreateObject< CameraComponent >();
		AddComponent( m_Camera, "camera" );
		//m_Movement	= CreateObject< MovementComponent >();

		// DEBUG
		m_Movement = CreateObject< NoClipMovementComponent >();
		AddComponent( m_Movement, "movement" );
	}


	void Character::OnDestroy()
	{
		m_Controller.Clear();

		DestroyObject( m_Camera );
		DestroyObject( m_Movement );
	}


	void Character::PerformPossession( const HypPtr< CharacterController >& inController )
	{
		if( inController )
		{
			if( !inController->IsValid() )
			{
				Console::WriteLine( "[Warning] Character: Failed to get possessed, controller was invalid" );
				return;
			}

			m_Controller = inController;
			OnPossessed( inController );
		}
		else
		{
			// Since the parameter is null, were unpossessing
			auto oldController = m_Controller;
			m_Controller.Clear();

			OnUnPossessed( oldController );
		}
	}


	bool Character::IsPossessed() const
	{
		return m_Controller && m_Controller->IsValid();
	}


	HypPtr< CharacterController > Character::GetController() const
	{
		return( ( m_Controller && m_Controller->IsValid() ) ? m_Controller : nullptr );
	}


	HypPtr< CameraComponent > Character::GetCamera() const
	{
		return m_Camera;
	}


	HypPtr< MovementComponent > Character::GetMovement() const
	{
		return m_Movement;
	}
	
	
	void Character::SetMovementInput( const Vector3D& inVec )
	{
		if( m_Movement && m_Movement->IsValid() )
		{
			m_Movement->Move( inVec );
		}
	}


	void Character::SetLookInput( const Vector2D& inVec )
	{
		if( m_Movement && m_Movement->IsValid() )
		{
			m_Movement->Look( inVec );
		}
	}


	void Character::SetEyeDirection( float inPitch, float inYaw )
	{
		// Pitch gets applied to the camera, yaw to the entire entity
		if( m_Camera && m_Camera->IsValid() )
		{
			m_Camera->SetRotation( Angle3D( inPitch, 0.f, 0.f ) );
			
		}

		// Yaw gets applied to entire entity
		SetRotation( Angle3D( 0.f, inYaw, 0.f ) );
	}


	Angle3D Character::GetEyeDirection() const
	{
		// Yaw from the eneity, pitch from the camera
		Angle3D dir{};
		auto thisRot = GetRotation();

		if( m_Camera && m_Camera->IsValid() )
		{
			dir.Pitch = m_Camera->GetRotation().Pitch;
		}
		else
		{
			dir.Pitch = thisRot.Pitch;
		}

		dir.Yaw		= thisRot.Yaw;
		dir.Roll	= 0.f;

		dir.ClampContents();
		return dir;
	}

}

HYPERION_REGISTER_OBJECT_TYPE( Character, Entity );