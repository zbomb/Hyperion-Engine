/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Assets/DynamicModelAsset.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Asset.h"
#include "Hyperion/Core/AssetLoader.h"


namespace Hyperion
{

	class DynamicModelAsset : public Asset
	{

	public:

		DynamicModelAsset()
		{}

		~DynamicModelAsset()
		{}

		String GetAssetType() const final { return "dynamic_model"; }
	};


	template<>
	inline std::shared_ptr< Asset > AssetLoader::Load< DynamicModelAsset >( const AssetPath& inFileName, const std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
	{
		return std::make_shared< DynamicModelAsset >();
	}

	template<>
	inline std::shared_ptr< Asset > AssetLoader::Stream< DynamicModelAsset >( const AssetPath& inFileName, AssetStream& inStream )
	{
		return std::make_shared< DynamicModelAsset >();
	}


}