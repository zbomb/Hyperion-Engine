/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/LightComponent.h
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
	class ProxyLight;


	class LightComponent : public RenderComponent
	{

	protected:

		bool PerformProxyCreation() override;

		void AddToRenderer() override;
		void RemoveFromRenderer() override;

		virtual std::shared_ptr< ProxyLight > CreateProxy() = 0;

	};

}
