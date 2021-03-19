/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Simulation/Game/Components/DynamicModelComponent.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Simulation/Game/Components/PrimitiveComponent.h"


namespace Hyperion
{

	class DynamicModelComponent : public PrimitiveComponent
	{

	protected:

		bool UpdateProxy( const std::shared_ptr< ProxyPrimitive >& inPtr ) override
		{
			return true; // TODO
		}

		std::shared_ptr< ProxyPrimitive > CreateProxy() override
		{
			return nullptr; // TODO
		}

	public:

	};

}