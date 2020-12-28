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

		virtual bool PerformProxyCreation()
		{
			return true; // TODO
		}

		virtual bool UpdateProxy()
		{
			return true; // TODO
		}

		virtual std::shared_ptr< ProxyPrimitive > CreateProxy()
		{
			return nullptr; // TODO
		}

	public:

	};

}