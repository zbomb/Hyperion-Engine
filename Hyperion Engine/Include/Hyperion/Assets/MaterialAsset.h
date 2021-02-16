/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Assets/MaterialAsset.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Asset.h"
#include "Hyperion/File/IFile.h"
#include <any>


namespace Hyperion
{
	/*
		Forward Declaration
	*/
	class MaterialAsset;
	class TextureAsset;


	/*
		class MaterialAsset
		* An engine asset containing a list of material parameters and textures
	*/
	class MaterialAsset : public AssetBase
	{

	protected:

		std::map< std::string, std::any > m_Values;
		std::map< std::string, std::shared_ptr< TextureAsset > > m_Textures;

		MaterialAsset( std::map< std::string, std::any >&& inData, const String& inPath, uint32 inIdentifier, uint64 inOffset, uint64 inLength );

	public:

		~MaterialAsset();

		MaterialAsset() = delete;
		MaterialAsset( const MaterialAsset& ) = delete;
		MaterialAsset( MaterialAsset&& ) = delete;
		MaterialAsset& operator=( const MaterialAsset& ) = delete;
		MaterialAsset& operator=( MaterialAsset&& ) = delete;


		bool GetBool( const std::string& inKey, bool inDefault = false );
		int32 GetInt( const std::string& inKey, int32 inDefault = 0 );
		uint32 GetUInt( const std::string& inKey, uint32 inDefault = 0 );
		float GetFloat( const std::string& inKey, float inDefault = 0.f );
		std::string GetString( const std::string& inKey, const std::string& inDefault = "" );

		std::shared_ptr< TextureAsset > GetTexture( const std::string& inKey );
		std::any GetValue( const std::string &inKey );

		inline auto ValuesBegin() const { return m_Values.begin(); }
		inline auto ValuesEnd() const { return m_Values.end(); }

		inline auto TexturesBegin() const { return m_Textures.begin(); }
		inline auto TexturesEnd() const { return m_Textures.end(); }

		
		// Loader Function
		static std::shared_ptr< AssetBase > LoadFromFile( std::unique_ptr< File >& inFile, const String& inPath, uint32 inIdentifier, uint64 inOffset, uint64 inLength );
		static std::shared_ptr< AssetBase > LoadFromReader( DataReader& inReader, const String& inPath, uint32 inIdentifier, uint64 inOffset, uint64 inLength );

	};


}