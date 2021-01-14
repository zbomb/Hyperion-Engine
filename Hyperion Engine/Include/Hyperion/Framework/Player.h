/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/Player.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/CharacterController.h"


namespace Hyperion
{

	class Player : public CharacterController
	{

	private:

		bool ProcessKeyBinding( const String& inBind );
		bool ProcessAxisBinding( const String& inBind, float inValue );

	protected:

		uint32 m_PlayerIdentifier;

	public:

		/*
			Constructor/Destructor
		*/
		Player( uint32 inIdentifier );
		virtual ~Player();

		virtual bool HandleKeyBinding( const String& inBinding );
		virtual bool HandleAxisBinding( const String& inBinding, float inValue );

		/*
			Getters
		*/
		inline uint32 GetPlayerIdentifier() const { return m_PlayerIdentifier; }


		friend class LocalPlayer;
	};

}