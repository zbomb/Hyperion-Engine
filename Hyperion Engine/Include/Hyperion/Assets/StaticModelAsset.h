/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Assets/StaticModelAsset.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/AssetManager.h"
#include "Hyperion/Assets/MaterialAsset.h"
#include "Hyperion/Library/Geometry.h"


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

		uint32 VertexCount;
		uint32 IndexCount;
	};


	struct StaticModelAssetLOD
	{

	public:

		uint8 Index;
		float MinScreenSize;
		std::vector< StaticModelAssetSubModel > SubObjects;

		// TODO: Collision Stuff
	};




	class StaticModelAsset : public AssetBase
	{

	private:

		AABB m_BoundingBox;
		BoundingSphere m_BoundingSphere;

		std::vector< StaticModelAssetLOD > m_LODs;

		FilePath m_DiskPath;

		StaticModelAsset( const String& inPath, const FilePath& inDiskPath, uint32 inIdentifier, uint64 inOffset, uint64 inLength );

	public:

		StaticModelAsset( const StaticModelAsset& ) = delete;
		StaticModelAsset( StaticModelAsset&& ) = delete;
		StaticModelAsset& operator=( const StaticModelAsset& ) = delete;
		StaticModelAsset& operator=( StaticModelAsset&& ) = delete;

		// Bounds Method
		const AABB& GetAABB() const { return m_BoundingBox; }
		const BoundingSphere& GetBoundingSphere() const { return m_BoundingSphere; }

		inline FilePath GetDiskPath() const { return m_DiskPath; }

		// LOD Methods
		std::vector< StaticModelAssetLOD >::const_iterator LODBegin() const { return m_LODs.begin(); }
		std::vector< StaticModelAssetLOD >::const_iterator LODEnd() const { return m_LODs.end(); }
		std::vector< StaticModelAssetLOD >::const_iterator GetLOD( uint8 inIndex ) const;
		uint8 GetNumLODs() const { return (uint8)m_LODs.size(); }

		// SubObject Methods
		uint8 GetNumSubModels() const;
		std::vector< StaticModelAssetSubModel >::const_iterator GetSubModelBegin( uint8 inLOD ) const;
		std::vector< StaticModelAssetSubModel >::const_iterator GetSubModelEnd( uint8 inLOD ) const;
		std::vector< StaticModelAssetSubModel >::const_iterator GetSubModel( uint8 inLOD, uint8 inIndex ) const;

		friend class HSMReader;

		// Loader Function
		static std::shared_ptr< AssetBase > LoadFromFile( std::unique_ptr< File >& inFile, const String& inPath, uint32 inIdentifier, uint64 inOffset, uint64 inLength );
	};


}