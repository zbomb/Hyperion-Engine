/*==================================================================================================
	Hyperion Engine
	Tests/BufferTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <iostream>

#include "Hyperion/Core/Stream.h"


namespace Hyperion
{
namespace Tests
{

	void RunBufferTests()
	{
		std::cout << "--------------------------------------------------------------------------------------------------\n---> Running stream tests!\n\n";
		
		std::cout << "--> Creating buffer...\n\n";
		GenericBuffer buffer;

		{
			std::cout << "--> Writing bytes...\n";
			DataWriter writer( buffer );
			bool res = writer.WriteBytes( { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 } );

			std::cout << ( res ? "----> Write Success!\n" : "----> Write Failed!\n" );
			std::cout << "\n";
		}

		{
			std::cout << "--> Reading bytes...\n";
			DataReader reader( buffer );
			std::vector< byte > ReadData;

			auto result = reader.ReadBytes( ReadData, 10 );
			if( result == DataReader::ReadResult::End )
			{
				std::cout << "----> Hit EOF!\n";
			}
			else if( result == DataReader::ReadResult::Fail )
			{
				std::cout << "----> Read Failed!\n";
			}
			else
			{
				std::cout << "----> Read Success!\n";
			}

			std::cout << "\n";
			Binary::PrintHex( ReadData.begin(), ReadData.end() );

			// Seek the begining + 4
			std::cout << "--> Reading from an offset of 4...\n";
			reader.SeekOffset( 4 );
			std::vector< byte > OtherData;
			auto otherRes = reader.ReadBytes( OtherData, 6 );
			if( otherRes == DataReader::ReadResult::End )
			{
				std::cout << "----> Hit EOF!\n";
			}
			else if( otherRes == DataReader::ReadResult::Fail )
			{
				std::cout << "-----> Failed!\n";
			}
			else
			{
				std::cout << "-----> Success\n";
			}

			std::cout << "\n";
			Binary::PrintHex( OtherData.begin(), OtherData.end() );

			// Intentionally hit EOF
			std::cout << "--> EOF Test... should hit eof on this read...\n";
			reader.SeekBegin();
			std::vector< byte > EOFData;
			auto eofRes = reader.ReadBytes( EOFData, 100 );
			if( eofRes == DataReader::ReadResult::End )
			{
				std::cout << "-----> Hit EOF as expected\n";
			}
			else if( eofRes == DataReader::ReadResult::Fail )
			{
				std::cout << "-----> Failed!\n";
			}
			else
			{
				std::cout << "-----> No EOF or fail?\n";
			}

			std::cout << "\n";
			Binary::PrintHex( EOFData.begin(), EOFData.end() );

			// Try to seek back to begining and read again
			std::cout << "----> Reseeking begining.. and attempting full read\n";
			reader.SeekBegin();
			std::vector< byte > finalData;
			auto finalRes = reader.ReadBytes( finalData, 10 );
			if( finalRes == DataReader::ReadResult::End )
			{
				std::cout << "-----> Hit EOF\n";
			}
			else if( finalRes == DataReader::ReadResult::Fail )
			{
				std::cout << "-----> Failed!\n";
			}
			else
			{
				std::cout << "----> Success\n";
			}

			std::cout << "\n";
			Binary::PrintHex( finalData.begin(), finalData.end() );

			// Test a couple other functions
			std::cout << "\n--> Testing data source size function...\n";
			std::cout << "-----> Size: ";
			std::cout << buffer.Size();
			std::cout << "\n";

			std::cout << "\n--> Testing clear function...\n";
			buffer.Clear();

			std::cout << "-----> Size: ";
			std::cout << buffer.Size();
			std::cout << "\n";
		}

		std::cout << "\n\n-------> Stream tests complete!\n";
		std::cout << "--------------------------------------------------------------------------------------------------\n\n";
	}

}
}