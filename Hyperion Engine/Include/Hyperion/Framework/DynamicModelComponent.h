/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/DynamicModelComponent.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Framework/PrimitiveComponent.h"



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