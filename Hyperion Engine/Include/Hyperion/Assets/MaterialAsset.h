/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Assets/MaterialAsset.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Asset.h"
#include "Hyperion/Core/AssetLoader.h"


namespace Hyperion
{

	class MaterialAsset : public Asset
	{

	public:

		MaterialAsset()
		{}

		~MaterialAsset()
		{}

		String GetAssetType() const final
		{
			return "material";
		}

	};


	template<>
	inline std::shared_ptr< Asset > AssetLoader::Load< MaterialAsset >( const AssetPath& inFileName, const std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
	{
		return std::make_shared< MaterialAsset >();
	}

	template<>
	inline std::shared_ptr< Asset > AssetLoader::Stream< MaterialAsset >( const AssetPath& inFileName, AssetStream& inStream )
	{
		return std::make_shared< MaterialAsset >();
	}


}