/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/AdaptiveDynamicModel.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Streaming/AdaptiveModel.h"
#include "Hyperion/Assets/DynamicModelAsset.h"


namespace Hyperion
{

	class AdaptiveDynamicModel : public AdaptiveModel
	{

	private:


	public:

		AdaptiveDynamicModel( const AssetRef< DynamicModelAsset >& inAsset );
		~AdaptiveDynamicModel();

		AdaptiveDynamicModel() = delete;


	};

}