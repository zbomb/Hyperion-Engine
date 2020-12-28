/*==================================================================================================
	Hyperion Engine
	Source/Framework/CameraComponent.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/CameraComponent.h"
#include "Hyperion/Framework/Player.h"


namespace Hyperion
{

	void CameraComponent::OnSelected( const HypPtr< Player >& inPlayer )
	{
		// Ensure the player is valid, and this player is not already using this camera
		if( !inPlayer || !inPlayer->IsSpawned() )
		{
			Console::WriteLine( "[WARNING] CameraComponent: Attempt to select camera with an invalid player entity!" );
			return;
		}

		for( auto& p : m_ActivePlayers )
		{
			if( p == inPlayer )
			{
				Console::WriteLine( "[WARNING] CameraComponent: Player attempted to select a camera, that they already are using" );
				return;
			}
		}

		m_ActivePlayers.push_back( inPlayer );
	}


	void CameraComponent::OnDeSelected( const HypPtr< Player >& inPlayer )
	{
		if( !inPlayer )
		{
			Console::WriteLine( "[WARNING] CameraComponent: Attempt to deselect camera with null player entity!" );
			return;
		}

		bool bFound = false;
		for( auto It = m_ActivePlayers.begin(); It != m_ActivePlayers.end(); )
		{
			if( *It == inPlayer )
			{
				It = m_ActivePlayers.erase( It );
				bFound = true;
			}
			else
			{
				It++;
			}
		}

		if( !bFound )
		{
			Console::WriteLine( "[WARNING] CameraComponent: Player attempted to deselect this camera, but this player wasnt in the active player list!" );
		}
	}


	void CameraComponent::OnDespawn( const HypPtr< World >& inWorld )
	{
		// Force all active players to deselect this camera
		auto thisPtr = AquirePointer< CameraComponent >();

		for( auto& p : m_ActivePlayers )
		{
			if( p && p->IsSpawned() )
			{
				p->SetActiveCamera( nullptr );
			}
		}

		m_ActivePlayers.clear();
	}

}

/*
*	Register Type
*/
HYPERION_REGISTER_OBJECT_TYPE( CameraComponent, Component );