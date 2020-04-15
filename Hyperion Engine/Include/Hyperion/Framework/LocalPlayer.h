/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/LocalPlayer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Object.h"
#include "Hyperion/Framework/ViewState.h"


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class Player;
	class CameraComponent;


	class LocalPlayer : public Object
	{

	private:

		HypPtr< Player > m_PlayerEntity;
		ViewState m_LastViewState;

		void SetPlayerEntity( const HypPtr< Player >& inPlayer );

	public:

		LocalPlayer();
		~LocalPlayer();

		uint32 GetPlayerIdentifier() const;
		HypPtr< CameraComponent > GetActiveCamera() const;
		bool GetViewState( ViewState& outState );

		inline HypPtr< Player > GetPlayerEntity() const { return m_PlayerEntity; }

		// TODO: Friend in the class that will assign the player entity to this local player
		// For now, were just going to perform this in the game intance
		friend class GameInstance;
	};

}