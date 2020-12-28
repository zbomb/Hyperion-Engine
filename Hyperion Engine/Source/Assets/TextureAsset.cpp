/*==================================================================================================
	Hyperion Engine
	Source/Assets/TextureAsset.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Tools/HTXReader.h"

using namespace std::placeholders;


namespace Hyperion
{
	
	/*
		Register asset type
	*/
	AssetType g_AssetType_Texture = AssetType(
		ASSET_TYPE_TEXTURE, "texture", ".htx",
		std::bind( &TextureAsset::LoadFromFile, _1, _2, _3, _4, _5 )
	);


	/*----------------------------------------------------------------------------------------------------------
		TextureAsset Class
	------------------------------------------------------------------------------------------------------------*/
	TextureAsset::TextureAsset( const TextureHeader& inHeader, const String& inPath, const FilePath& inDiskPath, uint32 inIdentifier, uint64 inOffset, uint64 inLength )
		: m_Header( inHeader ), m_DiskPath( inDiskPath ), AssetBase( inIdentifier, inPath, inOffset, inLength )
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
			Console::WriteLine( "[ERROR] TextureAsset: Invalid number of LOD levels detected in '", GetPath(), "' (", ret, ")" );
			return 0;
		}

		return (uint8)m_Header.LODs.size();
	}


	/*----------------------------------------------------------------------------------------------------------
		Texture Asset Loader Function
	------------------------------------------------------------------------------------------------------------*/
	std::shared_ptr< AssetBase > TextureAsset::LoadFromFile( std::unique_ptr< File >& inFile, const String& inPath, uint32 inIdentifier, uint64 inOffset, uint64 inLength )
	{
		if( !inFile || !inFile->IsValid() ) { return nullptr; }

		HTXReader reader( *inFile, inOffset, inLength );
		TextureHeader header;

		if( reader.ReadHeader( header ) != HTXReader::Result::Success )
		{
			Console::WriteLine( "[Warning] TextureAsset: Failed to load texture from file \"", inFile->GetPath().ToString(), "\" because the header was invalid" );
			return nullptr;
		}

		return std::shared_ptr< AssetBase >( new TextureAsset( header, inPath, inFile->GetPath(), inIdentifier, inOffset, inLength ) );
	}

}