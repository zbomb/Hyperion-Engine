/*==================================================================================================
	Hyperion Engine
	Source/Core/AssetManager.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/AssetManager.h"



namespace Hyperion
{
	/*
		Static Member Definitions
	*/
	std::unordered_map< String, std::shared_ptr< AssetInstance > > AssetManager::m_Assets;
	std::vector< String > AssetManager::m_CachedGroups;

	std::unordered_map< String, GroupManifestEntry > AssetManifest::m_Manifest;
	std::unordered_map< String, String > AssetManifest::m_AssetIndex;
	GroupManifestEntry AssetManifest::m_InvalidGroup;
	AssetManifestEntry AssetManifest::m_InvalidAsset;

	/*
		Asset Manifest
	*/
	void AssetManifest::LoadSync()
	{

	}

	
	std::pair< bool, GroupManifestEntry& > AssetManifest::FindGroupEntry( const String& groupIdentifier )
	{
		HYPERION_VERIFY_BASICSTR( groupIdentifier );

		// TODO: ToLower( groupIdentifier );

		auto groupEntry = m_Manifest.find( groupIdentifier );
		if( groupEntry == m_Manifest.end() )
		{
			// Group not found!
			return std::pair< bool, GroupManifestEntry& >( false, m_InvalidGroup );
		}
		else
		{
			return std::pair< bool, GroupManifestEntry& >( true, groupEntry->second );
		}
	}

	std::pair< bool, AssetManifestEntry& > AssetManifest::FindAssetEntry( const String& assetIdentifier, const String& groupIdentifier )
	{
		HYPERION_VERIFY_BASICSTR( assetIdentifier );
		HYPERION_VERIFY_BASICSTR( groupIdentifier );

		// TODO: ToLower( assetIdentifier ), ToLower( groupIdentifier );

		// First, we need to find the target group
		auto groupEntry = m_Manifest.find( groupIdentifier );
		if( groupEntry == m_Manifest.end() ) return std::pair< bool, AssetManifestEntry& >( false, m_InvalidAsset );

		// Now try and find this asset in the group
		auto assetEntry = groupEntry->second.Assets.find( assetIdentifier );
		if( assetEntry == groupEntry->second.Assets.end() ) return std::pair< bool, AssetManifestEntry& >( false, m_InvalidAsset );

		return std::pair< bool, AssetManifestEntry& >( true, assetEntry->second );
	}

	std::pair< bool, AssetManifestEntry& > AssetManifest::FindAssetEntry( const String& assetIdentifier )
	{
		HYPERION_VERIFY_BASICSTR( assetIdentifier );

		// TODO: ToLower( assetIdentifier )

		// Use the secondary index to quickly find the proper group
		auto targetGroup = m_AssetIndex.find( assetIdentifier );
		if( targetGroup == m_AssetIndex.end() ) return std::pair< bool, AssetManifestEntry& >( false, m_InvalidAsset );

		return FindAssetEntry( assetIdentifier, targetGroup->second );
	}




	bool AssetManager::IsCached( const String& groupIdentifier )
	{
		HYPERION_VERIFY_BASICSTR( groupIdentifier );

		// TODO: Convert string to lowercase

		for( auto& g : m_CachedGroups )
		{
			if( g.Equals( groupIdentifier ) )
				return true;
		}

		return false;
	}

	bool AssetManager::CacheGroupSync( const String& groupIdentifier )
	{
		HYPERION_VERIFY_BASICSTR( groupIdentifier );

		// TODO: ToLower( groupIdentifier );

		// Check if this group is already cached
		if( IsCached( groupIdentifier ) ) return true;

		// Find this groups manifest info, if exists
		auto manifestEntry = AssetManifest::FindGroupEntry( groupIdentifier );
		if( !manifestEntry.first )
		{
			// Group wasnt found....
			return false;
		}

		if( manifestEntry.second.Mode != AssetCacheMode::Dynamic ) return false;

		// Add this group to the 'cached' list
		m_CachedGroups.push_back( groupIdentifier );

		// Now, go through and cache each indivisual asset
		for( auto It = manifestEntry.second.Assets.begin(); It != manifestEntry.second.Assets.end(); It++ )
		{
			// Check if this asset is already in the cache
			auto existing = m_Assets.find( It->first );
			if( existing != m_Assets.end() )
			{
				// Already in the asset list.. so flag as 'cached'
				existing->second->m_Cached = true;
			}
			else
			{
				// We need to load this asset from memory
				// auto inst = AssetLoader::CreateFromChunk( manifestEntry.second.ChunkFile, It->second );
				// TODO
				return false;
			}
		}

		return true;
	}

	bool AssetManager::IsGroupCached( const String& groupIdentifier )
	{
		HYPERION_VERIFY_BASICSTR( groupIdentifier );

		// TODO: ToLower( groupIdentifier );

		for( auto& g : m_CachedGroups )
		{
			if( g.Equals( groupIdentifier ) ) return true;
		}

		return false;
	}

	bool AssetManager::FreeGroup( const String& groupIdentifier )
	{
		HYPERION_VERIFY_BASICSTR( groupIdentifier );

		// TODO: ToLower( groupIdentifier );

		// Check if this group is even cached, and if so, erase from list
		bool bFound = false;
		for( auto It = m_CachedGroups.begin(); It != m_CachedGroups.end(); )
		{
			if( It->Equals( groupIdentifier ) )
			{
				It = m_CachedGroups.erase( It );
				bFound = true;
			}
			else
			{
				It++;
			}
		}

		if( !bFound )
		{
			return true;
		}

		// Now, we need to go through and uncache all associated assets
		auto groupManifest = AssetManifest::FindGroupEntry( groupIdentifier );
		if( !groupManifest.first ) return false;

		for( auto It = groupManifest.second.Assets.begin(); It != groupManifest.second.Assets.end(); It++ )
		{
			auto& assetIdentifier = It->first;

			// Find this asset in the cache list
			auto assetEntry = m_Assets.find( It->first );
			if( assetEntry != m_Assets.end()  )
			{
				if( assetEntry->second )
				{
					assetEntry->second->m_Cached = false;
					if( assetEntry->second->m_RefCount <= 0 )
					{
						// TODO: More things to do when freeing asset?
						m_Assets.erase( It->first );
					}
				}
				else
				{
					// If this asset is invalid, then we will just erase it anyway
					m_Assets.erase( It->first );
				}
			}
		}

	}


}