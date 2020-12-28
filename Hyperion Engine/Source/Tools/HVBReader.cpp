/*==================================================================================================
	Hyperion Engine
	Source/Tools/HVBReader.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Tools/HVBReader.h"
#include "Hyperion/Library/Binary.h"



namespace Hyperion
{

	HVBReader::HVBReader( File& inFile )
		: m_Reader( inFile ), m_BlobBegin( 0 )
	{
		m_Reader.SeekBegin();
	}


	HVBReader::~HVBReader()
	{
	}


	bool HVBReader::CheckValidationSequence( const std::vector< byte >& inData )
	{
		static const std::vector< byte > correctSequence =
		{
			0x1A, 0xA1, 0xFF, 0x28,
			0x9D, 0xD9, 0x00, 0x03
		};

		return inData.size() == 8 && std::equal( inData.begin(), inData.end(), correctSequence.begin() );
	}


	HVBReader::ReadHeaderResult HVBReader::ReadHeader( HVBReader::HeaderData& Out )
	{
		// First, seek to the begining, and check the validation sequence
		m_Reader.SeekBegin();

		std::vector< byte > validBytes;
		if( m_Reader.ReadBytes( validBytes, 8 ) != DataReader::ReadResult::Success || !CheckValidationSequence( validBytes ) )
		{
			m_Reader.SeekBegin();
			return ReadHeaderResult::BadValidation;
		}

		// Next, were going to read the rest of the header into a single vector
		std::vector< byte > headerData;
		if( m_Reader.ReadBytes( headerData, 12 ) != DataReader::ReadResult::Success )
		{
			m_Reader.SeekBegin();
			return ReadHeaderResult::MissingData;
		}

		// Lets deserialize each number..
		Out.BlobOffset		= 0;
		Out.FileCount		= 0;
		Out.TotalSize		= 0;

		auto It = headerData.begin();

		Binary::DeserializeUInt32( It, Out.BlobOffset );
		Binary::DeserializeUInt32( It + 4, Out.TotalSize );
		Binary::DeserializeUInt32( It + 8, Out.FileCount );

		m_BlobBegin = Out.BlobOffset;

		return ReadHeaderResult::Success;
	}


	bool HVBReader::StartFiles()
	{
		// We want to advance the read position to the start of the file list
		if( m_Reader.Size() < 20 )
		{
			Console::WriteLine( "[ERROR] HVBReader: Failed to start reading files.. size of file invalid!" );
			return false;
		}

		if( m_BlobBegin == 0 )
		{
			Console::WriteLine( "[ERROR] HVBReader: Failed to start reading files.. the header needs to be read first!" );
			return false;
		}

		m_Reader.SeekOffset( 20 );
		return true;
	}


	bool HVBReader::NextFile()
	{
		// Return false if we hit the end of the file list
		return m_Reader.GetOffset() < m_BlobBegin;
	}


	HVBReader::ReadFileResult HVBReader::ReadFileInfo( HVBReader::FileInfo& Out )
	{
		// Ensure data structure is zeroed out
		Out.AssetIdentifier		= 0;
		Out.FileLength			= 0;
		Out.FileOffset			= 0;
		Out.PathLengthBytes		= 0;
		Out.Path.Clear();

		// Read the next 2 bytes to get the size of the string
		std::vector< byte > strLenData;
		if( m_Reader.ReadBytes( strLenData, 2 ) != DataReader::ReadResult::Success )
		{
			return ReadFileResult::InvalidPath;
		}

		Binary::DeserializeUInt16( strLenData.begin(), Out.PathLengthBytes );

		// Now, ensure there are enough bytes to read this entry
		std::streamoff totalLength = (uint32) Out.PathLengthBytes + 12; /* 2-bytes for str length, X-bytes for string itself, 4-bytes for offset, 4-bytes for length, 4-bytes for asset identifier, 
																X + 14 total, but weve already read 2-bytes, so we want to ensure there is 'X + 12' bytes left in the stream */
		if( m_BlobBegin <= m_Reader.GetOffset() + totalLength )
		{
			return ReadFileResult::NotEnoughData;
		}

		// Now lets read the string in
		std::vector< byte > strData;
		if( m_Reader.ReadBytes( strData, Out.PathLengthBytes ) != DataReader::ReadResult::Success )
		{
			return ReadFileResult::InvalidPath;
		}

		Out.Path = std::move( String( strData, StringEncoding::UTF8 ) );
		std::vector< byte >().swap( strData );

		if( Out.Path.Length() <= 3 )
		{
			return ReadFileResult::InvalidPath;
		}

		// Finally, read the rest of the data in
		std::vector< byte > restData;
		if( m_Reader.ReadBytes( restData, 12 ) != DataReader::ReadResult::Success )
		{
			return ReadFileResult::NotEnoughData;
		}

		// Deserialize the rest of the structure
		auto It = restData.begin();

		Binary::DeserializeUInt32( It, Out.FileOffset );
		std::advance( It, 4 );

		Binary::DeserializeUInt32( It, Out.FileLength );
		std::advance( It, 4 );

		Binary::DeserializeUInt32( It, Out.AssetIdentifier );
		std::advance( It, 4 );

		return ReadFileResult::Success;
	}

}