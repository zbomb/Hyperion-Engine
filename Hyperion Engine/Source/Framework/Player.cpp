/*==================================================================================================
	Hyperion Engine
	Source/Framework/Player.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/Player.h"
#include "Hyperion/Framework/CameraComponent.h"


namespace Hyperion
{

	Player::Player( uint32 inIdentifier )
		: m_PlayerIdentifier( inIdentifier )
	{

	}


	Player::~Player()
	{

	}


	bool Player::ProcessKeyBinding( const String& inBind )
	{
		// First, check if the derived class can handle this key binding
		if( HandleKeyBinding( inBind ) ) { return true; }

		// Pass this down to the character
		if( m_PossessedCharacter && m_PossessedCharacter->IsValid() )
		{
			return m_PossessedCharacter->ProcessKeyBinding( inBind );
		}

		return false;
	}


	bool Player::ProcessAxisBinding( const String& inBind, float inValue )
	{
		// First, check if the derived class can handle this axis binding
		if( HandleAxisBinding( inBind, inValue ) ) { return true; }

		if( m_PossessedCharacter && m_PossessedCharacter->IsValid() )
		{
			return m_PossessedCharacter->ProcessAxisBinding( inBind, inValue );
		}

		return false;
	}


	bool Player::HandleKeyBinding( const String& inKey )
	{
		return false;
	}


	bool Player::HandleAxisBinding( const String& inKey, float inValue )
	{
		return false;
	}


}


/*
*	Register Type
*/
HYPERION_REGISTER_OBJECT_TYPE( Player, Entity, Hyperion::PLAYER_INVALID );