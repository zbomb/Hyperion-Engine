/*==================================================================================================
	Hyperion Engine
	Source/Assets/TextureAsset.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Tools/HTXReader.h"



namespace Hyperion
{
	std::map< uint32, std::weak_ptr< TextureAsset > > TextureAssetCache::m_Cache;
	std::mutex TextureAssetCache::m_CacheMutex;


	/*----------------------------------------------------------------------------------------------------------
		TextureAssetCache Class
	------------------------------------------------------------------------------------------------------------*/
	std::weak_ptr< TextureAsset > TextureAssetCache::Get( uint32 inIdentifier )
	{
		std::lock_guard< std::mutex > lock( m_CacheMutex );

		auto entry = m_Cache.find( inIdentifier );
		if( entry == m_Cache.end() )
		{
			return std::weak_ptr< TextureAsset >();
		}

		if( entry->second.expired() )
		{
			m_Cache.erase( entry );
			return std::weak_ptr< TextureAsset >();
		}
		else
		{
			return entry->second;
		}
	}


	void TextureAssetCache::Store( uint32 inIdentifier, const std::shared_ptr< TextureAsset >& inPtr )
	{
		std::lock_guard< std::mutex > lock( m_CacheMutex );

		m_Cache.emplace( inIdentifier, std::weak_ptr< TextureAsset >( inPtr ) );
	}


	/*----------------------------------------------------------------------------------------------------------
		TextureAssetLoader Class
	------------------------------------------------------------------------------------------------------------*/
	std::shared_ptr< TextureAsset > TextureAssetLoader::Load( uint32 inIdentifier, std::unique_ptr< IFile >& inFile )
	{
		// Use the HTX Reader class to read in the header data for this texture asset
		HTXReader Reader( *inFile );
		
		TextureHeader assetHeader;
		if( Reader.ReadHeader( assetHeader ) != HTXReader::Result::Success )
		{
			Console::WriteLine( "[WARNING] TextureAssetLoader: Failed to read header for '", inFile->GetPath().ToString(), "'" );
			return nullptr;
		}

		// Return the new texture asset created from the header we read from file
		return std::shared_ptr< TextureAsset >( new TextureAsset( assetHeader, inFile->GetPath(), inIdentifier ) );
	}


	/*----------------------------------------------------------------------------------------------------------
		TextureAsset Class
	------------------------------------------------------------------------------------------------------------*/
	TextureAsset::TextureAsset( const TextureHeader& inHeader, const FilePath& inPath, uint32 inIdentifier )
		: m_Header( inHeader ), AssetBase( inPath, inIdentifier )
	{}


	uint32 TextureAsset::GetWidth() const
	{
		return m_Header.LODs.empty() ? 0 : m_Header.LODs.front().Width;
	}


	uint32 TextureAsset::GetHeight() const
	{
		return m_Header.LODs.empty() ? 0 : m_Header.LODs.front().Height;
	}


	uint8 TextureAsset::GetLODCount() const
	{
		// Check for overflow
		auto ret			= m_Header.LODs.size();
		static auto maxNum	= (uint32)std::numeric_limits< uint8 >::max();

		if( ret > maxNum )
		{
			Console::WriteLine( "[ERROR] TextureAsset: Invalid number of LOD levels detected in '", GetPath().ToString(), "' (", ret, ")" );
			return 0;
		}

		return (uint8)m_Header.LODs.size();
	}

}