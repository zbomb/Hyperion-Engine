/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Simulation/Game/Entities/CharacterController.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Simulation/Game/Entities/Character.h"


namespace Hyperion
{

	class CharacterController : public Entity
	{

	protected:

		HypPtr< Character > m_PossessedCharacter;

	public:

		bool IsPossessingCharacer() const;
		bool PossessCharacter( const HypPtr< Character >& inTarget );
		HypPtr< Character > GetCharacter() const;

	protected:

		virtual void OnCharacterPossessed( const HypPtr< Character >& inCharacter ) {}
		virtual void OnCharacterUnPossessed( const HypPtr< Character >& inCharacter ) {}

		virtual void OnDestroy() override;

	};

}