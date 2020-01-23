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
	std::shared_mutex AssetManager::m_CacheMutex;
	std::unordered_map< String, std::shared_ptr< AssetInstance > > AssetManager::m_Assets;
	std::vector< String > AssetManager::m_CachedGroups;

	std::map< size_t,
		std::pair<
		std::function< std::shared_ptr< Asset >( const AssetPath&, std::vector< byte >::const_iterator, std::vector< byte >::const_iterator ) >,
		std::function< std::shared_ptr< Asset >( const AssetPath&, AssetStream& ) >
		>
	> AssetLoader::m_TypeToLoadMap;

	std::map< String, size_t > AssetLoader::m_ExtensionToTypeMap;



	void AssetRefBase::_IncRefCount()
	{
		if( m_Inst )
		{
			auto newRefCount = ++( m_Inst->m_RefCount );
			//Console::WriteLine( "[DEBUG] AssetRefBase: Incrementing... new ref count: ", newRefCount, "" );
		}
	}

	void AssetRefBase::_DecRefCount( const std::shared_ptr< AssetInstance >& Target )
	{
		if( Target )
		{
			// Decrement ref counter atomically
			auto newRefCount = --( Target->m_RefCount );
			//Console::WriteLine( "[DEBUG] AssetRefBase: Decrementing.. new ref count: ", newRefCount, "" );

			if( newRefCount <= 0 && !Target->m_Cached )
			{
				// Tell the asset manager this asset instance might need to be cleaned up
				AssetManager::CheckAsset( Target );
			}
		}
	}



	bool AssetManager::CacheGroupSync( const String& inGroupIdentifier, bool bBatchRead /* = false */ )
	{
		// Verify input string, and convert to lowercase
		HYPERION_VERIFY_BASICSTR( inGroupIdentifier );
		auto groupIdentifier = inGroupIdentifier.TrimBoth().ToLower();

		// Aquire shared lock to read the group list
		// Check if this group is already cached
		{
			std::shared_lock< std::shared_mutex > lock( m_CacheMutex );
			for( const auto& g : m_CachedGroups )
			{
				if( g.Equals( groupIdentifier ) )
					return true;
			}
		}

		// Find this groups manifest entry
		auto manifestEntry = VirtualFileSystem::FindGroupEntry( groupIdentifier );
		if( !manifestEntry.first )
		{
			// If there isnt a manifest for this group, return false
			return false;
		}

		// We can only cache dynamic asset groups...
		if( manifestEntry.second.Mode != AssetCacheMode::Dynamic ) return false;

		// Aquire unique lock so we can create cache entries
		std::map< String, std::shared_ptr< AssetInstance > > instancesToLoad;
		{
			std::unique_lock< std::shared_mutex > lock( m_CacheMutex );
			
			// Add this to the cached group list
			m_CachedGroups.push_back( groupIdentifier );

			// Loop through assets in this group
			for( auto It = manifestEntry.second.Assets.begin(); It != manifestEntry.second.Assets.end(); It++ )
			{
				// Check if this asset already has a cache entry
				auto entry = m_Assets.find( It->first );
				if( entry != m_Assets.end() )
				{
					if( !entry->second )
					{
						// If this asset state is null, then create a new state, set it up and schedule it to be loaded
						entry->second = std::make_shared< AssetInstance >();

						entry->second->m_Cached		= true;
						entry->second->m_Loaded		= false;
						entry->second->m_Ref		= nullptr;
						entry->second->m_RefCount	= 0;
						entry->second->m_Location	= AssetLocation::Virtual;

						instancesToLoad[ It->first ] = entry->second;
					}
				}
				else
				{
					// If this asset isnt in the cache, then were going to create an entry for it while were loading
					auto& i = m_Assets[ It->first ] = std::make_shared< AssetInstance >();

					i->m_Cached		= true;
					i->m_Loaded		= false;
					i->m_Ref		= nullptr;
					i->m_RefCount	= 0;
					i->m_Location	= AssetLocation::Virtual;

					instancesToLoad[ It->first ] = i;
				}
			}
		}

		std::map< String, std::shared_ptr< Asset > > loadedAssets;
		bool bBatchFallback = false;

		if( bBatchRead )
		{
			std::map< String, std::vector< byte > > assetData;
			if( VirtualFileSystem::BatchReadGroup( groupIdentifier, assetData ) )
			{
				for( auto It = instancesToLoad.begin(); It != instancesToLoad.end(); It++ )
				{
					// Ensure this instance is in the group we loaded
					auto dataEntry = assetData.find( It->first );
					if( dataEntry != assetData.end() )
					{
						// Load this asset from the raw data we read from disk
						auto asset_ptr = AssetLoader::LoadFromFileName( AssetPath( It->first, AssetLocation::Virtual ), dataEntry->second.begin(), dataEntry->second.end() );
						asset_ptr->m_Path = AssetPath( It->first, AssetLocation::Virtual );

						// Erase this data from the list of asset data, swap forces deallocation
						std::vector< byte >().swap( dataEntry->second );

						if( asset_ptr )
						{
							loadedAssets[ It->first ] = asset_ptr;
						}
						else
						{
							Console::WriteLine( "[ERROR] AssetManager: Failed to load an asset while caching a group.. '", It->first, "' couldnt be loaded!" );
						}
					}
				}
			}
			else
			{
				Console::WriteLine( "[ERROR] AssetManager: Failed to batch read asset group '", groupIdentifier, "', attempting to load indivisually" );
				bBatchFallback = true;
			}
		}

		// If were not batch reading, or if batch reading failed.. then were going to stream the assets in indivisually
		if( !bBatchRead || bBatchFallback )
		{
			for( auto It = instancesToLoad.begin(); It != instancesToLoad.end(); It++ )
			{
				// Get the manifest info, and since we built the 'to load' list from the manifest list, were just going to 
				// use 'at' to get the entry, instead of checking if it exists first
				auto& assetManifest = manifestEntry.second.Assets.at( It->first );

				// Stream this asset in from the virtual file system
				auto assetStream = VirtualFileSystem::StreamChunkSection( manifestEntry.second.ChunkFile, assetManifest.Begin, assetManifest.Begin + assetManifest.Length );
				if( !assetStream )
				{
					Console::WriteLine( "[ERROR] AssetManager: Failed to load an asset while caching a group.. '", It->first, "' couldnt be loaded from the virtual file system" );
					continue;
				}

				auto assetRef = AssetLoader::StreamFromFileName( AssetPath( It->first, AssetLocation::Virtual ), *assetStream );
				if( !assetRef )
				{
					Console::WriteLine( "[ERROR] AssetManager: Failed to load an asset while caching a group.. '", It->first, "' couldnt be constructed from raw data" );
					continue;
				}

				assetRef->m_Path = AssetPath( It->first, AssetLocation::Virtual );

				// Store the loaded asset into a list, so we can update the cache once finished
				loadedAssets[ It->first ] = assetRef;
			}
		}

		// Now that we have loaded all of the assets in this group, reaquire lock so we can update the cache
		{
			std::unique_lock< std::shared_mutex > lock( m_CacheMutex );

			// Go through the list of instances we added to the cache, and update them
			for( auto& instance : instancesToLoad )
			{
				if( instance.second )
				{
					instance.second->m_Loaded = true;

					// Check if we were able to load this asset, and set the ref if so
					auto entry = loadedAssets.find( instance.first );
					if( entry != loadedAssets.end() && entry->second )
					{
						instance.second->m_Ref = entry->second;
					}
				}
			}
		}

		// Now that the lock is released, were going to go through the list of instances we created earlier
		// and trigger the cv for each one.. this ensures no threads get stuck waiting for an asset to load
		for( auto& instance : instancesToLoad )
		{
			if( instance.second )
			{
				instance.second->m_Wait.notify_all();
			}
		}

		return true;
	}

	TaskHandle< bool > AssetManager::CacheGroupAsync( const String& Identifier, bool bBatchRead /* = false */ )
	{
		HYPERION_VERIFY_BASICSTR( Identifier );
		return Task::Create< bool >( std::bind( &AssetManager::CacheGroupSync, Identifier, bBatchRead ) );
	}


	bool AssetManager::IsGroupCached( const String& groupIdentifier )
	{
		HYPERION_VERIFY_BASICSTR( groupIdentifier );
		auto id = groupIdentifier.TrimBoth().ToLower();

		{
			std::shared_lock< std::shared_mutex > lock( m_CacheMutex );

			for( const auto& g : m_CachedGroups )
			{
				if( g.Equals( id ) )
					return true;
			}
		}

		return false;
	}


	bool AssetManager::FreeGroupSync( const String& groupIdentifier )
	{
		HYPERION_VERIFY_BASICSTR( groupIdentifier );
		auto id = groupIdentifier.TrimBoth().ToLower();

		{
			// Aquire unique lock on the cache
			std::unique_lock< std::shared_mutex > lock( m_CacheMutex );

			// Erase this group name from the cached groups list
			bool bFound = false;
			for( auto It = m_CachedGroups.begin(); It != m_CachedGroups.end(); )
			{
				if( It->Equals( id ) )
				{
					It = m_CachedGroups.erase( It );
					bFound = true;
				}
				else
				{
					It++;
				}
			}

			if( !bFound ) { return false; }

			// Next, find the list of assets in this group
			auto manifest = VirtualFileSystem::FindGroupEntry( id );
			if( !manifest.first ) { return false; }

			// Loop through the list of assets, and uncache each one
			for( auto It = manifest.second.Assets.begin(); It != manifest.second.Assets.end(); It++ )
			{
				auto& identifier = It->first;

				// Find cache entry for this asset
				auto entry = m_Assets.find( identifier );
				if( entry != m_Assets.end() )
				{
					if( entry->second )
					{
						entry->second->m_Cached = false;
						if( entry->second->m_RefCount <= 0 && entry->second->m_Loaded )
						{
							// TODO: Some sort of function to free the actual asset instance?
							m_Assets.erase( identifier );
						}
					}
					else
					{
						if( entry->second->m_Loaded )
						{ 
							// TODO: Some function to free the actual asset instance?
							m_Assets.erase( identifier );
						}
					}
				}
			}
		}

		return true;
	}


	TaskHandle< bool > AssetManager::FreeGroupAsync( const String& Identifier )
	{
		HYPERION_VERIFY_BASICSTR( Identifier );
		auto id = Identifier.TrimBoth().ToLower();

		return Task::Create< bool >( std::bind( &AssetManager::FreeGroupSync, id ) );
	}


	AssetManager::CacheState AssetManager::IsAssetCached( const String& fileName )
	{
		HYPERION_VERIFY_BASICSTR( fileName );
		auto id = fileName.TrimBoth().ToLower();

		{
			std::shared_lock< std::shared_mutex > lock( m_CacheMutex );

			auto entry = m_Assets.find( id );
			if( entry != m_Assets.end() )
			{
				if( entry->second )
				{
					if( entry->second->m_Cached )
					{
						return CacheState::Hard;
					}
					else
					{
						return CacheState::Soft;
					}
				}
			}
		}

		return CacheState::None;
	}


	void AssetManager::CheckAsset( const std::shared_ptr< AssetInstance >& targetInstance )
	{
		// Aquire a unique lock, so no assets are created while were performing this operation
		std::unique_lock< std::shared_mutex > lock( m_CacheMutex );

		// We dont really want to iterate through every single asset
		// But for now, this is the solution
		// TODO: Somehow call this function using a String, so we can call map::find instead
		for( auto It = m_Assets.begin(); It != m_Assets.end(); )
		{
			// Check for null instances
			if( !It->second )
			{
				Console::WriteLine( "[DEBUG] Destroying null asset instance" );
				It = m_Assets.erase( It );
			}
			else if( It->second == targetInstance && It->second->m_RefCount <= 0 && !It->second->m_Cached && It->second->m_Loaded )
			{
				Console::WriteLine( "[DEBUG] Destroying ref count 0 asset instance" );
				It = m_Assets.erase( It );
			}
			else
			{
				It++;
			}

		}
	}

}