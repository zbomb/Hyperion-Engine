/*==================================================================================================
	Hyperion Engine
	Source/Assets/TextureAsset.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Core/RenderManager.h"
#include "Hyperion/Tools/ImageLoader.h"
#include "Hyperion/Renderer/Types/ITexture.h"



namespace Hyperion
{
	// Register our asset type
	static AssetRegistryEntry< TextureAsset > textureAssetEntry( { "png", "dds", "htex" } );

	uint64 TextureAsset::m_NextIdentifier( 1 );


	TextureAsset::TextureAsset()
		: m_Identifier( m_NextIdentifier++ )
	{
	}

	TextureAsset::~TextureAsset()
	{

	}

	String TextureAsset::GetAssetType() const
	{
		return "Texture";
	}

	uint32 TextureAsset::GetWidth() const
	{
		return 0;
	}

	uint32 TextureAsset::GetHeight() const
	{
		return 0;
	}

	bool TextureAsset::IsValidTexture() const
	{
		// Check for valid format
		if( m_Header.Format == TextureAssetFormat::NONE )
		{
			return false;
		}

		// Check for valid root LOD level
		if( m_Header.LODs.size() == 0 )
		{
			return false;
		}

		// Validate resolution & size values
		auto& rootLOD = m_Header.LODs.at( 0 );

		if( rootLOD.Width == 0 || rootLOD.Height == 0 || rootLOD.Size == 0 )
		{
			return false;
		}

		// Seems valid 'enough'
		// In the future, we might decide to do further validation, but this is good enough for most situations
		return true;
	}


	template<>
	std::shared_ptr< Asset > AssetLoader::Load< TextureAsset >( const AssetPath& Identifier, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
	{
		return std::make_shared< TextureAsset >( Identifier, nullptr );
	}

	template<>
	std::shared_ptr< Asset > AssetLoader::Stream< TextureAsset >( const AssetPath& Identifier, AssetStream& Stream )
	{
		return std::make_shared< TextureAsset >( Identifier, nullptr );
	}

}