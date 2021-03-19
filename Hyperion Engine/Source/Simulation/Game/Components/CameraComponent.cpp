/*==================================================================================================
	Hyperion Engine
	Source/Simulation/Game/Components/CameraComponent.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Simulation/Game/Components/CameraComponent.h"
#include "Hyperion/Simulation/Game/Entities/Player.h"


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
		m_ActivePlayers.clear();
	}

}

/*
*	Register Type
*/
HYPERION_REGISTER_OBJECT_TYPE( CameraComponent, Component );