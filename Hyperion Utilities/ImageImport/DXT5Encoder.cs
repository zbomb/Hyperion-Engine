
using System;
using System.Collections.Generic;


namespace Hyperion
{

	public static class DXT5Encoder
	{

		public static bool Encode( TextureImportLOD inLOD, TextureLODParameters inParams, out byte[] outData, out uint outRowSize )
		{
			// Lets just do some quick checks
			if( ( ( inLOD.Data?.LongLength ?? 0L ) != ( inLOD.Width * inLOD.Height * 4L ) ) ||
				inLOD.Width == 0 || inLOD.Height == 0 )
			{
				Core.WriteLine( "[ERROR] DXT5 Encoder: Input parameters were invalid?" );
				outData = null;
				outRowSize = 0;

				return false;
			}

			// This format is an RGBA format, so lets check if were coming from grayscale, or a non-alpha format
			bool bColorIncrease = ( inLOD.SourceFormat == ImageFormat.GS || inLOD.SourceFormat == ImageFormat.GSA );
			bool bAlphaIncrease = ( inLOD.SourceFormat == ImageFormat.GS || inLOD.SourceFormat == ImageFormat.RGB );

			// Loop through each source pixel, and process it before passing the data off to the compressor
			for( uint y = 0; y < inLOD.Height; y++ )
			{
				for( uint x = 0; x < inLOD.Width; x++ )
				{
					// Calculate the mem offset for this pixel
					long memOffset = ( ( y * inLOD.Width ) + x ) * 4L;

					// If were increasing the numebr of color channels, we need to clear data out of unwanted channels, since when images are imported
					// the grayscale data gets placed into all 3 color channels
					if( bColorIncrease )
					{
						if( !inParams.colorParams.r ) { inLOD.Data[ memOffset ] = ( byte ) 0; }
						if( !inParams.colorParams.g ) { inLOD.Data[ memOffset + 1L ] = ( byte ) 0; }
						if( !inParams.colorParams.b ) { inLOD.Data[ memOffset + 2L ] = ( byte ) 0; }
					}

					// Now, check if we are coming from a format without an alpha channel
					if( bAlphaIncrease )
					{
						inLOD.Data[ memOffset + 3L ] = inParams.alphaParam; // alpha param contains the value of the alpha channel we want
					}
				}
			}

			// Call into C++ to perform the encoding
			var result = DXT5Library.Encode( inLOD.Data, inLOD.Width, inLOD.Height, out outData, out outRowSize );
			if( result != DXT5Result.Success )
			{
				Core.WriteLine( "[Warning] DXT5 Encoder: Failed to encode a DXT5 texture! Err #" + ( ( int ) result ).ToString() );
				outData = null;
				outRowSize = 0;

				return false;
			}

			return true;
		}

	}

}