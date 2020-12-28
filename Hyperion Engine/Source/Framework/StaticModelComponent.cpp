/*==================================================================================================
	Hyperion Engine
	Source/Framework/StaticModelComponent.cpp
	© 2019, Zachary Berry
==================================================================================================*/


#include "Hyperion/Framework/StaticModelComponent.h"


namespace Hyperion
{
	
	std::shared_ptr< ProxyPrimitive > StaticModelComponent::CreateProxy()
	{
		return nullptr;
	}

	bool StaticModelComponent::UpdateProxy()
	{
		return true;
	}
	
}


/*
*	Register Type
*/
HYPERION_REGISTER_OBJECT_TYPE( StaticModelComponent, PrimitiveComponent );