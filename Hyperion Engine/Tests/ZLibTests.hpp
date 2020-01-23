/*==================================================================================================
	Hyperion Engine
	Tests/ZLibTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

// Engine Includes
#include "Hyperion/Tools/Deflate.h"


namespace Hyperion
{
namespace Tests
{
	void RunZLibTests()
	{
		std::this_thread::sleep_for( std::chrono::seconds( 2 ) );

		Console::WriteLine( "============================================================================================================\n" );
		Console::WriteLine( "-> Running Deflate Test...." );

		Console::WriteLine( "---> Constructing source data..." );
		std::vector< byte > SourceData;
		const uint32 dataSize = 1024 * 1024;

		for( uint32 i = 0; i < dataSize; i++ )
		{
			SourceData.push_back( i );
		}

		std::vector< byte > CompressedData;
		Console::WriteLine( "---> Compressing..." );

		if( !Tools::Deflate::PerformDeflate( SourceData, CompressedData ) )
		{
			Console::WriteLine( "--------> Compression failed!" );
		}
		else
		{
			Console::WriteLine( "--------> Compression success!" );
		}

		Console::Write( "\n" );

		// We can isnert a breakpoint here to check out the compressed data
		Console::WriteLine( "---> Decompressing..." );

		std::vector< byte > DecompressedData;
		if( !Tools::Deflate::PerformInflate( CompressedData, DecompressedData ) )
		{
			Console::WriteLine( "--------> Decompression failed!" );
		}
		else
		{
			Console::WriteLine( "--------> Decompression success!" );
		}

		// Now, lets just check if this data is correct
		Console::WriteLine( "\n---> Checking decompressed data integreity..." );

		bool bMatch = true;

		for( uint32 i = 0; i < DecompressedData.size(); i++ )
		{
			if( SourceData.size() < i || DecompressedData.at( i ) != SourceData.at( i ) )
			{
				bMatch = false;
				break;
			}
		}

		if( bMatch )
		{
			Console::WriteLine( "--------> Decompressed data is valid!" );
		}
		else
		{
			Console::WriteLine( "--------> Decompressed data is invalid!" );
		}

		Console::Write( "\n" );
		Console::WriteLine( "--> Deflate test complete!" );
		Console::WriteLine( "============================================================================================================\n" );

	}
}
}