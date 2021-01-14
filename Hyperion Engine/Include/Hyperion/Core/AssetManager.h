/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/AssetManager.h
	© 2019, Zachary Berry
==================================================================================================*/

/*
*	TODO:
*	- Implement universal asset header, containing the asset type identifier, if the asset should be prema-cached
*	- Cache groups
*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Asset.h"
#include "Hyperion/File/FileSystem.h"
#include "Hyperion/Library/Crypto.h"

#include <memory>


namespace Hyperion
{

	struct AssetInstanceInfo
	{

	public:

		uint32 Identifier;
		String Path;
		String Bundle;
		uint64 Offset;
		uint64 Length;
		//String CacheGroup; // TODO: Implement this
		bool AlwaysCached;
		uint32 AssetType;

		std::weak_ptr< AssetBase > Instance;
	};

	struct AssetTypeInfo
	{
		uint32 Identifier;
		String Name;
		String Extension;
		std::function< std::shared_ptr< AssetBase >( std::unique_ptr< File >&, const String&, uint32, uint64, uint64 ) > LoaderFunc;
	};


	class AssetManager
	{

	private:
		
		// NEW
		static std::map< uint32, AssetInstanceInfo > m_Cache;
		static std::map< uint32, AssetTypeInfo >& GetTypes();

		static std::mutex m_CacheMutex;

	public:

		/*
			bool [True if exists] AssetManager::TypeExists( uint32 [ID of target type] )
			- Checks to see if an asset type exists by identifier
		*/
		static bool TypeExists( uint32 inTypeIdentifier );

		/*
			bool [True if success] AssetManager::GetType( uint32 [ID of target type], AssetTypeInfo& [OUT the info about the type] )
			- Gets info about an asset type by identifier
		*/
		static bool GetType( uint32 inTypeIdentifier, AssetTypeInfo& outInfo );


		static uint32 FindTypeFromExtension( const String& inExt );

		/*
			bool [True if added] AssetManager::RegisterType( const AssetTypeInfo& [Type info structure] )
			- Adds a new type to the asset manager
		*/
		static bool RegisterType( const AssetTypeInfo& inInfo );

		/*
			bool [True if added] AssetManager::RegisterAsset( const AssetInstanceInfo& [Info about the asset to register] )
			- Adds a new assets metadata to the cache
		*/
		static bool RegisterAsset( const AssetInstanceInfo& inEntry );

		/*
			bool [True if found] AssetManager::GetAssetInfo( uint32 [Asset identifier to find], AssetInstanceInfo& [OUT info about the asset] )
			- Finds info about an asset by identifier
		*/
		static bool GetAssetInfo( uint32 inIdentifier, AssetInstanceInfo& outEntry );

		/*
			bool [True if found] AssetManager::GetAssetInfo( const String& [Asset path to find], AssetInstanceInfo& [OUT info about the asset] )
			- Finds info about an asset by the asset's path
		*/
		static bool GetAssetInfo( const String& inPath, AssetInstanceInfo& outEntry );

		/*
			uint32 [Result] AssetManager::CalculateIdentifier( const String& [Asset path] )
			- Takes the path for an asset, and calculates what the numeric identifier would be
		*/
		static uint32 CalculateIdentifier( const String& inPath );

		/*
			bool [True if registered] AssetManager::Exists( uint32 [Asset identifier] )
			- Checks if an asset is registered with the AssetManager using the numeric identifier
			- This DOES NOT mean that asset is cached!
		*/
		static bool Exists( uint32 inIdentifier );

		/*
			bool [True if registered] AssetManager::Exists( const String& [Asset path] )
			- Checks if an asset is registered with the AssetManager using the Asset's path
			- This DOES NOT mean that asset is cached!
		*/
		static bool Exists( const String& inPath );

		/*
			bool [True if cached] AssetManager::IsCached( uint32 [Asset Identifier] )
			- Checks if an asset is currently cached using the numeric identifier
		*/
		static bool IsCached( uint32 inIdentifier );

		/*
			bool [True if cached] AssetManager::IsCached( const String& [Asset Path] )
			- Checks if an asset is currently cached using the asset's path
		*/
		static bool IsCached( const String& inPath );

		/*
			std::shared_ptr< AssetBase > [Asset result] AssetManager::GetGeneric( uint32 [Asset Identifier] )
			- Gets an asset by identifier, and returns it as the generic AssetBase
			- First checks the cache (if applicable) and if its not found, loads it from file
		*/
		static std::shared_ptr< AssetBase > GetGeneric( uint32 inIdentifier )
		{
			if( inIdentifier == ASSET_INVALID ) { return nullptr; }

			// Get this assets info from the cache, if it doesnt exist, then we fail the function
			// We need to get a lock on the mutex to read the entry from the asset cache, copy into local structure and release the lock
			AssetInstanceInfo assetInfo;
			{
				std::lock_guard< std::mutex > cacheLock( m_CacheMutex );

				auto entry = m_Cache.find( inIdentifier );
				if( entry == m_Cache.end() || entry->second.Identifier == ASSET_INVALID || entry->second.AssetType == ASSET_TYPE_INVALID )
				{
					Console::WriteLine( "[Warning] AssetManager: Failed to find metadata for asset! (Identifier: ", inIdentifier, ")" );
					return nullptr;
				}
				else if( !entry->second.Instance.expired() )
				{
					// Returned the cached version
					return entry->second.Instance.lock();
				}

				assetInfo = entry->second;
			}

			// Find the correct type info, so we can use the loader
			auto& types = GetTypes();

			auto typeEntry = types.find( assetInfo.AssetType );
			if( typeEntry == types.end() || !typeEntry->second.LoaderFunc )
			{
				Console::WriteLine( "[Warning] AssetManager: Failed to find loader for asset type (AssetType#", assetInfo.AssetType, ") when caching \"", assetInfo.Path, "\"" );
				return nullptr;
			}

			// Next, we need to actually open the file where the asset is contained
			auto& typeInfo		= typeEntry->second;

			uint64 offset	= 0;
			uint64 length	= 0;
			String path;

			// Determine if the asset is contained within a bundle, or is standalone on disk
			if( assetInfo.Bundle.IsEmpty() )
			{
				path = assetInfo.Path;
				if( !path.EndsWith( typeInfo.Extension ) )
				{
					Console::WriteLine( "[Warning] AssetManager: Failed to load asset (", assetInfo.Path, ") from file! The extension is not valid for an asset of type \"", typeInfo.Name, "\"" );
					return nullptr;
				}

				if( !FileSystem::FileExists( FilePath( path, PathRoot::Content ) ) )
				{
					Console::WriteLine( "[Warning] AssetManager: Failed to load asset (", assetInfo.Path, ") from file!" );
					return nullptr;
				}
			}
			else
			{
				path		= assetInfo.Bundle;
				offset		= assetInfo.Offset;
				length		= assetInfo.Length;

				if( !FileSystem::FileExists( FilePath( path, PathRoot::Content ) ) )
				{
					Console::WriteLine( "[Warning] AssetManager: Failed to load asset (", assetInfo.Path, ") from file! The bundle (", assetInfo.Bundle, ") its inside doesnt exist" );
					return nullptr;
				}
			}

			// Open the file
			auto f = FileSystem::OpenFile( FilePath( path, PathRoot::Content ), FileMode::Read );
			if( !f || !f->IsValid() || f->GetSize() < offset + length )
			{
				Console::WriteLine( "[Warning] AssetManager: Failed to load asset (", assetInfo.Path, ") from file! It couldnt be opened or target range is out of bounds" );
				return nullptr;
			}

			// Invoke the loader
			auto instance = typeInfo.LoaderFunc( f, assetInfo.Path, assetInfo.Identifier, offset, length );
			if( !instance )
			{
				Console::WriteLine( "[Warning] AssetManager: Failed to load asset (", assetInfo.Path, "), the loader function failed!" );
				return nullptr;
			}

			return instance;
		}

		/*
			std::shared_ptr< AssetBase > [Asset result] AssetManager::GetGeneric( const String& [Asset path] )
			- Gets an asset by path, and returns it as a generic AssetBase result
			- First checks the cache (if applicable) and if its not found, lods it from file
		*/
		static std::shared_ptr< AssetBase > GetGeneric( const String& inPath )
		{
			// Quick validation of the path
			if( inPath.Length() < 2 || inPath.IsWhitespace() )
			{
				Console::WriteLine( "[Warning] AssetManager: Failed to load asset, invalid path!" );
				return nullptr;
			}

			// Calculate the numeric identifier, and load the instance
			auto inIdentifier = CalculateIdentifier( inPath );
			if( inIdentifier == ASSET_INVALID ) { return nullptr; }

			// Get this assets info from the cache, if it doesnt exist, then we fail the function
			// We need to get a lock on the mutex to read the entry from the asset cache, copy into local structure and release the lock
			AssetInstanceInfo assetInfo;
			{
				std::lock_guard< std::mutex > cacheLock( m_CacheMutex );

				auto entry = m_Cache.find( inIdentifier );
				if( entry == m_Cache.end() || entry->second.Identifier == ASSET_INVALID || entry->second.AssetType == ASSET_TYPE_INVALID )
				{
					Console::WriteLine( "[Warning] AssetManager: Failed to find metadata for asset! (Path: ", inPath, ")" );
					return nullptr;
				}
				else if( !entry->second.Instance.expired() )
				{
					// Returned the cached version
					return entry->second.Instance.lock();
				}

				assetInfo = entry->second;
			}

			// Find the correct type info, so we can use the loader
			auto& types = GetTypes();

			auto typeEntry = types.find( assetInfo.AssetType );
			if( typeEntry == types.end() || !typeEntry->second.LoaderFunc )
			{
				Console::WriteLine( "[Warning] AssetManager: Failed to find loader for asset type (AssetType#", assetInfo.AssetType, ") when caching \"", assetInfo.Path, "\"" );
				return nullptr;
			}

			// Next, we need to actually open the file where the asset is contained
			auto& typeInfo		= typeEntry->second;

			uint64 offset	= 0;
			uint64 length	= 0;
			String path;

			// Determine if the asset is contained within a bundle, or is standalone on disk
			if( assetInfo.Bundle.IsEmpty() )
			{
				path = assetInfo.Path;
				if( !path.EndsWith( typeInfo.Extension ) )
				{
					Console::WriteLine( "[Warning] AssetManager: Failed to load asset (", assetInfo.Path, ") from file! The extension is not valid for an asset of type \"", typeInfo.Name, "\"" );
					return nullptr;
				}

				if( !FileSystem::FileExists( FilePath( path, PathRoot::Content ) ) )
				{
					Console::WriteLine( "[Warning] AssetManager: Failed to load asset (", assetInfo.Path, ") from file!" );
					return nullptr;
				}
			}
			else
			{
				path		= assetInfo.Bundle;
				offset		= assetInfo.Offset;
				length		= assetInfo.Length;

				if( !FileSystem::FileExists( FilePath( path, PathRoot::Content ) ) )
				{
					Console::WriteLine( "[Warning] AssetManager: Failed to load asset (", assetInfo.Path, ") from file! The bundle (", assetInfo.Bundle, ") its inside doesnt exist" );
					return nullptr;
				}
			}

			// Open the file
			auto f = FileSystem::OpenFile( FilePath( path, PathRoot::Content ), FileMode::Read );
			if( !f || !f->IsValid() || f->GetSize() < offset + length )
			{
				Console::WriteLine( "[Warning] AssetManager: Failed to load asset (", assetInfo.Path, ") from file! It couldnt be opened or target range is out of bounds" );
				return nullptr;
			}

			// Invoke the loader
			auto instance = typeInfo.LoaderFunc( f, assetInfo.Path, assetInfo.Identifier, offset, length );
			if( !instance )
			{
				Console::WriteLine( "[Warning] AssetManager: Failed to load asset (", assetInfo.Path, "), the loader function failed!" );
				return nullptr;
			}

			return instance;
		}

		/*
			std::shared_ptr< _Ty > [Casted asset result] AssetManager::Get( uint32 [Asset identifier] )
			- Gets an asset by identifier, and casts to desired type
			- First checks the cache (if applicable) and if its not found, loads it from file
		*/
		template< typename _Ty >
		static std::shared_ptr< _Ty > Get( uint32 inIdentifier )
		{
			// Check if the asset id is valid
			if( inIdentifier == ASSET_INVALID ) { return nullptr; }

			auto instance = GetGeneric( inIdentifier );
			if( !instance ) { return nullptr; }

			return std::dynamic_pointer_cast<_Ty>( instance );
		}

		/*
			std::shared_ptr< _Ty > [Casted asset result] AssetManager::Get( const String& [Asset Path] )
			- Gets an asset by the path, and casts it to the desired type
			- First checks the cache (if applicable) and if not found, loads it from file
		*/
		template< typename _Ty >
		static std::shared_ptr< _Ty > Get( const String& inPath )
		{
			return std::dynamic_pointer_cast< _Ty >( GetGeneric( inPath ) );
		}
		
	};

}