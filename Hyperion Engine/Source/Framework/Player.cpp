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


	void Player::SetActiveCamera( const HypPtr< CameraComponent >& inCamera )
	{
		// First, we want to ensure the camera were targetting is valid
		if( inCamera && inCamera->IsActive() )
		{
			// Deselect the active camera (if there is one)
			auto thisPtr = AquirePointer< Player >();

			if( m_ActiveCamera && m_ActiveCamera->IsActive() )
			{
				m_ActiveCamera->OnDeSelected( thisPtr );
				OnCameraDeselected( m_ActiveCamera );
			}

			m_ActiveCamera = inCamera;

			inCamera->OnSelected( thisPtr );
			OnCameraSelected( inCamera );
		}
		else
		{
			// If the camera isnt valid (or the input is null) then we will have no valid selected camera
			if( m_ActiveCamera && m_ActiveCamera->IsActive() )
			{
				m_ActiveCamera->OnDeSelected( AquirePointer< Player >() );
				OnCameraDeselected( m_ActiveCamera );

				m_ActiveCamera.Clear();
			}
		}
	}


	void Player::OnCameraSelected( const HypPtr< CameraComponent >& inCamera )
	{

	}


	void Player::OnCameraDeselected( const HypPtr< CameraComponent >& inCamera )
	{

	}


	void Player::OnDespawn( const HypPtr< World >& inWorld )
	{
		if( m_ActiveCamera && m_ActiveCamera->IsActive() )
		{
			m_ActiveCamera->OnDeSelected( AquirePointer< Player >() );
			OnCameraDeselected( m_ActiveCamera );

			m_ActiveCamera.Clear();
		}
	}


}