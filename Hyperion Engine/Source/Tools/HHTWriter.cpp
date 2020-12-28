/*==================================================================================================
	Hyperion Engine
	Source/Tools/HHTWriter.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Tools/HHTWriter.h"
#include "Hyperion/Library/Binary.h"


namespace Hyperion
{

	HHTWriter::HHTWriter( File& inFile, bool bIsNewFile /* = true */ )
		: m_File( inFile ), m_Writer( inFile )
	{
		if( bIsNewFile )
		{
			_WriteHeader();
		}
	}


	HHTWriter::~HHTWriter()
	{
		auto res = Flush();
		if( res != Result::Success )
		{
			Console::WriteLine( "[WARNING] HHTWriter: Failed to flush to file, error code ", (uint32) res );
		}
	}


	void HHTWriter::_WriteHeader()
	{
		// The header is basically just a uint64, big-endian, with a specific value so we know its supposed to be a .hht file
		// Then, followed by another 4 bytes of reserved data
		static const std::vector< byte > headerData =
		{
			0x1A, 0xA1, 0xFF, 0x28,
			0x9D, 0xD9, 0x00, 0x01,
			0x00, 0x00, 0x00, 0x00
		};

		m_Writer.SeekBegin();
		if( !m_Writer.WriteBytes( headerData ) )
		{
			Console::WriteLine( "[WARNING] HHTWriter: Failed to write header to new file '", m_File.GetPath().ToString(), "'!" );
			m_bHeaderFailed = true;
		}
		else
		{
			m_bHeaderFailed = false;
		}
	}


	void HHTWriter::AddEntry( uint32 hashData, const std::vector< byte >& valueData )
	{
		m_Entries.emplace( hashData, valueData );
	}


	void HHTWriter::ClearList()
	{
		m_Entries.clear();
	}


	HHTWriter::Result HHTWriter::Flush()
	{
		if( m_bHeaderFailed ) { return Result::WriteFailed; }
		if( m_Entries.empty() ) { return Result::Success; }

		for( auto It = m_Entries.begin(); It != m_Entries.end(); )
		{
			// Serialize this entry
			std::vector< byte > entryData;
			Binary::SerializeUInt32( It->first, entryData );

			auto dataSize = It->second.size();
			if( std::numeric_limits< uint16 >::max() < dataSize )
			{
				return Result::SizeOverflow;
			}

			Binary::SerializeUInt16( (uint16) dataSize, entryData );
			entryData.insert( entryData.end(), It->second.begin(), It->second.end() );

			// Write to file
			if( !m_Writer.WriteBytes( entryData ) )
			{
				return Result::WriteFailed;
			}

			It = m_Entries.erase( It );
		}

		return Result::Success;
	}

}