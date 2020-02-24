/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/AdaptiveModel.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Streaming/AdaptiveBase.h"


namespace Hyperion
{

	class AdaptiveModel : public AdaptiveBase
	{

	public:

		AssetRef< ModelAsset > m_Asset;

	};

}