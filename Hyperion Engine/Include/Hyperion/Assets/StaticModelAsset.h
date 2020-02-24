/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Assets/StaticModelAsset.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Assets/ModelAsset.h"
#include "Hyperion/Library/Geometry.h"



namespace Hyperion
{

	// Forward Declarations
	class MaterialAsset;


	class StaticModelAsset : public Asset
	{

		AssetRef< MaterialAsset > m_DefaultMaterial;
		BoundingSphere m_BoundingSphere;

	public:

		StaticModelAsset()
		{}

		~StaticModelAsset()
		{}

		String GetAssetType() const final { return "static_model"; }

		inline const AssetRef< MaterialAsset >& GetDefaultMaterial() const { return m_DefaultMaterial; }
		inline const BoundingSphere& GetBoundingSphere() const { return m_BoundingSphere; }

	};


	template<>
	inline std::shared_ptr< Asset > AssetLoader::Load< StaticModelAsset >( const AssetPath& inFileName, const std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
	{
		return std::make_shared< StaticModelAsset >();
	}

	template<>
	inline std::shared_ptr< Asset > AssetLoader::Stream< StaticModelAsset >( const AssetPath& inFileName, AssetStream& inStream )
	{
		return std::make_shared< StaticModelAsset >();
	}


}