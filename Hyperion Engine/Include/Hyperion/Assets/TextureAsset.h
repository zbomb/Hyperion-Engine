/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Assets/TextureAsset.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Asset.h"
#include "Hyperion/Core/AssetLoader.h"



namespace Hyperion
{

	class TextureAsset : public Asset
	{

	private:

		AssetPath m_Path;
		static uint64 m_NextIdentifier;
		uint64 m_Identifier;

	public:

		TextureAsset() = delete;
		TextureAsset( const AssetPath& inPath );
		~TextureAsset();

		virtual String GetAssetType() const override;
		virtual AssetPath GetAssetPath() const override;
		uint32 GetWidth() const;
		uint32 GetHeight() const;

		inline uint64 GetIdentifier() const { return m_Identifier; }
	};


	template<>
	std::shared_ptr< Asset > AssetLoader::Load< TextureAsset >( const AssetPath& Identifier, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End );

	template<>
	std::shared_ptr< Asset > AssetLoader::Stream< TextureAsset >( const AssetPath& Identifier, AssetStream& Stream );

}
