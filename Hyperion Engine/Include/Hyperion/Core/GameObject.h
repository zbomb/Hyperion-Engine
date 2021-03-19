/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/GameObject.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"


namespace Hyperion
{

	class GameObject : public Object
	{
		/*
		*	Game Object Notes:
		*	- This is a type of object, that has more functionality and is designed for 'Objects' that are going to be used within and is syncronized with the Game Loop
		*	- Game Objects also have unique identifiers, and can be looked up by their identifier from a central list 
		*/

	private:

		uint32 m_Identifier;

	protected:



	public:



	};

}