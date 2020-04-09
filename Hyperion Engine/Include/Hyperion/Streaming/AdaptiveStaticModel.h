/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/AdaptiveStaticModel.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Streaming/AdaptiveModel.h"
#include "Hyperion/Assets/StaticModelAsset.h"


namespace Hyperion
{

	class AdaptiveStaticModel : public AdaptiveModel
	{

	private:


	public:

		AdaptiveStaticModel( const std::shared_ptr< StaticModelAsset >& inAsset );
		~AdaptiveStaticModel();

		AdaptiveStaticModel() = delete;

	};

}