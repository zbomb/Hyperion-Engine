/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/CharacterController.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/Entity.h"
#include "Hyperion/Framework/Character.h"


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