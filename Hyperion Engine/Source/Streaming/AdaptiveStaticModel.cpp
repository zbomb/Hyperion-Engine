/*==================================================================================================
	Hyperion Engine
	Source/Streaming/AdaptiveStaticModel.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Streaming/AdaptiveStaticModel.h"



namespace Hyperion
{

	AdaptiveStaticModel::AdaptiveStaticModel( const AssetRef< StaticModelAsset >& inAsset )
	{
		if( !inAsset.IsValid() )
		{
			Console::WriteLine( "[ERROR] AdaptiveAssetManager: Attempt to create static model instance, but the asset was invalid!" );
		}
		else
		{
			m_Asset = AssetCast< ModelAsset >( inAsset );
			
			// TODO TODO TODO
			// We need to figure out if we want to even have a common model baseclass, or just seperate assets for each
			// right now, this is not totally optimal
		}
	}



	AdaptiveStaticModel::~AdaptiveStaticModel()
	{

	}

}
