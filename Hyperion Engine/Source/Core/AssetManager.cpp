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
	std::map< String, 
			std::pair< 
				std::function< std::shared_ptr< Asset >( std::vector< byte >::const_iterator, std::vector< byte >::const_iterator ) >,
				std::function< std::shared_ptr< Asset >( AssetStream& ) > 
			>
		> AssetLoader::m_LoadFuncMap;

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

	bool AssetManager::CacheGroupSync( const String& groupIdentifier, bool bBatchRead /* = false */ )
	{
		HYPERION_VERIFY_BASICSTR( groupIdentifier );

		// TODO: ToLower( groupIdentifier );

		// Check if this group is already cached
		if( IsCached( groupIdentifier ) ) return true;

		// Find this groups manifest info, if exists
		auto manifestEntry = VirtualFileSystem::FindGroupEntry( groupIdentifier );
		if( !manifestEntry.first )
		{
			// Group wasnt found....
			return false;
		}

		if( manifestEntry.second.Mode != AssetCacheMode::Dynamic ) return false;

		// Add this group to the 'cached' list
		m_CachedGroups.push_back( groupIdentifier );

		if( bBatchRead )
		{
			std::map< String, std::vector< byte > > assetData;
			if( VirtualFileSystem::BatchReadGroup( groupIdentifier, assetData ) )
			{
				// Loop through the list of asset data, and load each of them in
				for( auto It = assetData.begin(); It != assetData.end(); It++ )
				{
					// Ensure this asset isnt already cached
					auto existing = m_Assets.find( It->first );
					if( existing != m_Assets.end() && existing->second )
					{
						existing->second->m_Cached = true;
						It->second.erase( It->second.begin(), It->second.end() );
						continue;
					}

					std::shared_ptr< Asset > assetPtr = AssetLoader::LoadFromIdentifier( It->first, It->second.begin(), It->second.end() );
					
					// Erase the source vector to free up memeory as soon as possible
					It->second.erase( It->second.begin(), It->second.end() );

					if( !assetPtr )
					{
						std::cout << "[ERROR} AssetManager: Failed to cache group '" << groupIdentifier << "', the asset '" << It->first << "' couldnt be loaded!\n";
						continue;
					}

					auto& inst = m_Assets[ It->first ] = std::make_shared< AssetInstance >();

					inst->m_Cached		= true;
					inst->m_Ref			= assetPtr;
					inst->m_RefCount	= 0;
				}
			}
		}
		else
		{
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
					auto streamResult = VirtualFileSystem::StreamChunkSection( manifestEntry.second.ChunkFile, It->second.Begin, It->second.Begin + It->second.Length );
					if( !streamResult )
					{
						// Couldnt read this asset in!
						std::cout << "[ERROR] AssetManager: Failed to cache group '" << manifestEntry.second.Name << "', the asset '" << It->second.Name << "' couldnt be streamed in!\n";
						continue;
					}

					// Use AssetLoader to cache this asset
					std::shared_ptr< Asset > assetPtr = AssetLoader::StreamFromIdentifier( It->second.Name, *streamResult );
					if( !assetPtr )
					{
						// Couldnt create this asset from the stream!
						std::cout << "[ERROR] AssetManager: Failed to cache group '" << manifestEntry.second.Name << "', the asset '" << It->second.Name << "' failed to load from the data stream!\n";
						continue;
					}

					// Create cache entry for this asset
					auto& inst = m_Assets[ It->second.Name ] = std::make_shared< AssetInstance >();

					inst->m_Cached		= true;
					inst->m_Ref			= assetPtr;
					inst->m_RefCount	= 0;
				}
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
		auto groupManifest = VirtualFileSystem::FindGroupEntry( groupIdentifier );
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

		return true;
	}


}