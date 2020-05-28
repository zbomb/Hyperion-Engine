
using System;
using System.Collections.Generic;


namespace Hyperion
{
	/*
	 * How are various formats imported?
	 * 
	 * Grayscale: R/B/G are set to the gray value, if alpha is defined, its also set, if not, then its default to 255
	 * Grayscale Alpha: Same as above
	 * 
	 * RGB: Normal import
	 * RGBA Same as above
	*/

	static class GrayscaleEncoder
	{

		public static bool Encode( TextureImportLOD inLOD, bool bIncludeAlpha, TextureLODParameters inParams, out byte[] outData, out uint outRowSize )
		{
			// Lets just do some quick checks
			if( ( ( inLOD.Data?.LongLength ?? 0L ) != ( inLOD.Width * inLOD.Height * 4L ) ) ||
				inLOD.Width == 0 || inLOD.Height == 0 )
			{
				Core.WriteLine( "[ERROR] Grayscale Encoder: Input parameters were invalid?" );
				outData		= null;
				outRowSize	= 0;

				return false;
			}

			outRowSize			= inLOD.Width * ( bIncludeAlpha ? 2U : 1U );
			long outputSize		= outRowSize * inLOD.Height;
			outData				= new byte[ outputSize ];
			long currentOffset  = 0L;

			// Now, we need to go through each pixel in the source data, and figure out how to convert it to a grayscale pixel
			for( uint y = 0; y < inLOD.Height; y++ )
			{
				for( uint x = 0; x < inLOD.Width; x++ )
				{
					// We need to get the location of the pixel in memory
					long memOffset = ( ( y * (long)inLOD.Width ) + (long)x ) * 4L;

					// And now we need to read in each channel
					byte r = inLOD.Data[ memOffset ];
					byte g = inLOD.Data[ memOffset + 1L ];
					byte b = inLOD.Data[ memOffset + 2L ];
					byte a = inLOD.Data[ memOffset + 3L ];

					// Now, based on the parameters, we need to decide how to calculate 'gray'
					uint grayAccum = 0;
					uint grayCount = 0;

					if( inParams.colorParams.r )
					{
						grayAccum += r;
						grayCount++;
					}

					if( inParams.colorParams.g )
					{
						grayAccum += g;
						grayCount++;
					}

					if( inParams.colorParams.b )
					{
						grayAccum += b;
						grayCount++;
					}

					outData[ currentOffset++ ] = (byte)( grayAccum / grayCount );

					if( bIncludeAlpha )
					{
						// And now, we need to figure out the alpha param
						// We might not be using alpha though, so lets check that first
						byte finalAlpha;
						switch( inLOD.SourceFormat )
						{
							case ImageFormat.GS:
							case ImageFormat.RGB:

								// If there is no source alpha, use the parameter
								finalAlpha = inParams.alphaParam;
								break;

							default:

								finalAlpha = a;
								break;
						}

						outData[ currentOffset++ ] = finalAlpha;
					}
				}
			}

			return true;
		}

	}

}