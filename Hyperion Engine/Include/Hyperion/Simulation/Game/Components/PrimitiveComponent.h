/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Simulation/Game/Components/PrimitiveComponent.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Simulation/Game/Components/RenderComponent.h"


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class ProxyPrimitive;


	class PrimitiveComponent : public RenderComponent
	{

	protected:

		std::weak_ptr< ProxyPrimitive > m_Proxy;

		bool PerformProxyCreation() override;
		bool PerformProxyUpdate() override;
		
		void AddToRenderer() override;
		void RemoveFromRenderer() override;

		virtual std::shared_ptr< ProxyPrimitive > CreateProxy() = 0;
		virtual bool UpdateProxy( const std::shared_ptr< ProxyPrimitive >& ) = 0;
	};

}