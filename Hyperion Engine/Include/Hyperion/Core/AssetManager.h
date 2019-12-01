/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/AssetManager.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Asset.h"
#include "Hyperion/Core/String.h"
#include "Hyperion/Core/VirtualFileSystem.h"
#include "Hyperion/Core/AssetLoader.h"

#include <atomic>
#include <unordered_map>

namespace Hyperion
{

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
			HYPERION_VERIFY_BASICSTR( assetIdentifier );
			// TODO: ToLower( assetIdentifier );

			if( assetIdentifier.IsEmpty() ) return nullptr;

			// First, check the active cache for this asset
			auto assetEntry = m_Assets.find( assetIdentifier );
			if( assetEntry != m_Assets.end() )
			{
				// Ensure this asset is valid
				if( assetEntry->second && assetEntry->second->m_Ref )
				{
					// Cast the asset to the desired type
					std::shared_ptr< T > castedAsset = std::dynamic_pointer_cast< T >( assetEntry->second->m_Ref );
					if( !castedAsset ) return nullptr; 

					return AssetRef< T >( castedAsset, assetEntry->second );
				}
				else
				{
					// Free this, and re-load
					m_Assets.erase( assetEntry );
				}
			}

			// Since we didnt have this asset cached already, we need to find its manifest entry so we can load it from disk
			if( VirtualFileSystem::FileExists( assetIdentifier ) )
			{
				auto streamResult = VirtualFileSystem::StreamFile( assetIdentifier );
				if( streamResult )
				{
					std::shared_ptr< Asset > assetRef = AssetLoader::StreamFromIdentifier( assetIdentifier, *streamResult );
					
					// Now we need to try and cast to the desired type
					std::shared_ptr< T > castedAsset = std::dynamic_pointer_cast< T >( assetRef );
					if( !castedAsset )
					{
						// If this fails, then we give up and return null
						return nullptr;
					}

					// Now, we need to setup a cache entry for this new asset 
					auto& cacheEntry = m_Assets[ assetIdentifier ] = std::make_shared< AssetInstance >();

					cacheEntry->m_Cached	= false;
					cacheEntry->m_Ref		= assetRef;
					cacheEntry->m_RefCount	= 0;

					// Create an AssetRef and return it
					// The AssetRef constructor will increment the ref counter
					return AssetRef< T >( castedAsset, cacheEntry );
				}
			}

			// We couldnt find the asset within the cache, and it didnt exist on the virtual file system.. 
			// So, we can check the actual disk for this asset before failing.. this allows developers to iterate designs quicker
			auto path = FilePath( assetIdentifier, PathRoot::Assets );

			if( IFileSystem::FileExists( path ) )
			{
				auto file = IFileSystem::OpenFile( path, FileMode::Read );
				if( file )
				{
					// Create AssetStream, so we can stream this asset data in
					auto f_size = file->Size();
					AssetStream stream( std::move( file ), 0, f_size );

					std::shared_ptr< Asset > assetRef = AssetLoader::StreamFromIdentifier( assetIdentifier, stream );

					// Cast to desired type and check for null
					std::shared_ptr< T > castedAsset = std::dynamic_pointer_cast< T >( assetRef );
					if( !castedAsset )
					{
						return nullptr;
					}

					auto& cacheEntry = m_Assets[ assetIdentifier ] = std::make_shared< AssetInstance >();

					cacheEntry->m_Cached	= false;
					cacheEntry->m_Ref		= assetRef;
					cacheEntry->m_RefCount	= 0;

					// Construct asset reference to return to caller
					return AssetRef< T >( castedAsset, cacheEntry );
				}
			}

			return AssetRef< T >( nullptr );
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
		static bool CacheGroupSync( const String& groupIdentifier, bool bBatchRead = false );

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