/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Simulation/Game/Entities/LocalPlayer.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Object.h"
#include "Hyperion/Simulation/Game/ViewState.h"


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class Player;
	struct Transform;


	class LocalPlayer : public Object
	{

	private:

		HypPtr< Player > m_PlayerEntity;

		void SetPlayer( const HypPtr< Player >& inPlayer );

	public:

		LocalPlayer();
		~LocalPlayer();

		void GetActiveCameraTransform( Transform& outTransform );

		inline HypPtr< Player > GetPlayer() const { return m_PlayerEntity; }

		// TODO: Friend in the class that will assign the player entity to this local player
		// For now, were just going to perform this in the game intance
		friend class GameInstance;
	};

}