/*==================================================================================================
	Hyperion Engine
	Source/Core/AssetManager.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/AssetManager.h"
#include "Hyperion/Core/AssetType.h"


namespace Hyperion
{
	// Static data structures
	std::map< uint32, AssetInstanceInfo > AssetManager::m_Cache;
	std::map< uint32, AssetTypeInfo > AssetManager::m_Types;
	std::mutex AssetManager::m_CacheMutex;


	// Function definitions
	bool AssetManager::TypeExists( uint32 inIdentifier )
	{
		return( inIdentifier != ASSET_TYPE_INVALID && m_Types.find( inIdentifier ) != m_Types.end() );
	}


	bool AssetManager::GetType( uint32 inIdentifier, AssetTypeInfo& outInfo )
	{
		outInfo.Identifier	= ASSET_TYPE_INVALID;
		outInfo.LoaderFunc	= nullptr;
		outInfo.Name.Clear();
		outInfo.Extension.Clear();

		if( inIdentifier == ASSET_TYPE_INVALID )
		{
			return false;
		}

		auto entry = m_Types.find( inIdentifier );
		if( entry == m_Types.end() || entry->second.Identifier == ASSET_TYPE_INVALID )
		{
			return false;
		}

		outInfo = entry->second;

		return true;
	}


	bool AssetManager::RegisterType( const AssetTypeInfo& inInfo )
	{
		if( inInfo.Identifier == ASSET_TYPE_INVALID || !inInfo.LoaderFunc || TypeExists( inInfo.Identifier ) )
		{
			return false;
		}

		m_Types.emplace( inInfo.Identifier, inInfo );
		return true;
	}


	bool AssetManager::RegisterAsset( const AssetInstanceInfo& inInfo )
	{
		// Perform validation on the parameter
		if( inInfo.Identifier == ASSET_INVALID || inInfo.Path.IsWhitespaceOrEmpty() ) // TODO: Better validation?
		{
			Console::WriteLine( "[Warning] AssetManager: Failed to register asset, invalid ID/Path" );
			return false;
		}

		if( Exists( inInfo.Identifier ) )
		{
			Console::WriteLine( "[Warning] AssetManager: Attempt to register asset with existing identifier! (", inInfo.Identifier, ") [", inInfo.Path, "]" );
			return false;
		}

		// Add metadata to the cache
		{
			m_Cache.emplace( inInfo.Identifier, inInfo );
		}
		return true;
	}


	bool AssetManager::GetAssetInfo( uint32 inIdentifier, AssetInstanceInfo& outInfo )
	{
		if( inIdentifier == ASSET_INVALID ) { return false; }

		// Lock the mutex and find the entry
		{
			std::lock_guard< std::mutex > cacheLock( m_CacheMutex );

			auto entry = m_Cache.find( inIdentifier );
			if( entry == m_Cache.end() || entry->second.Identifier == ASSET_INVALID )
			{
				return false;
			}

			outInfo = entry->second;
		}
		return true;
	}


	bool AssetManager::GetAssetInfo( const String& inPath, AssetInstanceInfo& outInfo )
	{
		// TODO: Better validation of path
		if( inPath.IsWhitespaceOrEmpty() ) { return false; }

		return GetAssetInfo( CalculateIdentifier( inPath ), outInfo );
	}


	uint32 AssetManager::CalculateIdentifier( const String& inPath )
	{
		if( inPath.IsWhitespaceOrEmpty() ) { return ASSET_INVALID; }

		// Get string data as UTF-8
		std::vector< byte > rawPathData;
		inPath.CopyData( rawPathData, StringEncoding::UTF8 );

		// Calculate hash using ELF
		return Crypto::ELFHash( rawPathData );
	}


	bool AssetManager::Exists( uint32 inIdentifier )
	{
		if( inIdentifier == ASSET_INVALID ) { return false; }

		// Lock mutex
		std::lock_guard< std::mutex > cacheLock( m_CacheMutex );
		return m_Cache.find( inIdentifier ) != m_Cache.end();
	}


	bool AssetManager::Exists( const String& inPath )
	{
		return Exists( CalculateIdentifier( inPath ) );
	}


	bool AssetManager::IsCached( uint32 inIdentifier )
	{
		if( inIdentifier == ASSET_INVALID ) { return false; }

		// Lock mutex
		std::lock_guard< std::mutex > cacheLock( m_CacheMutex );
		auto entry = m_Cache.find( inIdentifier );

		if( entry == m_Cache.end() ) { return false; }
		return !entry->second.Instance.expired();
	}


	bool AssetManager::IsCached( const String& inPath )
	{
		return IsCached( CalculateIdentifier( inPath ) );
	}


	// AssetTypeInfo Constructor
	// Placed here to avoid circular dependance
	AssetType::AssetType( uint32 inIdentifier, const String& inName, const String& inExt,
							  std::function< std::shared_ptr< AssetBase >( std::unique_ptr< File >&, const String&, uint32, uint64, uint64 ) > inLoadFunc )
	{
		// Validate arguments
		if( inIdentifier == ASSET_TYPE_INVALID || inName.IsWhitespaceOrEmpty() || inExt.IsWhitespaceOrEmpty() )
		{
			Console::WriteLine( "[Warning] AssetManager: Failed to register asset type (", inName, ") because the parameters were invalid" );
		}
		else if( !inLoadFunc )
		{
			Console::WriteLine( "[Warning] AssetManager: Failed to register asset type (", inName, ") because the loader function was invalid" );
		}
		else
		{
			AssetTypeInfo newInfo;
			newInfo.Identifier	= inIdentifier;
			newInfo.Name		= inName;
			newInfo.Extension	= inExt;
			newInfo.LoaderFunc	= inLoadFunc;

			if( AssetManager::RegisterType( newInfo ) )
			{
				#ifdef HYPERION_DEBUG_ASSETS
				Console::WriteLine( "[Status] AssetManager: Registered new asset type! \"", newInfo.Name, "\" with an identifier of ", newInfo.Identifier );
				#endif
			}
			else
			{
				Console::WriteLine( "[Warning] AssetManager: Failed to register asset type \"", newInfo.Name, "\"" );
			}
		}
	}

}