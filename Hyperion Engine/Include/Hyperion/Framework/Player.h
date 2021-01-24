/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/Player.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/CharacterController.h"


namespace Hyperion
{

	class Player : public CharacterController
	{

	protected:

		uint32 m_PlayerIdentifier;

	public:

		/*
			Constructor/Destructor
		*/
		Player( uint32 inIdentifier );
		virtual ~Player();

		/*
			Getters
		*/
		inline uint32 GetPlayerIdentifier() const { return m_PlayerIdentifier; }
		inline bool IsLocalPlayer() const { return m_PlayerIdentifier == PLAYER_LOCAL; }


	protected:

		virtual void UpdateInput( InputManager& im, double delta ) override;


		friend class LocalPlayer;
	};

}