/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/AssetLoader.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Asset.h"
#include "Hyperion/Core/File.h"
#include "Hyperion/Core/VirtualFileSystem.h"

#include <vector>
#include <map>
#include <functional>

namespace Hyperion
{

	class AssetLoader
	{

	private:

		static std::map< String, 
			std::pair< 
				std::function< std::shared_ptr< Asset >( std::vector< byte >::const_iterator, std::vector< byte >::const_iterator ) >,
				std::function< std::shared_ptr< Asset >( AssetStream& ) > 
			>
		> m_LoadFuncMap;

	public:

		AssetLoader() = delete;
		
		// Use template-specialization to define custom loaders for your asset type
		// You also need to use AssetLoader::RegisterAssetName to register the name, so the asset can be loaded from files
		template< typename _Ty >
		inline static std::shared_ptr< Asset > Load( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
		{
			// If we dont have a registered loader.. we need to create a 'GenericAsset'
			return std::make_shared< GenericAsset >( Begin, End );
		}

		template< typename _Ty >
		inline static std::shared_ptr< Asset > Stream( AssetStream& inStream )
		{
			// If we dont have a specialized stream function for this asset type, were going to cache all the
			// data available, and use the normal load function with iterators
			std::vector< byte > data;
			auto result = inStream.ReadBytes( data, (size_t) inStream.GetSize() );
			if( result != AssetStream::ReadResult::Fail )
			{
				return Load< _Ty >( data.begin(), data.end() );
			}
			else
			{
				std::cout << "[ERROR] AssetLoader: Failed to stream in asset! Couldnt read the data from the stream!\n";
				return nullptr;
			}
		}
		
		
		template< typename _Ty >
		static void RegisterAssetType( const String& Identifier )
		{
			HYPERION_VERIFY_BASICSTR( Identifier );
			m_LoadFuncMap[ Identifier ] = std::pair( 
				std::bind( &AssetLoader::Load< _Ty >, std::placeholders::_1, std::placeholders::_2 ),
				std::bind( &AssetLoader::Stream< _Ty >, std::placeholders::_1 )
			);
		}
		
		
		static std::shared_ptr< Asset > LoadFromIdentifier( const String& Identifier, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
		{
			HYPERION_VERIFY_BASICSTR( Identifier );

			// Check if we have a registered load function
			auto Entry = m_LoadFuncMap.find( Identifier );
			if( Entry == m_LoadFuncMap.end() || !Entry->second.first )
			{
				// No loader found.. so lets just create a generic asset
				return Load< void >( Begin, End );
			}

			return Entry->second.first( Begin, End );
		}

		static std::shared_ptr< Asset > StreamFromIdentifier( const String& Identifier, AssetStream& inStream )
		{
			HYPERION_VERIFY_BASICSTR( Identifier );

			// Check if this asset type is registerd
			auto Entry = m_LoadFuncMap.find( Identifier );
			if( Entry == m_LoadFuncMap.end() || !Entry->second.second )
			{
				// No stream function found.. so were going to cache the data and use a loader function
				std::vector< byte > data;
				auto result = inStream.ReadBytes( data, (size_t) inStream.GetSize() );

				if( result != AssetStream::ReadResult::Fail )
				{
					// We didnt fail, so if theres a loader function for this type, use that
					// otherwise, return a generic asset with this data inside of it
					if( Entry != m_LoadFuncMap.end() && Entry->second.first )
					{
						return Entry->second.first( data.begin(), data.end() );
					}
					else
					{
						return Load< void >( data.begin(), data.end() );
					}
				}
				else
				{
					// Failed to read from the AssetStream.. so lets return null
					return nullptr;
				}
			}
			else
			{
				// We found the stream function
				return Entry->second.second( inStream );
			}
		}
	};


}