/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Assets/ModelAsset.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Asset.h"
#include "Hyperion/Core/AssetLoader.h"


namespace Hyperion
{

	class ModelAsset : public Asset
	{

	public:

	};

	template<>
	inline std::shared_ptr< Asset > AssetLoader::Load< ModelAsset >( const AssetPath& inFileName, const std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
	{
		throw std::exception( "Cannot create the model asset baseclass!" );
	}

	template<>
	inline std::shared_ptr< Asset > AssetLoader::Stream< ModelAsset >( const AssetPath& inFileName, AssetStream& inStream )
	{
		throw std::exception( "Cannot create the model asset baseclass!" );
	}

}