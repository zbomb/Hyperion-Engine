/*==================================================================================================
	Hyperion Engine
	Source/Assets/MaterialAsset.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Assets/MaterialAsset.h"
#include "Hyperion/Tools/HMATReader.h"
#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Core/AssetManager.h"


namespace Hyperion
{
	/*
		Static Definitions
	*/
	std::map< uint32, std::weak_ptr< MaterialAsset > > MaterialAssetCache::m_Cache;
	std::mutex MaterialAssetCache::m_CacheMutex;

	/*----------------------------------------------------------------------------------------------------------
		MaterialAsset Class
	------------------------------------------------------------------------------------------------------------*/
	MaterialAsset::MaterialAsset( std::map< String, std::any >&& inData, const FilePath& inPath, uint32 inIdentifier )
		: AssetBase( inPath, inIdentifier ), m_Values( std::move( inData ) )
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
					Console::WriteLine( "[WARNING] MaterialAsset: Texture asset (", assetId, ") couldnt be found for material \"", GetPath().ToString(), "\"" );
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
		MaterialAssetLoader Class
	------------------------------------------------------------------------------------------------------------*/
	bool MaterialAssetLoader::IsValidFile( const FilePath& inPath )
	{
		if( inPath.HasExtension() )
		{
			auto ext = inPath.Extension().ToLower().TrimBoth();
			return ext == ".hmat";
		}

		return false;
	}


	std::shared_ptr< MaterialAsset > MaterialAssetLoader::Load( uint32 inIdentifier, std::unique_ptr< IFile >& inFile )
	{
		if( inIdentifier == ASSET_INVALID || !inFile || !inFile->IsValid() ) 
		{ 
			Console::WriteLine( "[ERROR] MaterialAssetLoader: Failed to load material asset.. identifier and/or file was invalid" );
			return nullptr; 
		}

		DataReader reader( *inFile );
		return Load( inIdentifier, reader, inFile->GetPath() );
	}

	std::shared_ptr< MaterialAsset > MaterialAssetLoader::Load( uint32 inIdentifier, DataReader& inReader, const FilePath& inPath )
	{
		if( inIdentifier == ASSET_INVALID || inReader.Size() == 0 )
		{
			Console::WriteLine( "[ERROR] MaterialAssetLoader: Failed to load material asset.. identifier and/or datareader was invalid" );
			return nullptr;
		}

		// Use reader to process file into a map
		HMATReader reader( inReader );
		reader.Begin();

		std::map< String, std::any > matData;

		while( reader.Next() )
		{
			String key;
			std::any val;

			// Read next entry, check if valid
			auto res = reader.ReadEntry( key, val );
			if( res != HMATReader::Result::Success )
			{
				Console::WriteLine( "[ERROR] MaterialAssetLoader: Invalid material asset '", inPath.ToString(), "' (Error Code: ", (uint32) res, ")" );
				return nullptr;
			}

			matData.emplace( key, val );
		}

		// Create new material asset from the map we loaded
		return std::shared_ptr< MaterialAsset >( new MaterialAsset( std::move( matData ), inPath, inIdentifier ) );
	}


	/*----------------------------------------------------------------------------------------------------------
		MaterialAssetCache Class
	------------------------------------------------------------------------------------------------------------*/

	std::weak_ptr< MaterialAsset > MaterialAssetCache::Get( uint32 inIdentifier )
	{
		std::lock_guard< std::mutex > lock( m_CacheMutex );

		auto entry = m_Cache.find( inIdentifier );
		if( entry == m_Cache.end() )
		{
			return std::weak_ptr< MaterialAsset >();
		}

		if( entry->second.expired() )
		{
			m_Cache.erase( entry );
			return std::weak_ptr< MaterialAsset >();
		}
		else
		{
			return entry->second;
		}
	}

	void MaterialAssetCache::Store( uint32 inIdentifier, const std::shared_ptr< MaterialAsset >& inPtr )
	{
		std::lock_guard< std::mutex > lock( m_CacheMutex );

		m_Cache.emplace( inIdentifier, std::weak_ptr< MaterialAsset >( inPtr ) );
	}
}
