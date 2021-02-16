/*==================================================================================================
	Hyperion Engine
	Source/Framework/Player.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/Player.h"
#include "Hyperion/Framework/CameraComponent.h"
#include "Hyperion/Core/InputManager.h"


namespace Hyperion
{

	Player::Player( uint32 inIdentifier )
		: m_PlayerIdentifier( inIdentifier )
	{
		// Request user inputs if were the 'local player' controller
		if( m_PlayerIdentifier == PLAYER_LOCAL )
		{
			bRequiresInput = true;
		}
	}


	Player::~Player()
	{
		bRequiresInput = false;
	}


	void Player::UpdateInput( InputManager& im, double delta )
	{
		// We need to scale the amount of movement by the frequency of the game thread
		// This way, movement inputs dont affect the player more when the FPS is higher
		float movementScale = delta / 0.01667f;

		if( m_PossessedCharacter && m_PossessedCharacter->IsSpawned() )
		{
			// First, lets poll for inputs that are relevant
			bool bFwd			= im.PollState( INPUT_STATE_MOVE_FORWARD );
			bool bBwd			= im.PollState( INPUT_STATE_MOVE_BACKWARD );
			bool bRight			= im.PollState( INPUT_STATE_MOVE_RIGHT );
			bool bLeft			= im.PollState( INPUT_STATE_MOVE_LEFT );
			float fPitch		= im.PollScalar( INPUT_AXIS_LOOK_PITCH );
			float fYaw			= im.PollScalar( INPUT_AXIS_LOOK_YAW );
			bool bLookUp		= im.PollState( INPUT_STATE_LOOK_UP );
			bool bLookDown		= im.PollState( INPUT_STATE_LOOK_DOWN );
			bool bLookRight		= im.PollState( INPUT_STATE_LOOK_RIGHT );
			bool bLookLeft		= im.PollState( INPUT_STATE_LOOK_LEFT );
			bool bSprint		= im.PollState( INPUT_STATE_SPRINT );
			bool bDown			= im.PollState( INPUT_STATE_MOVE_DOWN );
			bool bUp			= im.PollState( INPUT_STATE_MOVE_UP );

			// Now, lets calculate our movement vector
			Vector3D movementVec(
				( bFwd ? 1.f : 0.f ) - ( bBwd ? 1.f : 0.f ),
				( bRight ? 1.f : 0.f ) - ( bLeft ? 1.f : 0.f ),
				( bUp ? 1.f : 0.f ) - ( bDown ? 1.f : 0.f )
			);

			// And our look vector
			Vector2D lookVec(
				fPitch + ( bLookDown ? 1.f : 0.f ) - ( bLookUp ? 1.f : 0.f ),
				fYaw + ( bLookRight ? 1.f : 0.f ) - ( bLookLeft ? 1.f : 0.f )
			);

			movementVec		= movementVec * movementScale;
			lookVec			= lookVec * movementScale;

			// Determine if we should apply a look input, or a movement input
			if( movementVec.X != 0.f || movementVec.Y != 0.f || movementVec.Z != 0.f )
			{
				m_PossessedCharacter->SetMovementInput( bSprint ? movementVec * 2.f : movementVec );
			}

			if( lookVec.X != 0.f || lookVec.Y != 0.f )
			{
				m_PossessedCharacter->SetLookInput( lookVec );
			}
		}
	}


}


/*
*	Register Type
*/
HYPERION_REGISTER_OBJECT_TYPE( Player, Entity, Hyperion::PLAYER_INVALID );