/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/AdaptiveBase.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Streaming/DataTypes.h"



namespace Hyperion
{

	class AdaptiveBase
	{

	public:

		std::map< uint32, std::weak_ptr< AdaptiveAssetManagerObjectInfo > > m_Refs;
		uint32 m_Identifier;

	};

}