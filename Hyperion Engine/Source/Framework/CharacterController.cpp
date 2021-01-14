/*==================================================================================================
	Hyperion Engine
	Source/Framework/CharacterController.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/CharacterController.h"



namespace Hyperion
{

	bool CharacterController::IsPossessingCharacer() const
	{
		return m_PossessedCharacter && m_PossessedCharacter->IsSpawned();
	}


	bool CharacterController::PossessCharacter( const HypPtr< Character >& inTarget )
	{
		// First, lets check if we can possess this character
		if( !inTarget || !inTarget->IsValid() )
		{
			Console::WriteLine( "[Warning] CharacterController: Failed to possess character, because the target was invalid" );
			return false;
		}
		
		auto currentController = inTarget->GetController();
		if( currentController.IsValid() )
		{
			if( currentController->GetIdentifier() == GetIdentifier() )
			{
				// This character is already controlled by us
				return true;
			}
			else
			{
				// This character is controlled by another controller.. for now lets fail
				Console::WriteLine( "[Warning] CharacterController: Failed to possess character, because the target is already possessed by another controller" );
				return false;
			}
		}

		// If were already possessing a character, we need to unpossess it first
		if( m_PossessedCharacter && m_PossessedCharacter->IsValid() )
		{
			m_PossessedCharacter->PerformPossession( nullptr );
			OnCharacterUnPossessed( m_PossessedCharacter );
		}

		m_PossessedCharacter = inTarget;
		inTarget->PerformPossession( AquirePointer< CharacterController >() );
		OnCharacterPossessed( inTarget );
	}


	HypPtr< Character > CharacterController::GetCharacter() const
	{
		return( ( m_PossessedCharacter && m_PossessedCharacter->IsValid() ) ? m_PossessedCharacter : nullptr );
	}


	void CharacterController::OnDestroy()
	{
		m_PossessedCharacter.Clear();
	}

}

HYPERION_REGISTER_OBJECT_TYPE( CharacterController, Entity );