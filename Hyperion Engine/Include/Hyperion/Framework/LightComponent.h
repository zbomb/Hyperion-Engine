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

		std::weak_ptr< ProxyLight > m_Proxy;

		bool PerformProxyCreation() override;
		bool PerformProxyUpdate() override;

		void AddToRenderer() override;
		void RemoveFromRenderer() override;

		virtual std::shared_ptr< ProxyLight > CreateProxy() = 0;
		virtual bool UpdateProxy( const std::shared_ptr< ProxyLight >& ) = 0;
	};

}
