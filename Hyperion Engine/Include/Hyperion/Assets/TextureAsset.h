/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Assets/TextureAsset.h
	� 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Asset.h"
#include "Hyperion/Core/AssetLoader.h"



namespace Hyperion
{
	/*
		Formats that we actually use internally to store texture assets loaded from disk
		There are a lot of possible options, but were limiting it to a few
	*/
	enum class TextureAssetFormat
	{
		NONE						= 0,
		RGBA_UNCOMPRESSED_32BIT		= 1, // RGBA_8BIT_UNORM, 32-bits per pixel, highest quality
		RGB_UNCOMPRESSED_24BIT		= 2, // RGB_8BIT_UNORM, 24 bits per pixel, highest quality w/o alpha
		GS_UNCOMPRESSED_8BIT		= 3, // R_8BIT_UNORM, 8-bits per pixel, highest quality w/o color or alpha
		GSA_UNCOMPRESSED_16BIT		= 4, // RG_8BIT_UNORM, 16-bits per pixel, highest quality w/o color
		RGB_COMPRESSED_DXT1			= 5, // RGB_DXT_1, 4-bits per pixel, medium quality, w/o alpha
		RGB_COMPRESSED_BC7			= 6, // RGB_BC_7, 8-bits per pixel, higher quality, w/o alpha
		RGBA_COMPRESSED_BC7			= 7, // RGBA_BC_7, 8-bits per pixel, higher quality
		RGBA_COMPRESSED_DXT5		= 8 // RGBA_DXT_5, 8-bits per pixel, medium quality
	};

	struct TextureLODInfo
	{
		uint16 Width;
		uint16 Height;
		uint32 Size;
	};

	struct TextureAssetHeader
	{
		// uint64 ValidationBytes; [In File Only! We dont store it in memory]

		TextureAssetFormat Format; // Stored as uint8 in file

		uint8 LinePadding;
		uint8 LODPadding;
		
		// uint8 Reserved [In File Only!]

		std::vector< TextureLODInfo > LODs;

		TextureAssetHeader()
			: Format( TextureAssetFormat::NONE ), LinePadding( 0 ), LODPadding( 0 )
		{}
	};


	class TextureAsset : public Asset
	{

	private:

		static uint64 m_NextIdentifier;
		uint64 m_Identifier;

		TextureAssetHeader m_Header;

	public:

		TextureAsset();
		~TextureAsset();

		virtual String GetAssetType() const override;
		uint32 GetWidth() const;
		uint32 GetHeight() const;

		inline uint64 GetIdentifier() const { return m_Identifier; }
		inline const TextureAssetHeader& GetHeader() const { return m_Header; }

		bool IsValidTexture() const;
	};


	template<>
	std::shared_ptr< Asset > AssetLoader::Load< TextureAsset >( const AssetPath& Identifier, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End );

	template<>
	std::shared_ptr< Asset > AssetLoader::Stream< TextureAsset >( const AssetPath& Identifier, AssetStream& Stream );

}
