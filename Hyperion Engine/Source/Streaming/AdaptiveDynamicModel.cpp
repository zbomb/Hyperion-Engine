/*==================================================================================================
	Hyperion Engine
	Source/Streaming/AdaptiveDynamicModel.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Streaming/AdaptiveDynamicModel.h"



namespace Hyperion
{

	AdaptiveDynamicModel::AdaptiveDynamicModel( const AssetRef< DynamicModelAsset >& inAsset )
	{
		if( !inAsset.IsValid() )
		{
			Console::WriteLine( "[ERROR] AdaptiveAssetManager: Attempt to create a dynamic model isntance with an invalid asset!" );
		}
		else
		{
			m_Asset = AssetCast< ModelAsset >( inAsset );
		}
	}


	AdaptiveDynamicModel::~AdaptiveDynamicModel()
	{

	}

}
