/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Material/MaterialAsset.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Asset.h"
#include "Hyperion/Core/AssetLoader.h"


namespace Hyperion
{

	// Forward Declarations
	class MaterialInstance;


	class MaterialAsset : public Asset
	{

	private:

		std::shared_ptr< MaterialInstance > m_StaticInstance;



	public:

		std::shared_ptr< MaterialInstance > GetStaticInstance();
		std::shared_ptr< MaterialInstance > GetDynamicInstance();

	};


	template<>
	std::shared_ptr< Asset > AssetLoader::Load< MaterialAsset >( const AssetPath& inPath, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End );

	template<>
	std::shared_ptr< Asset > AssetLoader::Stream< MaterialAsset >( const AssetPath& inPath, AssetStream& inStream );

}
