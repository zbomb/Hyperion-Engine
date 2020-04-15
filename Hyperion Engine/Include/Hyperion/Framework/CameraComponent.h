/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/CameraComponent.h
	� 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/Component.h"


namespace Hyperion
{
	// Forward Declarations
	class Player;


	class CameraComponent : public Component
	{

	protected:

		std::vector< HypPtr< Player > > m_ActivePlayers;

	public:

		/*
			Member Functions
		*/
		void OnSelected( const HypPtr< Player >& inPlayer );
		void OnDeSelected( const HypPtr< Player >& inPlayer );

		/*
			Getters
		*/
		inline const std::vector< HypPtr< Player > >& GetPlayerList() const { return m_ActivePlayers; }

		/*
			Component Hooks
		*/
		virtual void OnDespawn( const HypPtr< World >& inWorld );

	};

}