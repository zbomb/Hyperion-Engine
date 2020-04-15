/*==================================================================================================
	Hyperion Engine
	Source/Framework/LocalPlayer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/LocalPlayer.h"
#include "Hyperion/Framework/Player.h"
#include "Hyperion/Framework/CameraComponent.h"
#include "Hyperion/Library/Math.h"



namespace Hyperion
{

	LocalPlayer::LocalPlayer()
	{
		// Default FOV for 'LastViewState'
		m_LastViewState.FOV = Math::PIf / 4.f; // TODO: We need to come up with a FOV system
	}


	LocalPlayer::~LocalPlayer()
	{

	}


	uint32 LocalPlayer::GetPlayerIdentifier() const
	{
		if( m_PlayerEntity.IsValid() && m_PlayerEntity->IsSpawned() )
		{
			return m_PlayerEntity->GetPlayerIdentifier();
		}

		// We get assigned a player identifier by the server during the join sequence
		// So, if we havent been assigned a player entity, then we dont actually have an identifier
		return PLAYER_INVALID;
	}


	HypPtr< CameraComponent > LocalPlayer::GetActiveCamera() const
	{
		if( m_PlayerEntity.IsValid() && m_PlayerEntity->IsSpawned() )
		{
			return m_PlayerEntity->GetActiveCamera();
		}

		return nullptr;
	}


	bool LocalPlayer::GetViewState( ViewState& outState )
	{
		// We return true whenever the view state has changed since the last call
		auto camera = GetActiveCamera();
		if( camera && camera->IsActive() )
		{
			outState.Position	= camera->GetPosition();
			outState.Rotation	= camera->GetRotation();
			outState.FOV		= Math::PIf / 4.f;		// TODO: Move this into a console command?
		}
		else
		{
			// If there is no active camera, we want to keep using the last known camera position
			outState = m_LastViewState;
			return false;
		}

		// Check if the current state differs from the cached state
		if( outState != m_LastViewState )
		{
			m_LastViewState = outState;
			return true;
		}
		else
		{
			return false;
		}
	}


	void LocalPlayer::SetPlayerEntity( const HypPtr< Player >& inPlayer )
	{
		// TODO: Possess hooks? and unposses hooks?
		m_PlayerEntity = inPlayer;
	}

}