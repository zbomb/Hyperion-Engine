/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Tools/HTXWriter.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Tools/HTXWriter.h"
#include "Hyperion/Library/Binary.h"


namespace Hyperion
{

	HTXWriter::Result HTXWriter::Write( std::unique_ptr< PhysicalFile >& inFile, HTXWriter::Input& inData )
	{
		// First, we need to validate the input
		if( !IsValidTextureAssetFormat( inData.Format ) ||
			inData.LODs.size() > TEXTURE_MAX_LODS )
		{
			return Result::InvalidInput;
		}

		if( !inFile || !inFile->IsValid() || !inFile->CanWriteStream() )
		{
			return Result::InvalidFile;
		}

		DataWriter Writer( inFile );
		Writer.SeekBegin();

		// First, we need to write the validation sequence
		static const std::vector< byte > headerSequence =
		{
			0x1A, 0xA1, 0xFF, 0x28,
			0x9D, 0xD9, 0x00, 0x02 // <- Last two bytes differentiates hyperion formats, 0x00, 0x02 is .htx
		};

		std::vector< byte > OutData( headerSequence );

		// Next, add the format byte
		Binary::SerializeUInt8( (uint8) inData.Format, OutData );

		// LOD Padding
		Binary::SerializeUInt8( inData.LevelPadding, OutData );

		// LOD Count
		Binary::SerializeUInt8( (uint8) inData.LODs.size(), OutData );

		// 9-bytes of filler data [RSVD]
		Binary::SerializeUInt8( 0, OutData );
		Binary::SerializeUInt64( 0, OutData );

		std::vector< uint32 > OffsetList;
		uint32 lastOffset = 20 + ( 16 * (uint32)inData.LODs.size() ); // This is the size of the header data (20 bytes) plus the list of LODs (16 bytes each)

		for( auto& lod : inData.LODs )
		{
			OffsetList.push_back( lastOffset );
			lastOffset += (uint32)lod.Data.size() + inData.LevelPadding;
		}

		// Next up we need to write the header data for each LOD
		for( uint8 i = 0; i < inData.LODs.size(); i++ )
		{
			// Each entry is 16-bytes
			// First, we need to write the width and height, each two bytes
			auto& target = inData.LODs.at( i );

			Binary::SerializeUInt16( target.Width, OutData );
			Binary::SerializeUInt16( target.Height, OutData );

			// Next, we need to calculate the offset in the file for the start of the data for this LOD level
			Binary::SerializeUInt32( OffsetList.at( i ), OutData );

			// Now, we need to figure out the size of the LOD
			Binary::SerializeUInt32( (uint32)target.Data.size(), OutData );

			// Finally, we need the row size
			Binary::SerializeUInt32( target.RowSize, OutData );
		}

		// Now, we need to go through and write the pixel data (and fillers) for each LOD level
		for( auto& target : inData.LODs )
		{
			OutData.insert( OutData.end(), target.Data.begin(), target.Data.end() );
			OutData.insert( OutData.end(), inData.LevelPadding, 0 );
		}

		// Now all data is written to a vector, so now write this to file
		if( Writer.WriteBytes( OutData ) )
		{
			return Result::Success;
		}
		else
		{
			return Result::WriteFailed;
		}
	}

}
