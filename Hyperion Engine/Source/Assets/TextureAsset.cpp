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


	TextureAsset::TextureAsset( const AssetPath& inPath )
		: m_Path( inPath ), m_Identifier( m_NextIdentifier++ )
	{
	}

	TextureAsset::~TextureAsset()
	{

	}

	String TextureAsset::GetAssetType() const
	{
		return "Texture";
	}

	AssetPath TextureAsset::GetAssetPath() const
	{
		return m_Path;
	}

	uint32 TextureAsset::GetWidth() const
	{
		return 0;
	}

	uint32 TextureAsset::GetHeight() const
	{
		return 0;
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