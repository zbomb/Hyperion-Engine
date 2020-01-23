/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/PrimitiveComponent.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/RenderComponent.h"


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class ProxyPrimitive;


	class PrimitiveComponent : public RenderComponent
	{

	protected:

		bool PerformProxyCreation() override;
		
		void AddToRenderer() override;
		void RemoveFromRenderer() override;

		virtual std::shared_ptr< ProxyPrimitive > CreateProxy() = 0;
	};

}