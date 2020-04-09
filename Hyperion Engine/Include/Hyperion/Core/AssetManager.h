/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/AssetManager.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Asset.h"


namespace Hyperion
{

	class AssetManager
	{

	private:
		
		static std::map< uint32, String > m_HashTable;

	public:

		static bool RegisterAsset( uint32 inHash, const String& inPath );
		static String GetAssetPath( uint32 inHash );
		static uint32 GetAssetIdentifier( const String& inPath );

		template< typename _Ty, std::enable_if_t<
			std::is_base_of< AssetBase, _Ty >::value &&
			std::is_base_of< IAssetCache, typename _Ty::_CacheType >::value &&
			std::is_base_of< IAssetLoader, typename _Ty::_LoaderType >::value &&
			std::is_function< typename _Ty::GetCacheMethod >::value > >
		static std::shared_ptr< _Ty > Get( uint32 inIdentifier );

		template< typename _Ty, std::enable_if_t<
			std::is_base_of< AssetBase, _Ty >::value &&
			std::is_base_of< IAssetCache, typename _Ty::_CacheType >::value &&
			std::is_base_of< IAssetLoader, typename _Ty::_LoaderType >::value &&
			std::is_function< typename _Ty::GetCacheMethod >::value > >
			static std::shared_ptr< _Ty > Get( const String& inPath );

		template< typename _Ty, std::enable_if_t<
			std::is_base_of< AssetBase, _Ty >::value &&
			std::is_base_of< IAssetCache, typename _Ty::_CacheType >::value &&
			std::is_base_of< IAssetLoader, typename _Ty::_LoaderType >::value &&
			std::is_function< typename _Ty::GetCacheMethod >::value > >
		static std::shared_ptr< _Ty > GetUnique( uint32 inIdentifier );

		template< typename _Ty, std::enable_if_t<
			std::is_base_of< AssetBase, _Ty >::value &&
			std::is_base_of< IAssetCache, typename _Ty::_CacheType >::value &&
			std::is_base_of< IAssetLoader, typename _Ty::_LoaderType >::value &&
			std::is_function< typename _Ty::GetCacheMethod >::value > >
		static std::shared_ptr< _Ty > GetUnique( const String& inPath );

		template< typename _Ty, std::enable_if_t<
			std::is_base_of< AssetBase, _Ty >::value &&
			std::is_base_of< IAssetCache, typename _Ty::_CacheType >::value &&
			std::is_base_of< IAssetLoader, typename _Ty::_LoaderType >::value &&
			std::is_function< typename _Ty::GetCacheMethod >::value > >
		static std::shared_ptr< _Ty > GetCached( uint32 inIdentifier );

		template< typename _Ty, std::enable_if_t<
			std::is_base_of< AssetBase, _Ty >::value &&
			std::is_base_of< IAssetCache, typename _Ty::_CacheType >::value &&
			std::is_base_of< IAssetLoader, typename _Ty::_LoaderType >::value &&
			std::is_function< typename _Ty::GetCacheMethod >::value > >
		static std::shared_ptr< _Ty > GetCached( const String& inPath );

		
	};

}













/*
#include "Hyperion/Core/Asset.h"
#include "Hyperion/Core/String.h"
#include "Hyperion/Core/VirtualFileSystem.h"
#include "Hyperion/Core/AssetLoader.h"
#include "Hyperion/Core/Async.h"

#include <atomic>
#include <unordered_map>
#include <shared_mutex>

namespace Hyperion
{


	class AssetManager
	{

	private:

		static std::unordered_map< String, std::shared_ptr< AssetInstance > > m_Assets;
		static std::vector< String > m_CachedGroups;
		static std::shared_mutex m_CacheMutex;

		template< typename T >
		static AssetRef< T > Impl_LoadShared( const String& assetIdentifier, bool& bFailed )
		{
			// This boolean is set to true if the cast failed
			bFailed = false;

			// Attempt to load from the cache using a shared lock
			// This means, if the asset isnt in the cache, then we have to defer to
			// the unique lock version of this function to actually load the asset
			{
				std::shared_lock< std::shared_mutex > lock( m_CacheMutex );

				auto entry = m_Assets.find( assetIdentifier );
				if( entry != m_Assets.end() )
				{
					// If this asset is invalid, or is still loading, then we cant do anything in this function
					if( !entry->second || !entry->second->m_Loaded || !entry->second->m_Ref )
					{
						return nullptr;
					}

					std::shared_ptr< T > castedAsset = std::dynamic_pointer_cast< T >( entry->second->m_Ref );
					if( castedAsset )
					{
						return AssetRef< T >( castedAsset, entry->second, AssetPath( entry->first, AssetLocation:: );
					}
					else
					{
						bFailed = true;
						return nullptr;
					}
				}
			}

			return nullptr;
		}

		template< typename T >
		static AssetRef< T > Impl_LoadUnique( const String& assetIdentifier, bool bCheckDisk = true )
		{
			{
				std::unique_lock< std::shared_mutex > lock( m_CacheMutex );

				auto entry = m_Assets.find( assetIdentifier );
				if( entry != m_Assets.end() )
				{
					auto state = entry->second;

					// If the state is null, or its 'loaded' but the ref is null, then erase it, and reload the asset
					if( !state || ( !state->m_Ref && state->m_Loaded ) )
					{
						m_Assets.erase( entry );
					}
					else
					{
						// If another call is in the middle of loading this asset, then wait for it to finish
						if( !state->m_Loaded )
						{
							state->m_Wait.wait( lock, [ state ]{ return state->m_Loaded; } );

							// Now return the result (if we can cast it properly and its valid)
							// Were going to reaquire the iterator, incase the map changed more than we expected
							auto new_entry = m_Assets.find( assetIdentifier );
							if( new_entry != m_Assets.end() )
							{
								auto new_state = new_entry->second;
								if( new_state && new_state->m_Ref && new_state->m_Loaded )
								{
									std::shared_ptr< T > casted_asset = std::dynamic_pointer_cast< T >( new_state->m_Ref );
									if( casted_asset )
									{
										return AssetRef< T >( casted_asset, new_state, new_entry->first );
									}
								}
							}

							// If we couldnt get the asset after waiting for the function to finish loading, we assume
							// the load of this asset failed (or we specified an invalid cast) so we return null
							return nullptr;
						}
						else
						{
							// Already loaded, so we can cast and attempt to return
							std::shared_ptr< T > casted_asset = std::dynamic_pointer_cast< T >( state->m_Ref );
							if( casted_asset )
							{
								return AssetRef< T >( casted_asset, state, entry->first );
							}

							return nullptr;
						}
					}
				}

				// If we get here, then we werent able to get the asset from the cache, and we need to load it ourself
				// So first, we need to insert an entry into the cache to indicate were going to be loading it before we release the lock
				auto& newEntry = m_Assets[ assetIdentifier ] = std::make_shared< AssetInstance >();

				newEntry->m_Cached		= false;
				newEntry->m_Loaded		= false;
				newEntry->m_Ref			= nullptr;
				newEntry->m_RefCount	= 0;
				newEntry->m_Location	= AssetLocation::FileSystem;
			}

			std::shared_ptr< T > newAsset = nullptr;

			// First, were going to attempt to load this asset from the virtual file system
			if( VirtualFileSystem::FileExists( assetIdentifier ) )
			{
				auto streamResult = VirtualFileSystem::StreamFile( assetIdentifier );
				if( streamResult )
				{
					std::shared_ptr< Asset > assetRef = AssetLoader::StreamFromFileName( assetIdentifier, *streamResult );
					assetRef->m_Path = AssetPath( assetIdentifier, AssetLocation::Virtual );
					
					// Now we need to try and cast to the desired type
					newAsset = std::dynamic_pointer_cast< T >( assetRef );
					if( !newAsset )
					{
						// If we found the asset, but the cast failed, then dont bother checking disk
						bCheckDisk = false;
					}
				}
			}

			// If we werent able to load from the VFS, then check disk
			if( !newAsset && bCheckDisk )
			{
				auto path = FilePath( assetIdentifier, PathRoot::Assets );

				if( IFileSystem::FileExists( path ) )
				{
					auto file = IFileSystem::OpenFile( path, FileMode::Read );
					if( file )
					{
						// Create AssetStream, so we can stream this asset data in
						auto f_size = file->Size();
						AssetStream stream( std::move( file ), 0, f_size );

						auto gasset = AssetLoader::StreamFromFileName( AssetPath( assetIdentifier, AssetLocation::FileSystem ), stream );
						gasset->m_Path = AssetPath( assetIdentifier, AssetLocation::FileSystem );

						newAsset = std::dynamic_pointer_cast<T>( gasset );
					}
				}
			}

			// Reaquire unique lock so we can indicate that we finished loading
			{
				std::unique_lock< std::shared_mutex > lock( m_CacheMutex );

				auto entry = m_Assets.find( assetIdentifier );
				if( entry == m_Assets.end() || !entry->second )
				{
					auto& i = m_Assets[ assetIdentifier ] = std::make_shared< AssetInstance >();

					i->m_Cached		= false;
					i->m_Loaded		= true;
					i->m_Ref		= newAsset;
					i->m_RefCount	= 0;
					i->m_Location	= bCheckDisk ? AssetLocation::FileSystem : AssetLocation::Virtual;
					
					// Somehow the entry went missing, so were going to recreate it and return
					return AssetRef< T >( newAsset, i, AssetLocation( entry->first, i->m_Location ) );
				}
				else
				{
					auto i = entry->second;

					i->m_Loaded		= true;
					i->m_Ref		= newAsset;
					i->m_RefCount	= 0;
					i->m_Location	= bCheckDisk ? AssetLocation::FileSystem : AssetLocation::Virtual;

					// Release the lock and trigger CV, we dont have to worry about the AssetInstance being erased
					// because its a shared_ptr, so we can safely unlock before building the asset ref were returning
					lock.unlock();

					i->m_Wait.notify_all();
					return AssetRef< T >( newAsset, i, AssetLocation( entry->first, i->m_Location ) );
				}
			}
		}

	public:

		AssetManager() = delete;

		/*
			LoadSync
			* Loads an asset from either the virtual file system or disk
			* If bCheckDisk is false, only the VFS will be checked for the asset
			* Can be called from any thread.. thread-safe!
		
		template< typename T >
		static AssetRef< T > LoadSync( const String& inAssetIdentifier, bool bCheckDisk = true )
		{
			// Verify the identifier argument, needs to be a basic string (non localized), lowercase, and not empty
			HYPERION_VERIFY_BASICSTR( inAssetIdentifier );
			auto assetIdentifier = inAssetIdentifier.TrimBoth().ToLower();

			if( assetIdentifier.IsEmpty() ) return nullptr;

			// First, attempt to read this from the cache, if we cant, then we have to call in a slower function
			// to either wait on another thread to finish loading this asset, or load it ourself
			bool bFailed = false;
			auto quick_res = Impl_LoadShared< T >( assetIdentifier, bFailed );

			// If we either got a valid result, or are unable to load this asset, then return the result
			if( quick_res || bFailed )
			{
				return quick_res;
			}

			return Impl_LoadUnique< T >( assetIdentifier, bCheckDisk );
		}

		/*
			LoadAsync
		
		template< typename T >
		static TaskHandle< AssetRef< T > > LoadAsync( const String& Identifier, bool bCheckDisk = true )
		{
			HYPERION_VERIFY_BASICSTR( Identifier );

			// Basically, were just going to perform asset loading on the thread pool
			// Issue is.. how to handle multi-threading with IFileSystem/VirtualFileSystem
			return Task::Create< AssetRef< T > >( std::bind( &AssetManager::LoadSync< T >, Identifier, bCheckDisk ) );
		}

		/*
			CacheGroupSync
		
		static bool CacheGroupSync( const String& groupIdentifier, bool bBatchRead = false );

		/*
			CacheGroupAsync
		
		static TaskHandle< bool > CacheGroupAsync( const String& groupIdentifier, bool bBatchRead = false );

		/*
			IsGroupCached
		
		static bool IsGroupCached( const String& groupIdentifier );

		/*
			FreeGroupSync
		
		static bool FreeGroupSync( const String& groupIdentifier );

		/*
			FreeGroupAsync
		
		static TaskHandle< bool > FreeGroupAsync( const String& groupIdentifier );

		/*
			IsAssetCached
		

		enum class CacheState
		{
			None = 0,
			Soft = 1,
			Hard = 2
		};

		static CacheState IsAssetCached( const String& assetIdentifier );

		static void CheckAsset( const std::shared_ptr< AssetInstance >& targetInstance );
	};

}
*/