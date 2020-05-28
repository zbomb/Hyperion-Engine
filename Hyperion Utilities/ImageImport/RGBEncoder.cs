
using System;
using System.Collections.Generic;


namespace Hyperion
{

	public static class RGBEncoder
	{

		public static bool Encode( TextureImportLOD inLOD, bool bSRGB, TextureLODParameters inParams, out byte[] outData, out uint outRowSize )
		{
			// Lets just do some quick checks
			if( ( ( inLOD.Data?.LongLength ?? 0L ) != ( inLOD.Width * inLOD.Height * 4L ) ) ||
				inLOD.Width == 0 || inLOD.Height == 0 )
			{
				Core.WriteLine( "[ERROR] RGB Encoder: Input parameters were invalid?" );
				outData = null;
				outRowSize = 0;

				return false;
			}

			outRowSize			= inLOD.Width * 4;
			long outputSize     = outRowSize * inLOD.Height;
			outData				= new byte[ outputSize ];
			long outOffset      = 0L;

			for( uint y = 0; y < inLOD.Height; y++ )
			{
				for( uint x = 0; x < inLOD.Width; x++ )
				{
					// Calculate the offset and read the source pixel in
					long memOffset = ( ( y * inLOD.Width ) + x ) * 4L;

					byte r = inLOD.Data[ memOffset ];
					byte g = inLOD.Data[ memOffset + 1L ];
					byte b = inLOD.Data[ memOffset + 2L ];
					byte a = inLOD.Data[ memOffset + 3L ];

					if( bSRGB )
					{
						throw new NotImplementedException( "sRGB encoding has not been implemented yet!" );
					}
					else
					{
						byte finalR = 0;
						byte finalG = 0;
						byte finalB = 0;

						// Now if were going from grayscale to RGB, we need to select which channels to place data into
						switch( inLOD.SourceFormat )
						{
							case ImageFormat.GS:
							case ImageFormat.GSA:

								// In grayscale source, r, g, and b are all the same value, so we just read from r
								finalR = inParams.colorParams.r ? r : (byte) 0;
								finalG = inParams.colorParams.g ? g : (byte) 0;
								finalB = inParams.colorParams.b ? b : (byte) 0;

								break;

							default:

								finalR = r;
								finalG = g;
								finalB = b;

								break;
						}

						// Now, we need to calculate the final alpha value
						byte finalA = 255;

						switch( inLOD.SourceFormat )
						{
							case ImageFormat.GS:
							case ImageFormat.RGB:

								// Were going from no alpha, to alpha, so we need to use the parameter value
								finalA = inParams.alphaParam;
								break;

							default:

								finalA = a;
								break;
						}

						// Just pass the values through into the output stream
						outData[ outOffset++ ] = finalR;
						outData[ outOffset++ ] = finalG;
						outData[ outOffset++ ] = finalB;
						outData[ outOffset++ ] = finalA;
					}
				}
			}

			return true;
		}

	}

}