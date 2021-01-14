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


	bool Character::ProcessKeyBinding( const String& inBind )
	{
		// First, check if dervied classes can handle this
		if( HandleKeyBinding( inBind ) ) { return true; }

		if( m_Movement && m_Movement->IsValid() )
		{
			return m_Movement->HandleKeyBinding( inBind );
		}

		return false;
	}


	bool Character::ProcessAxisBinding( const String& inBind, float inValue )
	{
		if( HandleAxisBinding( inBind, inValue ) ) { return true; }

		if( m_Movement && m_Movement->IsValid() )
		{
			return m_Movement->HandleAxisBinding( inBind, inValue );
		}

		return false;
	}


	bool Character::HandleKeyBinding( const String& inBind )
	{
		return false;
	}


	bool Character::HandleAxisBinding( const String& inAxis, float inValue )
	{
		return false;
	}

}

HYPERION_REGISTER_OBJECT_TYPE( Character, Entity );