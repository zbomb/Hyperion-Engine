/*==================================================================================================
	Hyperion Engine
	Source/Core/VirtualFileSystem.cpp
	© 2019, Zachary Berry
==================================================================================================*/


#include "Hyperion/Core/VirtualFileSystem.h"
#include "Hyperion/Core/File.h"


namespace Hyperion
{

	/*
		Static Definitions
	*/
	GroupManifestEntry VirtualFileSystem::m_InvalidGroupEntry;
	AssetManifestEntry VirtualFileSystem::m_InvalidAssetEntry;
	std::unordered_map< String, GroupManifestEntry > VirtualFileSystem::m_Manifest;
	std::unordered_map< String, String > VirtualFileSystem::m_AssetIndex;

	/*-----------------------------------------------------------
		AssetStream
	-----------------------------------------------------------*/
	AssetStream::AssetStream( std::unique_ptr< File >&& inFile, std::streamoff beginOffset, std::streamoff endOffset )
		: m_TargetFile( std::move( inFile ) ), m_LinkedReader( m_TargetFile ), m_Begin( beginOffset ), m_End( endOffset )
	{
		HYPERION_VERIFY( m_TargetFile && m_End >= m_Begin && m_LinkedReader.Size() >= ( m_Begin + m_End ), "Invalid parameters to asset stream!" );

		// Seek the clamped begining of the file
		m_LinkedReader.SeekOffset( m_Begin );
	}

	AssetStream::~AssetStream()
	{
	}

	AssetStream::AssetStream( AssetStream&& Other ) noexcept
		: m_TargetFile( std::move( Other.m_TargetFile ) ), m_LinkedReader( m_TargetFile ), 
		m_Begin( std::move( Other.m_Begin ) ), m_End( std::move( Other.m_End ) )
	{
		// Reset linked reader position to the previous instances position
		m_LinkedReader.SeekOffset( Other.m_LinkedReader.GetOffset() );
	}

	std::streamoff AssetStream::GetSize() const
	{
		return m_End - m_Begin;
	}

	std::streamoff AssetStream::GetOffset()
	{
		return m_LinkedReader.GetOffset() - m_Begin;
	}

	void AssetStream::SeekOffset( std::streamoff Offset )
	{
		// Ensure this offset is within range
		if( Offset <= GetSize() )
		{
			m_LinkedReader.SeekOffset( m_Begin + Offset );
		}
		else
		{
			Console::WriteLine( "[Warning] AssetStream: Atempt to seek to an out of range position!" );
		}
	}

	void AssetStream::SeekBegin()
	{
		SeekOffset( 0 );
	}

	void AssetStream::SeekEnd()
	{
		SeekOffset( m_End );
	}

	AssetStream::ReadResult AssetStream::ReadBytes( std::vector< byte >& outData, size_t inCount, size_t& outCount )
	{
		// First, determine we wont hit the end of our limited range, if so then we need to handle it
		auto localEnd	= ( m_LinkedReader.GetOffset() - m_Begin ) + std::streamoff( inCount );
		bool bHitEnd	= false;

		if( localEnd > m_End )
		{
			// Hit the end of stream!
			bHitEnd		= true;
			inCount		= (size_t)localEnd;
		}

		// Now we can just call the underlying read function normally
		auto result = m_LinkedReader.ReadBytes( outData, inCount, outCount );
		if( result == DataReader::ReadResult::Success )
		{
			return bHitEnd ? ReadResult::End : ReadResult::Success;
		}
		else if( result == DataReader::ReadResult::End )
		{
			return ReadResult::End;
		}
		else
		{
			return ReadResult::Fail;
		}
	}

	AssetStream::ReadResult AssetStream::ReadBytes( std::vector< byte >& outData, size_t inCount )
	{
		size_t _dummy;
		return ReadBytes( outData, inCount, _dummy );
	}

	AssetStream::ReadResult AssetStream::PeekBytes( std::vector< byte >& outData, size_t inCount, size_t& outCount )
	{
		auto cur_pos = m_LinkedReader.GetOffset();
		auto result = ReadBytes( outData, inCount, outCount );
		m_LinkedReader.SeekOffset( cur_pos );
		return result;
	}

	AssetStream::ReadResult AssetStream::PeekBytes( std::vector< byte >& outData, size_t inCount )
	{
		size_t _dummy;
		return PeekBytes( outData, inCount, _dummy );
	}


	
	bool VirtualFileSystem::FileExists( const String& relativePath )
	{
		return FindAssetEntry( relativePath ).first;
	}

	std::unique_ptr< AssetStream > VirtualFileSystem::StreamFile( const String& relativePath )
	{
		HYPERION_VERIFY_BASICSTR( relativePath );
		// TODO: ToLower( relativePath );

		// We need to get the asset and group manifests for this asset
		auto manifestInfo = FindAssetAndGroupEntry( relativePath );
		if( !manifestInfo.first )
		{
			return nullptr;
		}
		
		// Now we have the info needed to stream this section from the chunk file
		return StreamChunkSection( manifestInfo.second.second.ChunkFile, manifestInfo.second.first.Begin, manifestInfo.second.first.Begin + manifestInfo.second.first.Length );
	}

	std::unique_ptr< AssetStream > VirtualFileSystem::StreamChunkSection( const String& chunkIdentifier, std::streamoff Begin, std::streamoff End )
	{
		HYPERION_VERIFY_BASICSTR( chunkIdentifier );
		// TODO: ToLower( chunkIdentifier );

		auto f = IFileSystem::OpenFile( FilePath( chunkIdentifier, PathRoot::Assets ), FileMode::Read );
		if( !f )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Asset chunk file wasnt found!" );
			return nullptr;
		}

		// Now, ensure the offsets are correct
		if( f->Size() < End )
		{
			Console::WriteLine( "[ERROR] VitualFileSystem: Asset chunk file size was less than the requested end offset!" );
			return nullptr;
		}

		return std::make_unique< AssetStream >( std::move( f ), Begin, End );
	}

	bool VirtualFileSystem::ReadFile( const String& relativePath, std::vector< byte >& outData )
	{
		HYPERION_VERIFY_BASICSTR( relativePath );
		// TODO: ToLower( relativePath );

		// First, we need to get the manifest information
		auto manifestInfo = FindAssetAndGroupEntry( relativePath );
		if( !manifestInfo.first )
		{
			return false;
		}

		// Now, we need to try and open the target chunk file
		auto f = IFileSystem::OpenFile( FilePath( manifestInfo.second.second.ChunkFile, PathRoot::Assets ), FileMode::Read );
		if( !f )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Asset chunk file wasnt found.. but the manifest entry appeared valid!" );
			return false;
		}

		auto beginOffset	= manifestInfo.second.first.Begin;
		auto dataLength		= manifestInfo.second.first.Length;
		auto endOffset		= beginOffset + dataLength;

		if( f->Size() < endOffset )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Asset chunk file size was less than the requested end offset!" );
			return false;
		}

		// Now, we will use a normal data reader to get this data into the output vector
		DataReader r( f );
		r.SeekOffset( beginOffset );

		auto res = r.ReadBytes( outData, (size_t) dataLength );
		if( res == DataReader::ReadResult::Fail )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Failed to read asset from chunk file!" );
			return false;
		}
		else if( res == DataReader::ReadResult::End )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Failed to read asset from chunk file.. hit end of file!" );
			return false;
		}
		else
		{
			return true;
		}
	}

	bool VirtualFileSystem::BatchReadGroup( const String& groupIdentifier, std::map< String, std::vector< byte > >& outData )
	{
		HYPERION_VERIFY_BASICSTR( groupIdentifier );
		// TODO: ToLower( groupIdentifier );

		// Find group manifest
		auto groupEntry = FindGroupEntry( groupIdentifier );
		if( !groupEntry.first )
		{
			return false;
		}

		if( groupEntry.second.Assets.size() == 0 )
		{
			// No assets in this group
			return true;
		}

		// Open this groups chunk file
		auto f = IFileSystem::OpenFile( FilePath( groupEntry.second.ChunkFile, PathRoot::Assets ), FileMode::Read );
		if( !f )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Failed to batch read group.. couldnt open the chunk file!" );
			return false;
		}

		// Determine the data range that contians all of the assets we want to read
		// Groups are store continuously in memory, so we could also jsut store the offsets for the group in the manifest structure and avoid this loop
		std::streamoff groupBegin	= -1;
		std::streamoff groupEnd		= -1;

		for( auto It = groupEntry.second.Assets.begin(); It != groupEntry.second.Assets.end(); It++ )
		{
			if( groupBegin < 0 || It->second.Begin < groupBegin )
			{
				groupBegin = It->second.Begin;
			}

			auto assetEnd = It->second.Begin + It->second.Length;

			if( assetEnd > groupEnd )
			{
				groupEnd = assetEnd;
			}
		}

		// Validate offsets
		if( groupBegin < 0 || groupEnd < 0 || f->Size() < groupEnd )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Failed to batch read group.. invalid offsets calculated!" );
			return false;
		}

		// Read the entire data range we need into a vector
		std::vector< byte > groupData;
		DataReader reader( f );

		reader.SeekOffset( groupBegin );
		auto result = reader.ReadBytes( groupData, (size_t)( groupEnd - groupBegin ) );
		if( result == DataReader::ReadResult::Fail )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Failed to batch read group.. i/o operation failed!" );
			return false;
		}
		else if( result == DataReader::ReadResult::End )
		{
			Console::WriteLine( "[ERROR] VirtualFileSystem: Failed to batch read group.. hit the end of the chunk file!" );
			return false;
		}

		// Now we need to move this data in sections into the output map
		for( auto It = groupEntry.second.Assets.begin(); It != groupEntry.second.Assets.end(); It++ )
		{
			auto& entry = outData[ It->first ] = std::vector< byte >();
			
			// TODO: Rewrite this to avoid having this extra memory stay allocated until the operation is finished
			// Right now, when we move the data, the space is still allocated in the original vector
			// But we cant erase as we go, because then, the offsets we have would become invalidated
			// The only way to accomplish this would be to sort the assets by their position in memory

			auto assetBegin	= It->second.Begin - groupBegin;
			auto assetEnd	= assetBegin + It->second.Length;

			entry.insert( entry.end(), 
				std::make_move_iterator( std::next( groupData.begin(), (int) assetBegin ) ), // Warning: Converting from long long to int.... this greatly limits our available asset size!
				std::make_move_iterator( std::next( groupData.begin(), (int) assetEnd ) ) );

		}

		groupData.erase( groupData.begin(), groupData.end() );
		return true;
	}

	std::pair< bool, GroupManifestEntry& > VirtualFileSystem::FindGroupEntry( const String& groupIdentifier )
	{
		HYPERION_VERIFY_BASICSTR( groupIdentifier );

		// TODO: ToLower( groupIdentifier );

		auto groupEntry = m_Manifest.find( groupIdentifier );
		if( groupEntry == m_Manifest.end() )
		{
			// Group not found!
			return std::pair< bool, GroupManifestEntry& >( false, m_InvalidGroupEntry );
		}
		else
		{
			return std::pair< bool, GroupManifestEntry& >( true, groupEntry->second );
		}
	}

	std::pair< bool, AssetManifestEntry& > VirtualFileSystem::FindAssetEntry( const String& assetIdentifier, const String& groupIdentifier )
	{
		HYPERION_VERIFY_BASICSTR( assetIdentifier );
		HYPERION_VERIFY_BASICSTR( groupIdentifier );

		// TODO: ToLower( assetIdentifier ), ToLower( groupIdentifier );

		// First, we need to find the target group
		auto groupEntry = m_Manifest.find( groupIdentifier );
		if( groupEntry == m_Manifest.end() ) return std::pair< bool, AssetManifestEntry& >( false, m_InvalidAssetEntry );

		// Now try and find this asset in the group
		auto assetEntry = groupEntry->second.Assets.find( assetIdentifier );
		if( assetEntry == groupEntry->second.Assets.end() ) return std::pair< bool, AssetManifestEntry& >( false, m_InvalidAssetEntry );

		return std::pair< bool, AssetManifestEntry& >( true, assetEntry->second );
	}

	std::pair< bool, AssetManifestEntry& > VirtualFileSystem::FindAssetEntry( const String& assetIdentifier )
	{
		HYPERION_VERIFY_BASICSTR( assetIdentifier );

		// TODO: ToLower( assetIdentifier )

		// Use the secondary index to quickly find the proper group
		auto targetGroup = m_AssetIndex.find( assetIdentifier );
		if( targetGroup == m_AssetIndex.end() ) return std::pair< bool, AssetManifestEntry& >( false, m_InvalidAssetEntry );

		return FindAssetEntry( assetIdentifier, targetGroup->second );
	}

	std::pair< bool, std::pair< AssetManifestEntry&, GroupManifestEntry& > > VirtualFileSystem::FindAssetAndGroupEntry( const String& assetIdentifier )
	{
		HYPERION_VERIFY_BASICSTR( assetIdentifier );

		// TODO: ToLower( assetIdentifier );

		// Use secondary index to find group name
		auto targetGroup = m_AssetIndex.find( assetIdentifier );
		if( targetGroup == m_AssetIndex.end() ) return std::pair( false, std::pair< AssetManifestEntry&, GroupManifestEntry& >( m_InvalidAssetEntry, m_InvalidGroupEntry ) );

		// Next we need to get a reference to the group
		auto groupEntry = m_Manifest.find( targetGroup->second );
		if( groupEntry == m_Manifest.end() ) return std::pair( false, std::pair< AssetManifestEntry&, GroupManifestEntry& >( m_InvalidAssetEntry, m_InvalidGroupEntry ) );

		// Finally, find the asset in this group
		auto assetEntry = groupEntry->second.Assets.find( assetIdentifier );
		if( assetEntry == groupEntry->second.Assets.end() ) return std::pair( false, std::pair< AssetManifestEntry&, GroupManifestEntry& >( m_InvalidAssetEntry, m_InvalidGroupEntry ) );

		// Return all of these references to the caller
		return std::pair( true, std::pair< AssetManifestEntry&, GroupManifestEntry& >( assetEntry->second, groupEntry->second ) );
	}


}