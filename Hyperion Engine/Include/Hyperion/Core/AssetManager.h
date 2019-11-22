/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/AssetManager.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Asset.h"
#include "Hyperion/Core/String.h"

#include <atomic>
#include <unordered_map>

namespace Hyperion
{
	enum class AssetCacheMode
	{
		Dynamic = 0,
		Static = 1,
		Stream = 2
	};

	struct AssetManifestEntry
	{
		String Name;
		std::streamoff Begin;
		std::streamoff Length;
		String Type;
	};

	struct GroupManifestEntry
	{
		String Name;
		String ChunkFile;
		AssetCacheMode Mode;
		std::unordered_map< String, AssetManifestEntry > Assets;
	};

	class AssetManifest
	{

		static std::unordered_map< String, GroupManifestEntry > m_Manifest;
		static std::unordered_map< String, String > m_AssetIndex;
		static GroupManifestEntry m_InvalidGroup;
		static AssetManifestEntry m_InvalidAsset;

	public:

		AssetManifest() = delete;

		static std::pair< bool, GroupManifestEntry& > FindGroupEntry( const String& groupIdentifier );

		static std::pair< bool, AssetManifestEntry& > FindAssetEntry( const String& assetIdentifier );
		static std::pair< bool, AssetManifestEntry& > FindAssetEntry( const String& assetIdentifier, const String& groupIdentifier );

		static void LoadSync();
	};

	class AssetManager
	{

	private:

		static std::unordered_map< String, std::shared_ptr< AssetInstance > > m_Assets;
		static std::vector< String > m_CachedGroups;

	public:

		AssetManager() = delete;

		/*
			LoadSync
		*/
		template< typename T >
		static AssetRef< T > LoadSync( const String& assetIdentifier )
		{
			return nullptr;
		}

		/*
			LoadAsync
		*/
		// TODO

		/*
			IsCached
		*/
		static bool IsCached( const String& assetIdentifier );

		/*
			CacheGroupSync
		*/
		static bool CacheGroupSync( const String& groupIdentifier );

		/*
			CacheGroupAsync
		*/
		// TODO

		/*
			IsGroupCached
		*/
		static bool IsGroupCached( const String& groupIdentifier );

		/*
			FreeGroup
		*/
		static bool FreeGroup( const String& groupIdentifier );
	};

}