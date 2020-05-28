using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hyperion
{
	static class Deflate
	{

		public static bool PerformDeflate( byte[] inData, out byte[] outData, byte inCompressionLevel = 6 )
		{
			outData = null;
			throw new Exception( "Deflate algorithm hasnt been implemented" );
			return false;
		}


		public static bool PerformInflate( byte[] inData, out byte[] outData )
		{
			outData = null;

			if( inData.LongLength == 0L )
			{
				Core.WriteLine( "[ERROR] Inflate: Failed to inflate, input data was null or empty" );
				return false;
			}

			// Were going to do this the more 'low level' way, since its the way I already know how to use deflate
			// Create the buffer and setup the stream
			var zStream = new zlib.ZStream();
			var buffer = new byte[ 256 * 1024 ];

			zStream.next_in = inData;
			zStream.next_in_index = 0;
			zStream.avail_in = inData.Length;
			zStream.next_out = buffer;
			zStream.next_out_index = 0;
			zStream.avail_out = buffer.Length;

			// Init the inflate algorithm
			if( zStream.inflateInit() != zlib.zlibConst.Z_OK )
			{
				Core.WriteLine( "[ERROR] Infalte: Failed, couldnt initialize" );
				return false;
			}

			// Start performing inflate on the input data
			while( zStream.avail_in != 0 )
			{
				var r = zStream.inflate( zlib.zlibConst.Z_NO_FLUSH );
				if( r == zlib.zlibConst.Z_STREAM_END )
				{
					break;
				}

				if( r != zlib.zlibConst.Z_OK )
				{
					zStream.inflateEnd();
					Core.WriteLine( "[ERROR] Inflate: Failed, main algorithm failed (", r, ")" );

					outData = null;
					return false;
				}

				// Check if we filled the output buffer
				if( zStream.avail_out == 0 )
				{
					// Copy data into output array, reset buffer
					var outSize = outData?.Length ?? 0;
					Array.Resize( ref outData, outSize + buffer.Length );
					Array.ConstrainedCopy( buffer, 0, outData, outSize, buffer.Length );

					zStream.next_out_index = 0;
					zStream.avail_out = buffer.Length;
				}
			}

			// Finish inflate
			int res = zlib.zlibConst.Z_OK;
			while( res == zlib.zlibConst.Z_OK )
			{
				if( zStream.avail_out == 0 )
				{
					// Copy data into output array, reset buffer
					var outSize = outData?.Length ?? 0;
					Array.Resize( ref outData, outSize + buffer.Length );
					Array.ConstrainedCopy( buffer, 0, outData, outSize, buffer.Length );

					zStream.next_out_index = 0;
					zStream.avail_out = buffer.Length;
				}

				res = zStream.inflate( zlib.zlibConst.Z_FINISH );
			}

			// Ensure we hit the end of the stream
			if( res != zlib.zlibConst.Z_STREAM_END )
			{
				zStream.inflateEnd();
				Core.WriteLine( "[ERROR] Inflate: Failed to hit the end of the stream!" );

				outData = null;
				return false;
			}

			// Copy remaining buffer data into the output 
			var nOutSize    = outData?.Length ?? 0;
			var dataSize    = buffer.Length - zStream.avail_out;

			Array.Resize( ref outData, nOutSize + dataSize );
			Array.ConstrainedCopy( buffer, 0, outData, nOutSize, dataSize );

			zStream.inflateEnd();
			return true;
		}

	}
}
