/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/Player.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/Entity.h"


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class CameraComponent;


	class Player : public Entity
	{

	protected:

		uint32 m_PlayerIdentifier;
		HypPtr< CameraComponent > m_ActiveCamera;

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
		inline HypPtr< CameraComponent > GetActiveCamera() const { return m_ActiveCamera; }

		/*
			Member Functions
		*/
		void SetActiveCamera( const HypPtr< CameraComponent >& inCamera );

		/*
			New Hooks
		*/
		virtual void OnCameraSelected( const HypPtr< CameraComponent >& inCamera );
		virtual void OnCameraDeselected( const HypPtr< CameraComponent >& inCamera );

		/*
			Entity Hook Overrides
		*/
		virtual void OnDespawn( const HypPtr< World >& inWorld );

	};

}