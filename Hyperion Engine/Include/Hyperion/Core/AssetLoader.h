/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/AssetLoader.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Asset.h"
#include "Hyperion/Core/File.h"

#include <vector>
#include <map>
#include <functional>

namespace Hyperion
{

	struct AssetInfo
	{
		std::streamoff DataBegin;
		std::streamoff DataEnd;
		String AssetType;
	};

	enum class AssetCacheType
	{
		Static,
		Dynamic,
		Stream,
		Grouped
	};

	struct AssetGroup
	{
		AssetCacheType CacheMode;
		std::map< String, AssetInfo > Assets;

		FilePath ChunkFile;
		std::streamoff GroupBegin;
		std::streamoff GroupEnd;
	};

	class AssetChunkManager
	{

	private:

		static std::unordered_map< String, AssetGroup > m_Groups;

	public:

		enum CacheResult
		{
			Success,
			NotFound,
			Error,
			AlreadyCached
		};

	};

	class AssetLoader
	{

	private:

		static std::map< String, std::function< std::shared_ptr< Asset >( std::vector< byte >::const_iterator, std::vector< byte >::const_iterator ) > > m_LoadFuncMap;

	public:

		AssetLoader() = delete;
		
		// Use template-specialization to define custom loaders for your asset type
		// You also need to use AssetLoader::RegisterAssetName to register the name, so the asset can be loaded from files
		template< typename _Ty >
		static std::shared_ptr< Asset > Load( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
		{
			// If we dont have a registered loader.. we need to create a 'GenericAsset'
			return std::make_shared< GenericAsset >( Begin, End );
		}
		
		
		template< typename _Ty >
		static void RegisterAssetType( const String& Identifier )
		{
			HYPERION_VERIFY_BASICSTR( Identifier );
			m_LoadFuncMap[ Identifier ] = std::bind( &AssetLoader::Load< _Ty >, std::placeholders::_1, std::placeholders::_2 );
		}
		
		
		static std::shared_ptr< Asset > LoadFromIdentifier( const String& Identifier, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
		{
			HYPERION_VERIFY_BASICSTR( Identifier );

			// Check if we have a registered load function
			auto Entry = m_LoadFuncMap.find( Identifier );
			if( Entry == m_LoadFuncMap.end() || !Entry->second )
			{
				// No loader found.. so lets just create a generic asset
				return Load< void >( Begin, End );
			}

			return Entry->second( Begin, End );
		}


		
		
	};
}