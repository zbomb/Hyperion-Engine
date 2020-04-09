/*==================================================================================================
	Hyperion Engine
	Source/Tools/HHTReader.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Tools/HHTReader.h"
#include "Hyperion/Library/Binary.h"


/*
	* .hht Layout
	--- Header Sequence ----|-- Meta Data --|-- Data Section --
	[0][1][2][3][4][5][6][7]  [8][9][10][11] [12] ...

	* Header Sequence: Fixed set of bytes letting us know this is a valid .hht file
	* MetaData: Currently, no data is actually stored here
		[8]  -> Always Zero
		[9]  -> Always Zero
		[10] -> Always Zero
		[11] -> Always Zero

	* Data Section Layout
		- Each entry in the data section is laid out sequentially, with no padding
		- Each entry in the data section contains...
			1. Hash Code (uint32)
			2. Data Length [in bytes] (uint16)
			3. Data (variable length)

	* Example:
	-- Hash Code ---| Data Len | -- Data --
	[12][13][14][15]  [16][17]  [18]...

	* Minimum File Size:
		- 8 + 4 + 4 + 2 = 18

	* Data Section Start:
		- 12
*/


namespace Hyperion
{

	HHTReader::HHTReader( PhysicalFile& inFile )
		: m_Reader( inFile )
	{
		m_Reader.SeekBegin();
	}


	HHTReader::~HHTReader()
	{

	}


	bool HHTReader::Validate()
	{
		// Check header sequence, to ensure this is a valid HHT file, also check length
		m_Reader.SeekBegin();

		static const std::vector< byte > correctSequence =
		{
			0x1A, 0xA1, 0xFF, 0x28,
			0x9D, 0xD9, 0x00, 0x01
		};

		if( m_Reader.Size() < 18 )
		{
			Console::WriteLine( "[WARNING] HHTReader: Failed to validate .hht file, not enough data" );
			return false;
		}

		std::vector< byte > fileSequence;

		if( m_Reader.ReadBytes( fileSequence, 8 ) != DataReader::ReadResult::Success ||
			!std::equal( correctSequence.begin(), correctSequence.end(), fileSequence.begin() ) )
		{
			Console::WriteLine( "[WARNING] HHTReader: Failed to validate .hht file, invalid header sequence" );
			return false;
		}

		// Were going to skip reading the metadata, since currently it doesnt hold any info

		return true;
	}

	
	void HHTReader::Begin()
	{
		// Jump to the start of the data section
		m_Reader.SeekBegin( 12 );
	}


	bool HHTReader::NextEntry()
	{
		// Check if there is enough data left for another entry
		// There needs to be at least 6 bytes left in the stream
		return( m_Reader.Size() - m_Reader.GetOffset() >= 6 );
	}


	HHTReader::Result HHTReader::ReadEntry( uint32& outHash, std::vector< byte >& outData )
	{
		// Attempt to read the next hash from the table
		std::vector< byte > hashData;
		auto result = m_Reader.ReadBytes( hashData, 6 );

		if( result == DataReader::ReadResult::End )
		{
			return Result::End;
		}
		else if( result != DataReader::ReadResult::Success )
		{
			return Result::Error;
		}

		// Next, deserialize this data into numbers
		auto hashIt = hashData.begin();

		Binary::DeserializeUInt32( hashIt, outHash );
		std::advance( hashIt, 4 );

		uint16 dataLen;
		Binary::DeserializeUInt16( hashIt, dataLen );

		// Now, read the data from the table
		auto readRes = m_Reader.ReadBytes( outData, (size_t) dataLen );
		if( readRes == DataReader::ReadResult::End )
		{
			return Result::End;
		}
		else if( readRes != DataReader::ReadResult::Success )
		{
			return Result::Error;
		}

		return Result::Success;
	}


}