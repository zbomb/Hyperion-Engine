/*==================================================================================================
	Hyperion Engine
	Source/Simulation/Game/LocalPlayer.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Simulation/Game/LocalPlayer.h"
#include "Hyperion/Simulation/Game/Entities/Player.h"
#include "Hyperion/Simulation/Game/Components/CameraComponent.h"
#include "Hyperion/Library/Geometry.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Core/InputManager.h"

#include <functional>


namespace Hyperion
{

	LocalPlayer::LocalPlayer()
	{

	}


	LocalPlayer::~LocalPlayer()
	{

	}


	void LocalPlayer::SetPlayer( const HypPtr< Player >& inPlayer )
	{
		m_PlayerEntity = inPlayer;
	}


	void LocalPlayer::GetActiveCameraTransform( Transform& outTransform )
	{
		auto character = m_PlayerEntity ? m_PlayerEntity->GetCharacter() : nullptr;
		auto camera = character ? character->GetCamera() : nullptr;

		if( camera && camera->IsValid() )
		{
			outTransform = camera->GetWorldTransform();
		}
		else
		{
			// TODO: Get level default camera position? Like a spawn position or something
			outTransform.Position.Clear();
			outTransform.Rotation.Clear();
		}

		outTransform.Scale.Clear();
	}

}


/*
*	Register Type
*/
HYPERION_REGISTER_OBJECT_TYPE( LocalPlayer, Object );