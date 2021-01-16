/*==================================================================================================
	Hyperion Engine
	Source/Framework/LocalPlayer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/LocalPlayer.h"
#include "Hyperion/Framework/Player.h"
#include "Hyperion/Framework/CameraComponent.h"
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


	bool LocalPlayer::ProcessKeyBinding( const String& inEvent )
	{
		if( m_PlayerEntity && m_PlayerEntity->IsValid() )
		{
			return m_PlayerEntity->ProcessKeyBinding( inEvent );
		}

		return false;
	}

	bool LocalPlayer::ProcessAxisBinding( const String& inEvent, float inValue )
	{
		if( m_PlayerEntity && m_PlayerEntity->IsValid() )
		{
			return m_PlayerEntity->ProcessAxisBinding( inEvent, inValue );
		}

		return false;
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