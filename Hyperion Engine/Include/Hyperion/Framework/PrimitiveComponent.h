/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/PrimitiveComponent.h
	© 2019, Zachary Berry
==================================================================================================*/


#pragma once
#include "Hyperion/Framework/Component.h"


namespace Hyperion
{

	class ProxyPrimitive;

	class PrimitiveComponent : public Component
	{

	protected:

		void RegisterPrimitive();
		void RemovePrimitive();

	public:

		virtual bool IsProxyStale() const;
		virtual bool IsProxyDirty() const;

		virtual std::shared_ptr< ProxyPrimitive > CreateProxy();
		virtual bool UpdateProxy( std::shared_ptr< ProxyPrimitive >& inProxy );




	};



}
