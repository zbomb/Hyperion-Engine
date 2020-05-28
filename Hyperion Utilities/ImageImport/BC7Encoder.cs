
using System;
using System.Collections.Generic;


namespace Hyperion
{

	public static class BC7Encoder
	{

		public static bool Encode( TextureImportLOD inLOD, bool bIncludeAlpha, TextureLODParameters inParams, out byte[] outData, out uint outRowSize )
		{
			// Lets just do some quick checks
			if( ( ( inLOD.Data?.LongLength ?? 0L ) != ( inLOD.Width * inLOD.Height * 4L ) ) ||
				inLOD.Width == 0 || inLOD.Height == 0 )
			{
				Core.WriteLine( "[ERROR] BC7 Encoder: Input parameters were invalid?" );
				outData = null;
				outRowSize = 0;

				return false;
			}

			// Determine changes in the source => output formats
			bool bGainingAlpha = bIncludeAlpha && ( inLOD.SourceFormat == ImageFormat.GS || inLOD.SourceFormat == ImageFormat.RGB );
			bool bColorIncrease = ( inLOD.SourceFormat == ImageFormat.GS || inLOD.SourceFormat == ImageFormat.GSA );

			// We need to go through the source data, and make sure it conforms to the conversion parameters
			// before we pass the data into comrpessonator to process it
			for( uint y = 0; y < inLOD.Height; y++ )
			{
				for( uint x = 0; x < inLOD.Width; x++ )
				{
					// Read into the source data 
					long memOffset = ( ( y * inLOD.Width ) + x ) * 4L;

					// Now, we need to modify the data based on the parameters
					// If we go from non-alpha, to alpha, we need to modify the alpha channel
					if( bGainingAlpha )
					{
						inLOD.Data[ memOffset + 3L ] = inParams.alphaParam;
					}

					// Now, if were going from grayscale to RGB, we need to modify the data as well
					if( bColorIncrease )
					{
						if( !inParams.colorParams.r ) { inLOD.Data[ memOffset ] = ( byte ) 0; }
						if( !inParams.colorParams.g ) { inLOD.Data[ memOffset + 1L ] = ( byte ) 0; }
						if( !inParams.colorParams.b ) { inLOD.Data[ memOffset + 2L ] = ( byte ) 0; }
					}
				}
			}

			// Call into C++ to perform the encoding
			var result = BC7Library.Encode( inLOD.Data, inLOD.Width, inLOD.Height, bIncludeAlpha, out outData, out outRowSize );
			if( result != BC7Result.Success )
			{
				Core.WriteLine( "[Warning] BC7 Encoder: Failed to encode a BC-7 texture! Err #" + ( (int) result ).ToString() );
				outData = null;
				outRowSize = 0;

				return false;
			}

			return true;
		}

	}

}