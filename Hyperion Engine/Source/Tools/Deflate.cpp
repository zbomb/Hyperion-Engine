/*==================================================================================================
	Hyperion Engine
	Source/Tools/Deflate.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Tools/Deflate.h"
#include "Hyperion/Extern/zlib/zlib.h"


constexpr auto CHUNK_SIZE = 256 * 1024; // 256kb buffer size


namespace Hyperion
{
namespace Tools
{
	/*
	*	Deflate::PerformDeflate
	*	* Note: If this function returns false, the output buffer is invalid!
	*/
	bool Deflate::PerformDeflate( const std::vector< byte >& Input, std::vector< byte >& Output, uint8 CompressionLevel /* = 6 */  )
	{
		return PerformDeflate( Input.begin(), Input.end(), Output, CompressionLevel );
	}


	bool Deflate::PerformInflate( const std::vector< byte >& Input, std::vector< byte >& Output )
	{
		return PerformInflate( Input.begin(), Input.end(), Output );
	}


	bool Deflate::PerformDeflate( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, std::vector< byte >& Output, uint8 CompressionLevel /* = 6 */ )
	{
		Output.clear();

		// Validate compression level
		if( CompressionLevel < 0 || CompressionLevel > 9 )
		{
			Console::WriteLine( "[ERROR] Deflate Library: Invalid compression level (", CompressionLevel, ") must be between 0 and 9, where 6 is default" );
			return false;
		}

		auto inSize = std::distance( Begin, End );

		// Check to ensure input data isnt empty
		if( inSize <= 0 )
		{
			Console::WriteLine( "[Warning] Deflate Library: Attempt to perform deflate on an empty data set!" );
			return true;
		}

		// Setup zlib stuff...
		z_stream stream;
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;

		uint8* tempBuffer = new uint8[ CHUNK_SIZE ];

		// Dirty work... so we can use const_iterator with this function...
		// TODO: Better Solution? TODO TODO TODO
		stream.next_in = const_cast< byte* >( std::addressof( *Begin ) );
		stream.avail_in = (uint32) inSize;
		stream.next_out = tempBuffer;
		stream.avail_out = CHUNK_SIZE;

		// Init the deflate stream
		if( deflateInit( &stream, (int) CompressionLevel ) != Z_OK )
		{
			Console::WriteLine( "[ERROR] Deflate Library: Failed to initialize!" );
			delete[] tempBuffer;
			return false;
		}

		// Perform deflate on input data
		while( stream.avail_in != 0 )
		{
			if( deflate( &stream, Z_NO_FLUSH ) != Z_OK )
			{
				deflateEnd( &stream );

				Console::WriteLine( "[ERROR] Defalte Library: Failed to perform deflate on input data! (err#1)" );
				delete[] tempBuffer;
				Output.clear();

				return false;
			}

			// Check if weve filled the output buffer, if so, copy the data into the final output vector
			if( stream.avail_out == 0 )
			{
				Output.insert( Output.end(), tempBuffer, tempBuffer + CHUNK_SIZE );

				stream.next_out = tempBuffer;
				stream.avail_out = CHUNK_SIZE;
			}
		}

		// Finish deflate
		int result = Z_OK;
		while( result == Z_OK )
		{
			if( stream.avail_out == 0 )
			{
				Output.insert( Output.end(), tempBuffer, tempBuffer + CHUNK_SIZE );

				stream.next_out = tempBuffer;
				stream.avail_out = CHUNK_SIZE;
			}

			result = deflate( &stream, Z_FINISH );
		}

		// Ensure we hit the end of the stream, otherwise there was an error
		if( result != Z_STREAM_END )
		{
			deflateEnd( &stream );

			Console::WriteLine( "[ERROR] Deflate Library: Failed to perform deflate on input data (err#2)" );
			delete[] tempBuffer;
			Output.clear();

			return false;
		}

		// Insert the last of the data into the output buffer
		Output.insert( Output.end(), tempBuffer, tempBuffer + CHUNK_SIZE - stream.avail_out );
		deflateEnd( &stream );
		delete[] tempBuffer;

		return true;
	}


	bool Deflate::PerformInflate( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, std::vector< byte >& Output )
	{
		auto inSize = std::distance( Begin, End );

		if( inSize <= 0 )
		{
			Console::WriteLine( "[Warning] Deflate Library: Failed to inflate, the input data was null" );
			return false;
		}

		z_stream stream;
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;

		uint8* tempBuffer = new uint8[ CHUNK_SIZE ];

		// Unfortunatley, this only accepts a non-const pointer, so were going to do some dirty work here
		// The main place this gets used is reading asset data, and that passed out a const_itersator
		// TODO: Better Solution? TODO TODO TODO
		stream.next_in = const_cast< byte* >( std::addressof( *Begin ) );
		stream.avail_in = (uint32) inSize;
		stream.next_out = tempBuffer;
		stream.avail_out = CHUNK_SIZE;

		// Init the deflate stream
		if( inflateInit( &stream ) != Z_OK )
		{
			Console::WriteLine( "[ERROR] Deflate Library: Failed to initialize inflate stream" );
			delete[] tempBuffer;
			return false;
		}

		// Perform inflate on the input data
		while( stream.avail_in != 0 )
		{
			auto res = inflate( &stream, Z_NO_FLUSH );

			if( res == Z_STREAM_END )
				break;

			if( res != Z_OK )
			{
				inflateEnd( &stream );

				Console::WriteLine( "[ERROR] Deflate Library: Inflate call failed! (err#1) [", res, "]" );
				delete[] tempBuffer;
				Output.clear();

				return false;
			}

			// Check if weve filled our output buffer
			if( stream.avail_out == 0 )
			{
				// Copy data into output buffer, reset the temp buffer
				Output.insert( Output.end(), tempBuffer, tempBuffer + CHUNK_SIZE );

				stream.next_out = tempBuffer;
				stream.avail_out = CHUNK_SIZE;
			}
		}

		// Finish inflate
		int result = Z_OK;
		while( result == Z_OK )
		{
			if( stream.avail_out == 0 )
			{
				Output.insert( Output.end(), tempBuffer, tempBuffer + CHUNK_SIZE );

				stream.next_out = tempBuffer;
				stream.avail_out = CHUNK_SIZE;
			}

			result = inflate( &stream, Z_FINISH );
		}

		// Ensure we hit the end of the stream
		if( result != Z_STREAM_END )
		{
			inflateEnd( &stream );

			Console::WriteLine( "[ERROR] Deflate Library: Failed to hit the end of the infalte stream!" );
			delete[] tempBuffer;
			Output.clear();

			return false;
		}

		// Copy remaining data in the temp buffer to the output vector
		Output.insert( Output.end(), tempBuffer, tempBuffer + CHUNK_SIZE - stream.avail_out );
		inflateEnd( &stream );
		delete[] tempBuffer;

		return true;
	}

}
}