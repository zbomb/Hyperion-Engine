
using System;
using System.Collections.Generic;


namespace Hyperion
{

	public static class DXT1Encoder
	{

		public static bool Encode( TextureImportLOD inLOD, TextureLODParameters inParams, out byte[] outData, out uint outRowSize )
		{
			// Lets just do some quick checks
			if( ( ( inLOD.Data?.LongLength ?? 0L ) != ( inLOD.Width * inLOD.Height * 4L ) ) ||
				inLOD.Width == 0 || inLOD.Height == 0 )
			{
				Core.WriteLine( "[ERROR] DXT1 Encoder: Input parameters were invalid?" );
				outData = null;
				outRowSize = 0;

				return false;
			}

			// Determine if we are increasing color channel count
			bool bColorIncrease		= ( inLOD.SourceFormat == ImageFormat.GS || inLOD.SourceFormat == ImageFormat.GSA );
			bool bAlphaLoss			= ( inLOD.SourceFormat == ImageFormat.GSA || inLOD.SourceFormat == ImageFormat.RGBA );

			// The source data needs to be converted before we pass the data into the algorithm to compress it
			// DXT1 is a non-alpha, RGB only format, so we need to check if were going from grayscale, or an alpha format
			for( uint y = 0; y < inLOD.Height; y++ )
			{
				for( uint x = 0; x < inLOD.Width; x++ )
				{
					// Calculate the mem offset for this pixel
					long memOffset = ( ( y * inLOD.Width ) + x ) * 4L;

					// First, handle changes in color channel count
					if( bColorIncrease )
					{
							// FIgure out what channels we strip color from
						if( !inParams.colorParams.r ) { inLOD.Data[ memOffset ] = ( byte ) 0; }
						if( !inParams.colorParams.g ) { inLOD.Data[ memOffset + 1L ] = ( byte ) 0; }
						if( !inParams.colorParams.b ) { inLOD.Data[ memOffset + 2L ] = ( byte ) 0; }
					}

					// Now, handle loss of alpha
					if( bAlphaLoss )
					{
						// For now, we dont have any methods besides loosing the source alpha
						inLOD.Data[ memOffset + 3L ] = ( byte ) 255;
					}
				}
			}


			// Call into C++ to perform the encoding
			var result = DXT1Library.Encode( inLOD.Data, inLOD.Width, inLOD.Height, out outData, out outRowSize );
			if( result != DXT1Result.Success )
			{
				Core.WriteLine( "[Warning] DXT1 Encoder: Failed to encode a DXT1 texture! Err #" + ( ( int ) result ).ToString() );
				outData = null;
				outRowSize = 0;

				return false;
			}

			return true;
		}

	}

}