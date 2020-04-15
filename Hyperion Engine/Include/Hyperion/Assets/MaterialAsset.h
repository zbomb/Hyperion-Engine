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
		class MaterialAssetLoader
		* A class used to load material assets from file
	*/
	class MaterialAssetLoader : public IAssetLoader
	{

	public:

		static std::shared_ptr< MaterialAsset > Load( uint32 inIdentifier, std::unique_ptr< IFile >& inFile );
		static bool IsValidFile( const FilePath& inPath );

	};


	/*
		class MaterialAssetCache
		* A class used to store and get material assets
	*/
	class MaterialAssetCache : public IAssetCache
	{

	private:

		static std::map< uint32, std::weak_ptr< MaterialAsset > > m_Cache;
		static std::mutex m_CacheMutex;

	public:

		static std::weak_ptr< MaterialAsset > Get( uint32 inIdentifier );
		static void Store( uint32 inIdentifier, const std::shared_ptr< MaterialAsset >& inPtr );

	};


	/*
		class MaterialAsset
		* An engine asset containing a list of material parameters and textures
	*/
	class MaterialAsset : public AssetBase
	{

	public:

		using _LoaderType = MaterialAssetLoader;
		//using _CacheType = MaterialAssetCache;

		static inline AssetCacheMethod GetCacheMethod() { return AssetCacheMethod::Full; }

	protected:

		std::map< String, std::any > m_Values;
		std::map< String, std::shared_ptr< TextureAsset > > m_Textures;

		MaterialAsset( std::map< String, std::any >&& inData, const FilePath& inPath, uint32 inIdentifier );

	public:

		~MaterialAsset();

		MaterialAsset() = delete;
		MaterialAsset( const MaterialAsset& ) = delete;
		MaterialAsset( MaterialAsset&& ) = delete;
		MaterialAsset& operator=( const MaterialAsset& ) = delete;
		MaterialAsset& operator=( MaterialAsset&& ) = delete;


		bool GetBool( const String& inKey, bool inDefault = false );
		int32 GetInt( const String& inKey, int32 inDefault = 0 );
		uint32 GetUInt( const String& inKey, uint32 inDefault = 0 );
		float GetFloat( const String& inKey, float inDefault = 0.f );
		String GetString( const String& inKey, const String& inDefault = "" );

		std::shared_ptr< TextureAsset > GetTexture( const String& inKey );
		std::any GetValue( const String& inKey );

		inline auto ValuesBegin() const { return m_Values.begin(); }
		inline auto ValuesEnd() const { return m_Values.end(); }

		inline auto TexturesBegin() const { return m_Textures.begin(); }
		inline auto TexturesEnd() const { return m_Textures.end(); }

		friend class MaterialAssetLoader;

	};


}