using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hyperion
{
	class ImageUtils
	{

		/*
		 * ImageUtils::SwapChannel( bbyte[], byte[] )
		 * This function can perform channel swaps on uncompressed image data, that 4 channels, each one byte (i.e. RGBA_8888)
		 * The swap matrix tells us how the transform should happen, the index represents the source channel, and the value represents which channel to place it in the output
		 * data set. i.e. { 3, 2, 1, 0 } would reverse the channels
		 */
		public static bool SwapChannels( byte[] inData, int[] swapMatrix )
		{
			// Ensure the parameters are present and appear valid
			if( inData == null || swapMatrix == null || inData.LongLength == 0L || inData.LongLength % 4L != 0L || swapMatrix.Length != 4 )
			{
				return false;
			}

			// Ensure the swap matrix is valid
			if( !swapMatrix.Contains( 0 ) || !swapMatrix.Contains( 1 ) || !swapMatrix.Contains( 2 ) || !swapMatrix.Contains( 3 ) )
			{
				return false;
			}

			for( long offset = 0; offset < inData.LongLength; offset += 4 )
			{
				byte val0 = inData[ offset ];
				byte val1 = inData[ offset + 1L ];
				byte val2 = inData[ offset + 2L ];
				byte val3 = inData[ offset + 3L ];

				for( int sourceIndex = 0; sourceIndex < swapMatrix.Length; sourceIndex++ )
				{
					int destIndex = swapMatrix[ sourceIndex ];

					if( sourceIndex == 0 )			{ inData[ offset + destIndex ] = val0; }
					else if( sourceIndex == 1 )		{ inData[ offset + destIndex ] = val1; }
					else if( sourceIndex == 2 )		{ inData[ offset + destIndex ] = val2; }
					else if( sourceIndex == 3 )		{ inData[ offset + destIndex ] = val3; }
				}
			}

			return true;
		}

	}
}
