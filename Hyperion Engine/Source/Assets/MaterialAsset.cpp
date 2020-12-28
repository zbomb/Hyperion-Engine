/*==================================================================================================
	Hyperion Engine
	Source/Assets/MaterialAsset.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Assets/MaterialAsset.h"
#include "Hyperion/Tools/HMATReader.h"
#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Core/AssetManager.h"

using namespace std::placeholders;


namespace Hyperion
{

	/*
		Register Asset Type
	*/
	AssetType g_Asset_Type_Material = AssetType(
		ASSET_TYPE_MATERIAL, "material", ".hmat",
		std::bind( &MaterialAsset::LoadFromFile, _1, _2, _3, _4, _5 )
	);


	/*----------------------------------------------------------------------------------------------------------
		MaterialAsset Class
	------------------------------------------------------------------------------------------------------------*/
	MaterialAsset::MaterialAsset( std::map< String, std::any >&& inData, const String& inPath, uint32 inIdentifier, uint64 inOffset, uint64 inLength )
		: AssetBase( inIdentifier, inPath, inOffset, inLength ), m_Values( std::move( inData ) )
	{
		// We want to find all of the textures, and get them from the asset manager, so they get cached in as well
		for( auto It = m_Values.begin(); It != m_Values.end(); )
		{
			if( It->second.type() == typeid( TextureReference ) )
			{
				// Get texture identifier and get it from the asset manager
				uint32 assetId = std::any_cast< TextureReference >( It->second ).Identifier;
				auto inst = AssetManager::Get< TextureAsset >( assetId );

				if( !inst )
				{
					Console::WriteLine( "[WARNING] MaterialAsset: Texture asset (", assetId, ") couldnt be found for material \"", GetPath(), "\"" );
				}
				else
				{
					m_Textures.emplace( It->first, inst );
				}
				
				It = m_Values.erase( It );
			}
			else
			{
				It++;
			}
		}
	}


	MaterialAsset::~MaterialAsset()
	{
		m_Values.clear();
	}


	std::any MaterialAsset::GetValue( const String& inKey )
	{
		auto entry = m_Values.find( inKey.ToLower() );
		if( entry != m_Values.end() )
		{
			return entry->second;
		}

		return std::any();
	}


	std::shared_ptr< TextureAsset > MaterialAsset::GetTexture( const String& inKey )
	{
		auto entry = m_Textures.find( inKey.ToLower() );
		if( entry != m_Textures.end() )
		{
			return entry->second;
		}

		return nullptr;
	}


	bool MaterialAsset::GetBool( const String& inKey, bool inDefault /* = false */ )
	{
		auto val = GetValue( inKey );
		bool* casted = std::any_cast< bool >( &val );
		return casted ? *casted : inDefault;
	}


	int32 MaterialAsset::GetInt( const String& inKey, int32 inDefault /* = 0 */ )
	{
		auto val = GetValue( inKey );
		int32* casted = std::any_cast< int32 >( &val );
		return casted ? *casted : inDefault;
	}


	uint32 MaterialAsset::GetUInt( const String& inKey, uint32 inDefault /* = 0 */ )
	{
		auto val = GetValue( inKey );
		uint32* casted = std::any_cast< uint32 >( &val );
		return casted ? *casted : inDefault;
	}


	float MaterialAsset::GetFloat( const String& inKey, float inDefault /* = 0.f */ )
	{
		auto val = GetValue( inKey );
		float* casted = std::any_cast< float >( &val );
		return casted ? *casted : inDefault;
	}

	String MaterialAsset::GetString( const String& inKey, const String& inDefault /* = "" */ )
	{
		auto val = GetValue( inKey );
		String* casted = std::any_cast< String >( &val );
		return casted ? *casted : inDefault;
	}


	/*----------------------------------------------------------------------------------------------------------
		Material Asset Loader Function
	------------------------------------------------------------------------------------------------------------*/
	std::shared_ptr< AssetBase > MaterialAsset::LoadFromFile( std::unique_ptr< File >& inFile, const String& inPath, uint32 inIdentifier, uint64 inOffset, uint64 inLength )
	{
		DataReader reader( *inFile );
		return LoadFromReader( reader, inPath, inIdentifier, inOffset, inLength );
	}

	std::shared_ptr< AssetBase > MaterialAsset::LoadFromReader( DataReader& inReader, const String& inPath, uint32 inIdentifier, uint64 inOffset, uint64 inLength )
	{
		if( inIdentifier == ASSET_INVALID )
		{
			Console::WriteLine( "[Warning] MaterialAsset: Failed to load from file.. file/id was invalid (", inPath, ")" );
			return nullptr;
		}

		{
			if( inReader.Size() == 0 || inOffset > (uint64)inReader.Size() || inOffset + inLength > (uint64)inReader.Size() )
			{
				Console::WriteLine( "[Warning] MaterialAsset: Failed to load from file (", inPath, ").. not enough data or range is out of bounds!" );
				return nullptr;
			}

			HMATReader reader( inReader, inOffset, inLength );
			reader.Begin();

			std::map< String, std::any > materialData;
			while( reader.Next() )
			{
				String key;
				std::any val;

				// Read next entry, check if valid
				auto res = reader.ReadEntry( key, val );
				if( res != HMATReader::Result::Success )
				{
					Console::WriteLine( "[ERROR] MaterialAssetLoader: Invalid material asset '", inPath, "' (Error Code: ", (uint32) res, ")" );
					return nullptr;
				}

				materialData.emplace( key, val );
			}

			// Create new material asset from the map we loaded
			return std::shared_ptr< MaterialAsset >( new MaterialAsset( std::move( materialData ), inPath, inIdentifier, inOffset, inLength ) );
			
		}
	}
}
