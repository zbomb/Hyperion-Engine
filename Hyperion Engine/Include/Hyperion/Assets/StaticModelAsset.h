/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Assets/StaticModelAsset.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/AssetManager.h"
#include "Hyperion/Assets/MaterialAsset.h"
#include "Hyperion/Library/Math/Geometry.h"



namespace Hyperion
{
	// Forward Declarations
	class StaticModelAsset;


	struct StaticModelAssetSubModel
	{

	public:

		uint8 Index;
		uint8 MaterialIndex;

		uint32 VertexOffset;
		uint32 VertexLength;

		uint32 IndexOffset;
		uint32 IndexLength;
	};


	struct StaticModelAssetLOD
	{

	public:

		uint8 Index;
		float MinScreenSize;
		std::map< uint8, StaticModelAssetSubModel > SubObjects;
		std::map< uint8, std::shared_ptr< MaterialAsset > > MaterialSlots;

		// TODO: Collision Stuff
	};


	class StaticModelAssetCache : public IAssetCache
	{

	private:

		static std::map< uint32, std::weak_ptr< StaticModelAsset > > m_Cache;
		static std::mutex m_CacheMutex;

	public:

		static std::weak_ptr< StaticModelAsset > Get( uint32 inIdentifier );
		static void Store( uint32 inIdentifier, const std::shared_ptr< StaticModelAsset >& inPtr );

	};


	class StaticModelAssetLoader : public IAssetLoader
	{

	public:

		static std::shared_ptr< StaticModelAsset > Load( uint32 inIdentifier, std::unique_ptr< IFile >& inFile );
		static bool IsValidFile( const FilePath& inPath );

	};




	class StaticModelAsset : public AssetBase
	{

	public:

		// Things needed for the asset manager to handle this type properly
		using _CacheType	= StaticModelAssetCache;
		using _LoaderType	= StaticModelAssetLoader;

		static inline AssetCacheMethod GetCacheMethod() { return AssetCacheMethod::Full; }

	private:

		AABB m_BoundingBox;
		BoundingSphere m_BoundingSphere;

		std::vector< StaticModelAssetLOD > m_LODs;
		std::map< uint8, std::shared_ptr< MaterialAsset > > m_MaterialSlots;

		StaticModelAsset( const FilePath& inPath, uint32 inIdentifier );

	public:

		StaticModelAsset( const StaticModelAsset& ) = delete;
		StaticModelAsset( StaticModelAsset&& ) = delete;
		StaticModelAsset& operator=( const StaticModelAsset& ) = delete;
		StaticModelAsset& operator=( StaticModelAsset&& ) = delete;

		// Bounds Method
		const AABB& GetAABB() const { return m_BoundingBox; }
		const BoundingSphere& GetBoundingSphere() const { return m_BoundingSphere; }

		// LOD Methods
		std::vector< StaticModelAssetLOD >::const_iterator LODBegin() const { return m_LODs.begin(); }
		std::vector< StaticModelAssetLOD >::const_iterator LODEnd() const { return m_LODs.end(); }
		std::vector< StaticModelAssetLOD >::const_iterator GetLOD( uint8 inIndex ) const;
		uint8 GetNumLODs() const { return (uint8)m_LODs.size(); }

		// Material Methods
		uint8 GetNumGlobalMaterialSlots() const { return (uint8)m_MaterialSlots.size(); }
		uint8 GetNumOverrideMaterialSlots( uint8 inLOD ) const;
		std::map< uint8, std::shared_ptr< MaterialAsset > >::const_iterator GetGlobalMaterialsBegin() const { return m_MaterialSlots.begin(); }
		std::map< uint8, std::shared_ptr< MaterialAsset > >::const_iterator GetGlobalMaterialsEnd() const { return m_MaterialSlots.end(); }
		std::shared_ptr< MaterialAsset > GetGlobalMaterial( uint8 inSlot ) const;
		std::map< uint8, std::shared_ptr< MaterialAsset > >::const_iterator GetOverrideMaterialsBegin( uint8 inLOD ) const;
		std::map< uint8, std::shared_ptr< MaterialAsset > >::const_iterator GetOverrideMaterialsEnd( uint8 inLOD ) const;
		std::shared_ptr< MaterialAsset > GetOverrideMaterial( uint8 inLOD, uint8 inSlot ) const;

		// SubObject Methods
		uint8 GetNumSubModels( uint8 inLOD ) const;
		std::map< uint8, StaticModelAssetSubModel >::const_iterator GetSubModelBegin( uint8 inLOD ) const;
		std::map< uint8, StaticModelAssetSubModel >::const_iterator GetSubModelEnd( uint8 inLOD ) const;
		std::map< uint8, StaticModelAssetSubModel >::const_iterator GetSubModel( uint8 inLOD, uint8 inIndex ) const;

		friend class StaticModelAssetLoader;
		friend class HSMReader;
	};


}