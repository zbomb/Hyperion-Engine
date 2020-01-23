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

		static std::map< size_t,
			std::pair<
			std::function< std::shared_ptr< Asset >( const AssetPath&, std::vector< byte >::const_iterator, std::vector< byte >::const_iterator ) >,
			std::function< std::shared_ptr< Asset >( const AssetPath&, AssetStream& ) >
			>
		> m_TypeToLoadMap;

		static std::map< String, size_t > m_ExtensionToTypeMap;

	
		static std::shared_ptr< Asset > DefaultStream( const AssetPath& path, AssetStream& inStream )
		{
			std::vector< byte > Data;
			auto Size = inStream.GetSize();

			inStream.SeekBegin();
			if( inStream.ReadBytes( Data, Size ) != AssetStream::Success )
			{
				Console::WriteLine( "[WARNING] AssetLoader: Failed to stream or load asset '", path.GetPath(), "', returning null..." );
				return nullptr;
			}

			return LoadFromFileName( path, Data.begin(), Data.end() );
		}

	public:

		AssetLoader() = delete;
		
		// Use template-specialization to define custom loaders for your asset type
		// You also need to use AssetLoader::RegisterAssetName to register the name, so the asset can be loaded from files
		template< typename _Ty >
		inline static std::shared_ptr< Asset > Load( const AssetPath& Identifier, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
		{
			// If we dont have a registered loader.. we need to create a 'GenericAsset'
			return std::make_shared< GenericAsset >( Begin, End );
		}

		template< typename _Ty >
		inline static std::shared_ptr< Asset > Stream( const AssetPath& Identifier, AssetStream& inStream )
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
				Console::WriteLine( "[ERROR] AssetLoader: Failed to stream in asset! Couldnt read the data from the stream!" );
				return nullptr;
			}
		}
		

		template< typename _Ty >
		static void RegisterAssetType()
		{
			m_TypeToLoadMap[ typeid( _Ty ).hash_code() ] = std::pair(
				std::bind( &AssetLoader::Load< _Ty >, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ),
				std::bind( &AssetLoader::Stream< _Ty >, std::placeholders::_1, std::placeholders::_2 ) );
		}

		template< typename _Ty >
		static void RegisterAssetFileExtension( const String& inExt )
		{
			HYPERION_VERIFY_BASICSTR( inExt );
			auto ext = inExt.TrimBoth().ToLower();

			if( m_ExtensionToTypeMap.find( ext ) != m_ExtensionToTypeMap.end() )
			{
				Console::WriteLine( "[ERROR] AssetLoader: Attempt to register a second type to the specified extension '", ext, "'" );
				return;
			}

			m_ExtensionToTypeMap[ ext ] = typeid( _Ty ).hash_code();
		}
	

		static std::shared_ptr< Asset > LoadFromTypeId( const size_t& inHash, const AssetPath& fileName, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
		{
			auto entry = m_TypeToLoadMap.find( inHash );
			if( entry == m_TypeToLoadMap.end() || !entry->second.first )
			{
				Console::WriteLine( "[WARNING] AssetLoader: Failed to find asset loader for specified type.. creating generic asset..." );
				return Load< GenericAsset >( fileName, Begin, End );
			}

			return entry->second.first( fileName, Begin, End );
		}

		static std::shared_ptr< Asset > LoadFromFileName( const AssetPath& path, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
		{
			auto fileName	= path.GetPath();
			auto exp		= fileName.Explode( '.' );

			if( exp.size() < 2 )
			{
				Console::WriteLine( "[WARNING] AssetLoader: Failed to find asset loader for specified file name.. couldnt read extension" );
				return Load< GenericAsset >( path, Begin, End );
			}

			auto ext = exp.at( exp.size() - 1 ).TrimBoth().ToLower();

			// Lookup the type for this extension
			auto entry = m_ExtensionToTypeMap.find( ext );
			if( entry == m_ExtensionToTypeMap.end() )
			{
				Console::WriteLine( "[WARNING] AssetLoader: Failed to find asset loader for specified file name.. couldnt associate extension with type" );
				return Load< GenericAsset >( path, Begin, End );
			}

			return LoadFromTypeId( entry->second, path, Begin, End );
		}

		static std::shared_ptr< Asset > StreamFromTypeId( const size_t& inHash, const AssetPath& path, AssetStream& inStream )
		{
			// Find load function
			auto entry = m_TypeToLoadMap.find( inHash );
			if( entry == m_TypeToLoadMap.end() || !entry->second.second )
			{
				Console::WriteLine( "[WARNING] AssetLoader: Failed to find stream function for asset type! (", path.GetPath(), ")" );
				return DefaultStream( path, inStream );
			}

			return entry->second.second( path, inStream );
		}

		static std::shared_ptr< Asset > StreamFromFileName( const AssetPath& path, AssetStream& inStream )
		{
			// Get the extension of the file
			auto fileName	= path.GetPath();
			auto exp		= fileName.Explode( '.' );

			if( exp.size() < 2 )
			{
				Console::WriteLine( "[WARNING] AssetLoader: Failed to find asset stream function for specified file name.. couldnt read extension" );
				return DefaultStream( path, inStream );
			}

			auto ext = exp.at( exp.size() - 1 ).TrimBoth().ToLower();

			// Find type_id for this extension
			auto entry = m_ExtensionToTypeMap.find( ext );
			if( entry == m_ExtensionToTypeMap.end() )
			{
				Console::WriteLine( "[WARNING] AssetLoader: Failed to stream asset, couldnt find asset type from extension" );
				return DefaultStream( path, inStream );
			}

			return StreamFromTypeId( entry->second, path, inStream );
		}
	};


	template< typename T >
	class AssetRegistryEntry
	{

	public:

		AssetRegistryEntry()
		{
			AssetLoader::RegisterAssetType< T >();
		}

		AssetRegistryEntry( const String& fileExtension )
		{
			AssetLoader::RegisterAssetType< T >();
			AssetLoader::RegisterAssetFileExtension< T >( fileExtension );
		}

		AssetRegistryEntry( const std::vector< String >& inExtensions )
		{
			AssetLoader::RegisterAssetType< T >();

			for( auto& ext : inExtensions )
			{
				AssetLoader::RegisterAssetFileExtension< T >( ext );
			}
		}

		AssetRegistryEntry( const AssetRegistryEntry& ) = delete;
		AssetRegistryEntry( AssetRegistryEntry&& ) = delete;
		AssetRegistryEntry& operator=( const AssetRegistryEntry& ) = delete;
		AssetRegistryEntry& operator=( AssetRegistryEntry&& ) = delete;

	};


}