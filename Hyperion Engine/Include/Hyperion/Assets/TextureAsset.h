/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Assets/TextureAsset.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Asset.h"



namespace Hyperion
{

	// Maybe instead of having a bunch of texture pixel format enums, maybe we should just make
	// a single one or two, otherwise conversions will be quite annoying. We can still limit the ones
	// that are able to be used with certain classes or functions

	/*
		Forward Declaration
	*/
	class TextureAsset;
	class IFile;


	/*
		struct TextureLOD
		* A data structure describing a texture mip (LOD) level
	*/
	struct TextureLOD
	{
		uint16 Width;
		uint16 Height;
		uint32 FileOffset;
		uint32 LODSize;
		uint32 RowSize;
	};


	/*
		struct TextureHeader
		* A data structure describing a texture and its mip/LOD levels
		* In the future, we will most likely add more data members to this structure
	*/
	struct TextureHeader
	{
		TextureFormat Format;
		uint8 LODPadding;
		std::vector< TextureLOD > LODs;
	};


	/*
		class TextureAssetCache
		* Class that handles the caching of texture assets
	*/
	class TextureAssetCache : public IAssetCache
	{

	private:

		static std::map< uint32, std::weak_ptr< TextureAsset > > m_Cache;
		static std::mutex m_CacheMutex;

	public:

		static std::weak_ptr< TextureAsset > Get( uint32 inIdentifier );
		static void Store( uint32 inIdentifier, const std::shared_ptr< TextureAsset >& inPtr );

	};


	/*
		class TextureAssetLoader
		* Class that handles the loading of texture assets from data sources
	*/
	class TextureAssetLoader : public IAssetLoader
	{

	public:

		static std::shared_ptr< TextureAsset > Load( uint32 inIdentifier, std::unique_ptr< IFile >& inFile );
		static bool IsValidFile( const FilePath& inPath );

	};


	/*
		class TextureAsset
		* An engine asset containing a texture
	*/
	class TextureAsset : public AssetBase
	{

	public:

		// Things needed for the asset manager to handle this type properly
		using _CacheType	= TextureAssetCache;
		using _LoaderType	= TextureAssetLoader;

		static inline AssetCacheMethod GetCacheMethod() { return AssetCacheMethod::Full; }

	protected:

		// Data Members
		TextureHeader m_Header;

		TextureAsset( const TextureHeader& inHeader, const FilePath& inPath, uint32 inIdentifier );

	public:

		// Delted Functions
		TextureAsset() = delete;
		TextureAsset( const TextureAsset& ) = delete;
		TextureAsset( TextureAsset&& ) = delete;
		TextureAsset& operator=( const TextureAsset& ) = delete;
		TextureAsset& operator=( TextureAsset&& ) = delete;

		// Member Functions
		inline const TextureHeader& GetHeader() const { return m_Header; }

		uint32 GetWidth() const;
		uint32 GetHeight() const;
		uint8 GetLODCount() const;

		friend class TextureAssetLoader;
	};


}



/*
namespace Hyperion
{
	/*
		Formats that we actually use internally to store texture assets loaded from disk
		There are a lot of possible options, but were limiting it to a few
	
	enum class TextureAssetFormat
	{
		NONE						= 0,
		RGBA_UNCOMPRESSED_32BIT		= 1, // RGBA_8BIT_UNORM, 32-bits per pixel, highest quality
		GS_UNCOMPRESSED_8BIT		= 3, // R_8BIT_UNORM, 8-bits per pixel, highest quality w/o color or alpha
		GSA_UNCOMPRESSED_16BIT		= 4, // RG_8BIT_UNORM, 16-bits per pixel, highest quality w/o color
		RGB_COMPRESSED_DXT1			= 5, // RGB_DXT_1, 4-bits per pixel, medium quality, w/o alpha
		RGB_COMPRESSED_BC7			= 6, // RGB_BC_7, 8-bits per pixel, higher quality, w/o alpha
		RGBA_COMPRESSED_BC7			= 7, // RGBA_BC_7, 8-bits per pixel, higher quality
		RGBA_COMPRESSED_DXT5		= 8 // RGBA_DXT_5, 8-bits per pixel, medium quality
	};

	static TextureFormat ConvertAssetToTextureFormat( TextureAssetFormat inFmt )
	{
		switch( inFmt )
		{
		case TextureAssetFormat::RGBA_UNCOMPRESSED_32BIT:
			return TextureFormat::RGBA_8BIT_UNORM;
		case TextureAssetFormat::GS_UNCOMPRESSED_8BIT:
			return TextureFormat::R_8BIT_UNORM;
		case TextureAssetFormat::GSA_UNCOMPRESSED_16BIT:
			return TextureFormat::RG_8BIT_UNORM;
		case TextureAssetFormat::RGB_COMPRESSED_DXT1:
			return TextureFormat::RGB_DXT_1;
		case TextureAssetFormat::RGB_COMPRESSED_BC7:
			return TextureFormat::RGB_BC_7;
		case TextureAssetFormat::RGBA_COMPRESSED_BC7:
			return TextureFormat::RGBA_BC_7;
		case TextureAssetFormat::RGBA_COMPRESSED_DXT5:
			return TextureFormat::RGBA_DXT_5;
		case TextureAssetFormat::NONE:
		default:
			return TextureFormat::NONE;
		}
	}

	static uint32 GetTextureAssetHeaderSize( uint8 inLODCount )
	{
		return 8 + 1 + 1 + 1 + ( 12 * inLODCount );
	}

	struct TextureLODInfo
	{
		uint16 Width;
		uint16 Height;
		uint32 Offset;
		uint32 Size;
		uint32 RowSize;
	};

	struct TextureAssetHeader
	{
		// uint64 ValidationBytes; [In File Only! We dont store it in memory]

		TextureAssetFormat Format; // Stored as uint8 in file

		uint8 LODPadding;
		
		// uint8 Reserved [In File Only!]

		std::vector< TextureLODInfo > LODs;

		TextureAssetHeader()
			: Format( TextureAssetFormat::NONE ), LODPadding( 0 )
		{}
	};


	class TextureAsset : public Asset
	{

	private:

		static uint32 m_NextIdentifier;
		uint32 m_Identifier;

		TextureAssetHeader m_Header;

	public:

		TextureAsset();
		~TextureAsset();

		virtual String GetAssetType() const override;
		uint32 GetWidth() const;
		uint32 GetHeight() const;

		inline uint32 GetIdentifier() const { return m_Identifier; }
		inline const TextureAssetHeader& GetHeader() const { return m_Header; }

		bool IsValidTexture() const;
	};


	template<>
	std::shared_ptr< Asset > AssetLoader::Load< TextureAsset >( const AssetPath& Identifier, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End );

	template<>
	std::shared_ptr< Asset > AssetLoader::Stream< TextureAsset >( const AssetPath& Identifier, AssetStream& Stream );

}
*/