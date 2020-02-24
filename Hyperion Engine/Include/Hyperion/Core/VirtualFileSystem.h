/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/VirtualFileSystem.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"
#include "Hyperion/Core/Stream.h"
#include "Hyperion/Core/File.h"

#include <vector>

namespace Hyperion
{

	class AssetStream
	{

	private:

		std::streamoff m_Begin;
		std::streamoff m_End;
		std::unique_ptr< File > m_TargetFile;
		DataReader m_LinkedReader;

	public:

		AssetStream() = delete;
		AssetStream( std::unique_ptr< File >&&, std::streamoff, std::streamoff );
		~AssetStream();

		AssetStream( AssetStream&& ) noexcept;
		AssetStream( const AssetStream& ) = delete;
		AssetStream& operator=( const AssetStream& ) = delete;

		inline std::streamoff GetSize() const;
		inline std::streamoff GetOffset();

		void SeekOffset( std::streamoff );
		void SeekBegin();
		void SeekEnd();

		enum ReadResult
		{
			Success,
			Fail,
			End
		};

		ReadResult ReadBytes( std::vector< byte >& outData, size_t inCount, size_t& outCount );
		ReadResult ReadBytes( std::vector< byte >& outData, size_t inCount);
		ReadResult PeekBytes( std::vector< byte >& outData, size_t inCount, size_t& outCount );
		ReadResult PeekBytes( std::vector< byte >& outData, size_t inCount );
	};

	enum class AssetCacheMode
	{
		Dynamic = 0,
		Static = 1,
		Stream = 2
	};

	struct AssetManifestEntry
	{
		String Name;
		std::streamoff Begin;
		std::streamoff Length;
		String Type;
	};

	struct GroupManifestEntry
	{
		String Name;
		String ChunkFile;
		AssetCacheMode Mode;
		std::unordered_map< String, AssetManifestEntry > Assets;
	};

	class VirtualFileSystem
	{

	private:

		static GroupManifestEntry m_InvalidGroupEntry;
		static AssetManifestEntry m_InvalidAssetEntry;

		static std::unordered_map< String, GroupManifestEntry > m_Manifest;
		static std::unordered_map< String, String > m_AssetIndex;
		static std::map< String, std::unique_ptr< File > > m_ChunkHandles;

		static bool ProcessManifest( std::unique_ptr< File >& inFile, const String& chunkName );

	public:

		VirtualFileSystem() = delete;

		static bool FileExists( const String& relativePath );

		static std::unique_ptr< AssetStream > StreamFile( const String& relativePath );
		static bool ReadFile( const String& relativePath, std::vector< byte >& outData );
		static bool BatchReadGroup( const String& groupIdentifier, std::map< String, std::vector< byte > >& outData );

		static std::unique_ptr< AssetStream > StreamChunkSection( const String& chunkIdentifier, std::streamoff Begin, std::streamoff End );

		static std::pair< bool, GroupManifestEntry& > FindGroupEntry( const String& groupIdentifier );

		static std::pair< bool, AssetManifestEntry& > FindAssetEntry( const String& assetIdentifier );
		static std::pair< bool, AssetManifestEntry& > FindAssetEntry( const String& assetIdentifier, const String& groupIdentifier );
		static std::pair< bool, std::pair< AssetManifestEntry&, GroupManifestEntry& > > FindAssetAndGroupEntry( const String& assetIdentifier );

		static void MountChunks();
		static void UnMountChunks();


	};

}