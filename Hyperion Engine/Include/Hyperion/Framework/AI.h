/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/AI.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/CharacterController.h"


namespace Hyperion
{

	class AI : public CharacterController
	{

	};

}

HYPERION_REGISTER_OBJECT_TYPE( AI, CharacterController );