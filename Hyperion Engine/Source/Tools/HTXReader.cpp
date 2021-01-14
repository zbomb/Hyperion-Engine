/*==================================================================================================
	Hyperion Engine
	Source/Tools/HTXReader.cpp
	© 2019, Zachary Berry
==================================================================================================*/

/*
*	TODO:
*	- We should be allowing variations in endian order.. currently only works with big endian
*/

#include "Hyperion/Tools/HTXReader.h"
#include "Hyperion/Library/Binary.h"


namespace Hyperion
{

	HTXReader::HTXReader( IFile& inFile, uint64 inOffset, uint64 inLength )
		: m_Reader( inFile ), m_bValidFormat( false ), m_Target( inFile ), m_Offset( inOffset ), m_Length( inLength )
	{
		// Check if this file is a valid texture file
		m_Reader.SeekOffset( inOffset );

		std::vector< byte > headerSequence;
		static const std::vector< byte > correctSequence =
		{
			0x1A, 0xA1, 0xFF, 0x28,
			0x9D, 0xD9, 0x00, 0x02 // <- Last two bytes differentiates hyperion formats, 0x00, 0x02 is .htx
		};

		if( m_Reader.Size() < 20 || ( m_Length != 0 && m_Length < 20 ) ||
			m_Reader.ReadBytes( headerSequence, 8 ) != DataReader::ReadResult::Success ||
			!std::equal( correctSequence.begin(), correctSequence.end(), headerSequence.begin() ) )
		{
			Console::WriteLine( "[WARNING] HTXReader: '", inFile.GetPath().ToString(), "' is not a valid .htx format file!" );
		}
		else
		{
			m_bValidFormat = true;
		}
	}


	HTXReader::~HTXReader()
	{

	}


	HTXReader::Result HTXReader::ReadHeader( TextureHeader& outHeader )
	{
		// During construction, we check if the file appears valid, so lets see if that returned false
		if( !m_bValidFormat )
		{
			#ifdef HYPERION_DEBUG
			Console::WriteLine( "[DEBUG] HTXReader: Attempt to read header data from '", m_Target.GetPath().ToString(), "' but this files format is invalid" );
			#endif
			return Result::InvalidFormat;
		}

		// Next, we need to come up with a format for these files

		// --- Validation Bytes --- || Fmt | LOD Padding | LOD Count | RSVD || -------- Addtl Reserved -------- || ~~~~~ LOD List ~~~~~ || ~~~~~ Pixel Data ~~~~~~ || EOF
		// [0][1][2][3][4][5][6][7] || [8] |    [9]      |    [10]   | [11] || [12][13][14][15][16][17][18][19] || [...]                || [...]                   || EOF

		// Minimum File Size:
		// Empty Texture: 8 + 4 + 8
		// There would be 0 LODs and 0 pixels, so we would have 20 bytes minimum

		// LOD List Entry Format
		//  Width | Height || -- Offset -- | ---- Size ---- || --- Row Size --- | END
		// [0][1] | [2][3] || [4][5][6][7] | [8][9][10][11] || [12][13][14][15] | END

		// LOD Entry Size: 16 bytes

		// First, lets read in all of the data fields from the header 
		m_Reader.SeekOffset( 8  +  m_Offset);
		std::vector< byte > headerData;
		if( m_Reader.ReadBytes( headerData, 12 ) != DataReader::ReadResult::Success )
		{
			#ifdef HYPERION_DEBUG
			Console::WriteLine( "[DEBUG] HTXReader: Failed to read texture asset '", m_Target.GetPath().ToString(), "' because the static header data couldnt be read!" );
			#endif

			return Result::InvalidFormat;
		}

		uint8 formatNum		= headerData.at( 0 );
		uint8 lodPadding	= headerData.at( 1 );
		uint8 lodCount		= headerData.at( 2 );

		// TODO: Any future fields will be read from the data here

		// Validate and convert values we just read
		if( !IsValidTextureAssetFormat( (TextureFormat) formatNum ) )
		{
			#ifdef HYPERION_DEBUG
			Console::WriteLine( "[DEBUG] HTXReader: Failed to read texture asset '", m_Target.GetPath().ToString(), "' because the format (", (uint32) formatNum, ") is not a valid texture asset format!" );
			#endif

			return Result::InvalidHeader;
		}
		else if( lodCount > TEXTURE_MAX_LODS )
		{
			#ifdef HYPERION_DEBUG
			Console::WriteLine( "[DEBUG] HTXReader: Failed to read texture asset '", m_Target.GetPath().ToString(), "' because the number of LODs is invalid (", lodCount, ")" );
			#endif

			return Result::InvalidHeader;
		}

		outHeader.Format		= static_cast< TextureFormat >( formatNum );
		outHeader.LODPadding	= lodPadding;

		// Now, we have all of the static data read in, lets validate the size of the file, ensure we have enough data to read the LOD level(s) in
		uint64 fileSize = m_Length == 0 ? (uint64)m_Reader.Size() : m_Length;
		if( fileSize > (uint64)m_Reader.Size() )
		{
			Console::WriteLine( "[DEBUG] HTXReader: Failed to read texture asset '", m_Target.GetPath().ToString(), "' because the desired length is larger than the current file" );
			return Result::InvalidFormat;
		}

		if( fileSize < ( lodCount * 16 ) + 20  )
		{
			#ifdef HYPERION_DEBUG
			Console::WriteLine( "[DEBUG] HTXReader: Failed to read texture asset '", m_Target.GetPath().ToString(), "' because there isnt enough data for the LOD header list (",
								fileSize, "bytes in file)" );
			#endif

			return Result::InvalidFormat;
		}

		// Loop and read each entry in
		for( uint8 i = 0; i < lodCount; i++ )
		{
			TextureLOD newLOD;

			// Read all of the data for this LOD in a single call
			std::vector< byte > lodData;
			if( m_Reader.ReadBytes( lodData, 16 ) != DataReader::ReadResult::Success )
			{
				#ifdef HYPERION_DEBUG
				Console::WriteLine( "[DEBUG] HTXReader: Failed to read texture asset '", m_Target.GetPath().ToString(), "' because an LOD level (", (uint32)i, ") failed to read in!" );
				#endif

				return Result::InvalidFormat;
			}

			// Deserialize each value in the data we read
			auto It = lodData.begin();

			Binary::DeserializeUInt16( It, newLOD.Width, false );
			std::advance( It, 2 );
			
			Binary::DeserializeUInt16( It, newLOD.Height, false );
			std::advance( It, 2 );
			
			uint32 off = 0;
			Binary::DeserializeUInt32( It, off, false );
			std::advance( It, 4 );

			newLOD.FileOffset = off;

			Binary::DeserializeUInt32( It, newLOD.LODSize, false );
			std::advance( It, 4 );

			Binary::DeserializeUInt32( It, newLOD.RowSize, false );

			// Validate this LOD structure
			if( ( newLOD.Height * newLOD.RowSize > newLOD.LODSize ) ||
				( newLOD.FileOffset + newLOD.LODSize > fileSize ) )
			{
				#ifdef HYPERION_DEBUG
				Console::WriteLine( "[DEBUG] HTXReader: Failed to read texture asset '", m_Target.GetPath().ToString(), "' LOD #", (uint32) i, " because the data failed validation" );
				#endif

				outHeader.LODs.clear();
				return Result::InvalidLOD;
			}
			
			outHeader.LODs.push_back( newLOD );
		}

		return Result::Success;
	}


	HTXReader::Result HTXReader::ReadRawData( uint64 inOffset, uint32 inSize, std::vector< byte >& outData )
	{
		// Clear the output data vector
		std::vector< byte >().swap( outData );

		// Ensure this file was already deemed valid
		if( !m_bValidFormat )
		{
			#ifdef HYPERION_DEBUG
			Console::WriteLine( "[DEBUG] HTXReader: Attempt to read data from '", m_Target.GetPath().ToString(), "' but this files format is invalid" );
			#endif

			return Result::InvalidFormat;
		}

		// Ensure the ranges provided fall in the readers range
		m_Reader.SeekBegin( m_Offset );
		uint64 fileSize = m_Length == 0 ? (uint64)m_Reader.Size() : m_Length;

		if( inOffset + (uint64)inSize > fileSize )
		{
			#ifdef HYPERION_DEBUG
			Console::WriteLine( "[DEBUG] HTXReader: Failed to read data from '", m_Target.GetPath().ToString(), "' but the requested range falls outside of the file size" );
			#endif

			return Result::InvalidParams;
		}

		// Read the data
		m_Reader.SeekOffset( m_Offset + inOffset );
		if( m_Reader.ReadBytes( outData, inSize ) != DataReader::ReadResult::Success )
		{
			#ifdef HYPERION_DEBUG
			Console::WriteLine( "[DEBUG] HTXReader: Failed to read data from '", m_Target.GetPath().ToString(), "' but the reader failed to read the data" );
			#endif

			return Result::Failed;
		}

		return Result::Success;
	}


	HTXReader::Result HTXReader::ReadLODData( const TextureHeader& inHeader, uint8 inLevel, std::vector< byte >& outData )
	{
		// Clear the output data vector
		std::vector< byte >().swap( outData );

		// Ensure this file was already deemed valid
		if( !m_bValidFormat )
		{
			#ifdef HYPERION_DEBUG
			Console::WriteLine( "[DEBUG] HTXReader: Attempt to read LOD data from '", m_Target.GetPath().ToString(), "' but this files format is invalid" );
			#endif

			return Result::InvalidFormat;
		}

		// Find the offset and size of the LOD data
		if( inLevel >= inHeader.LODs.size() )
		{
			#ifdef HYPERION_DEBUG
			Console::WriteLine( "[DEBUG] HTXReader: Attempt to read an invalid LOD level from '", m_Target.GetPath().ToString(), "' (", (uint32) inLevel, ")" );
			#endif

			return Result::InvalidLOD;
		}

		auto& targetLOD = inHeader.LODs.at( inLevel );
		
		return ReadRawData( targetLOD.FileOffset, targetLOD.LODSize, outData );
	}


	HTXReader::Result HTXReader::ReadLODRange( const TextureHeader& inHeader, std::vector< uint8 >& inLevels, std::map< uint8, std::vector< byte > >& outData )
	{
		// CLear the output data vector
		outData.clear();

		if( !m_bValidFormat )
		{
			#ifdef HYPERION_DEBUG
			Console::WriteLine( "[DEBUG] HTXReader: Attempt to read LOD range data from '", m_Target.GetPath().ToString(), "' but this files format is invalid" );
			#endif

			return Result::InvalidFormat;
		}

		// Ensure all LOD levels are valid, and there are no repeating levels
		std::sort( inLevels.begin(), inLevels.end() );

		auto validCount = inHeader.LODs.size();
		std::vector< byte >::const_iterator lastIt = inLevels.end();

		for( auto It = inLevels.begin(); It != inLevels.end(); It++ )
		{
			// First, ensure this is a vlaid LOD level
			if( *It >= validCount )
			{
				#ifdef HYPERION_DEBUG
				Console::WriteLine( "[DEBUG] HTXReader: Failed to read LOD range for '", m_Target.GetPath().ToString(), "' because there is an invalid LOD in the input range (", *It, ")" );
				#endif
				return Result::InvalidParams;
			}

			// Next, if were not the first iteratation, compare against the last value
			if( lastIt != inLevels.end() )
			{
				if( *It == *lastIt )
				{
					#ifdef HYPERION_DEBUG
					Console::WriteLine( "[DEBUG] HTXReader: Failed to read LOD range for '", m_Target.GetPath().ToString(), "' because there are duplicate LODs in the input range (", *It, ")" );
					#endif
					return Result::InvalidParams;
				}
			}

			lastIt = It;
		}

		// Now, go through each LOD level and read the data into the output vector
		for( auto& l : inLevels )
		{
			auto& targetLOD = inHeader.LODs.at( l );

			auto res = ReadRawData( targetLOD.FileOffset, targetLOD.LODSize, outData[ l ] );
			if( res != Result::Success )
			{
				#ifdef HYPERION_DEBUG
				Console::WriteLine( "[DEBUG] HTXReader: Failed tor ead LOD range for '", m_Target.GetPath().ToString(), "' because LOD ", l, " couldnt be read!" );
				#endif

				return res;
			}
		}

		return Result::Success;
	}

}