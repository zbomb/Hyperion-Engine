using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hyperion.Util
{

	public struct GeneratedMip
	{
		public byte[] Data;
		public uint Width;
		public uint Height;
	}

	public enum MipSampleMethod
	{
		Average = 0
	}


	static class MipGenerator
	{

		private struct PixelSample
		{
			public uint x;
			public uint y;
		}

		private struct WeightedPixelSample
		{
			public uint x;
			public uint y;
			public float weight;
		}

		private static bool IsPowerOfTwo( uint inNum )
		{
			return ( inNum != 0 ) && ( ( inNum & ( inNum - 1 ) ) == 0 );
		}


		private static bool PerformPowerOfTwoAverageSample( uint x, uint y, uint w, uint h, uint sourceWidth, uint sourceHeight, byte[] sourceData, out byte[] pixelData )
		{
			pixelData = new byte[ 4 ];

			// This is a little more efficient than the odd version of the same function
			// First, we want a list of all pixels we need to sample from the source image
			uint sampleW = sourceWidth / w;
			uint sampleH = sourceHeight / h;

			uint sampleX = sampleW * x;
			uint sampleY = sampleH * y;

			var samplePixels = new PixelSample[ sampleW * sampleH ];

			for( uint j = 0; j < sampleH; j++ )
			{
				for( uint i = 0; i < sampleW; i++ )
				{
					uint index = i + ( j * sampleW );

					samplePixels[ index ].x = sampleX + i;
					samplePixels[ index ].y = sampleY + j;
				}
			}

			// Now, we need to add up all of the values, and average them out to get the output
			uint totalRed		= 0;
			uint totalGreen		= 0;
			uint totalBlue		= 0;
			uint totalAlpha		= 0;

			foreach( var px in samplePixels )
			{
				uint startIndex = ( px.x * 4 ) + ( px.y * sourceWidth * 4 );

				totalRed	+= sourceData[ startIndex++ ];
				totalGreen	+= sourceData[ startIndex++ ];
				totalBlue	+= sourceData[ startIndex++ ];
				totalAlpha	+= sourceData[ startIndex ];
			}

			totalRed		/= (uint) samplePixels.Length;
			totalGreen		/= (uint) samplePixels.Length;
			totalBlue		/= (uint) samplePixels.Length;
			totalAlpha		/= (uint) samplePixels.Length;

			if( totalRed > 255 || totalGreen > 255 || totalBlue > 255 || totalAlpha > 255 )
			{
				return false;
			}

			pixelData[ 0 ] = (byte) totalRed;
			pixelData[ 1 ] = (byte) totalGreen;
			pixelData[ 2 ] = (byte) totalBlue;
			pixelData[ 3 ] = (byte) totalAlpha;

			return true;
		}


		private static bool PerformOddPowerAverageSample( uint x, uint y, uint w, uint h, uint sourceWidth, uint sourceHeight, byte[] sourceData, out byte[] pixelData )
		{
			pixelData = new byte[ 4 ];

			// Since we can have an odd number of pixels, the way we sample source pixels is much more complicated
			var pixelW          =  sourceWidth / (float) w;
			var pixelH          = sourceHeight / (float) h;
			var sourceXStart    = pixelW * (float) x;
			var sourceYStart    = pixelH * (float) y;
			var sourceXEnd      = sourceXStart + pixelW;
			var sourceYEnd      = sourceYStart + pixelH;

			if( pixelW < 1.0f || pixelH < 1.0f )
			{
				return false;
			}

			// First, we need to generate a list of pixels to sample, and the weight of each
			var xSamples = new List< WeightedPixelSample >();
			var ySamples = new List< WeightedPixelSample >();

			for( float i = (float) Math.Floor( sourceXStart ); i < sourceXEnd; i++ )
			{
				var targetPixel = new WeightedPixelSample()
				{
					x = (uint) i,
					y = 0
				};

				if( i < sourceXStart )
				{
					targetPixel.weight = ( i + 1.0f ) - sourceXStart;
				}
				else if( i > sourceXEnd )
				{
					targetPixel.weight = sourceXEnd - i;
				}
				else
				{
					targetPixel.weight = 1.0f;
				}

				xSamples.Add( targetPixel );
			}

			for( float i = (float) Math.Floor( sourceYStart ); i < sourceYEnd; i++ )
			{
				var targetPixel = new WeightedPixelSample()
				{
					x = 0,
					y = (uint) i
				};

				if( i < sourceYStart )
				{
					targetPixel.weight = ( i + 1.0f ) - sourceYStart;
				}
				else if( i > sourceYEnd )
				{
					targetPixel.weight = sourceYEnd - i;
				}
				else
				{
					targetPixel.weight = 1.0f;
				}

				ySamples.Add( targetPixel );
			}

			var finalPixels = new List<WeightedPixelSample>();
			foreach( var xpx in xSamples )
			{
				foreach( var ypx in ySamples )
				{
					finalPixels.Add( new WeightedPixelSample()
					{
						x = xpx.x,
						y = ypx.y,
						weight = xpx.weight * ypx.weight
					} );
				}
			}

			xSamples.Clear();
			ySamples.Clear();

			// Now, we have the final pixel list has been built, along with the proper weights
			// Now, we just have to actually perform the math to calculate the output pixel color
			uint totalRed = 0;
			uint totalGreen = 0;
			uint totalBlue = 0;
			uint totalAlpha = 0;

			foreach( var px in finalPixels )
			{
				uint sourceOffset = ( px.x * 4 ) + ( px.y * sourceWidth * 4 );

				totalRed	+= sourceData[ sourceOffset++ ];
				totalGreen	+= sourceData[ sourceOffset++ ];
				totalBlue	+= sourceData[ sourceOffset++ ];
				totalAlpha	+= sourceData[ sourceOffset ];
			}

			totalRed /= (uint) finalPixels.Count;
			totalGreen /= (uint) finalPixels.Count;
			totalBlue /= (uint) finalPixels.Count;
			totalAlpha /= (uint) finalPixels.Count;

			if( totalRed > 255 || totalGreen > 255 || totalBlue > 255 || totalAlpha > 255 )
			{
				return false;
			}

			pixelData[ 0 ] = (byte) totalRed;
			pixelData[ 1 ] = (byte) totalGreen;
			pixelData[ 2 ] = (byte) totalBlue;
			pixelData[ 3 ] = (byte) totalAlpha;

			return true;
		}


		static bool Generate( byte[] inData, uint inWidth, uint inHeight, MipSampleMethod inMethod, out List< GeneratedMip > outList )
		{
			outList = new List< GeneratedMip >();

			// Validate all of the parameters
			if( inWidth <= 1 || inHeight <= 1 )
			{
				Core.WriteLine( "[Warning] MipGenerator: Failed to generate mips, width/height was already one or less" );
				return false;
			}
			else if( ( inData?.Length ?? 0 ) != ( inWidth * inHeight * 4 ) )
			{
				Core.WriteLine( "[Warning] MipGenerator: Input data was the wrong length! (", inData.Length, ") for a ", inWidth, "x", inHeight, "px image" );
				return false;
			}

			uint w = inWidth;
			uint h = inHeight;
			bool bPowerOfTwo = IsPowerOfTwo( inWidth ) && IsPowerOfTwo( inHeight );

			while( w > 1 && h > 1 )
			{
				w /= 2;
				h /= 2;

				var newMip = new byte[ w * h * 4 ];
				
				// Loop through each pixel we want to generate, and sample from the source data
				for( uint y = 0; y < h; y++ )
				{
					for( uint x = 0; x < w; x++ )
					{
						byte[] newPixel;

						switch( inMethod )
						{
							case MipSampleMethod.Average:
									
								if( bPowerOfTwo && !PerformPowerOfTwoAverageSample( x, y, w, h, inWidth, inHeight, inData, out newPixel ) ||
										!PerformOddPowerAverageSample( x, y, w, h, inWidth, inHeight, inData, out newPixel ) ||
										( newPixel?.Length ?? 0 ) != 4 )
								{
									Core.WriteLine( "[Warning] MipGenerator: Failed to perform average sample!" );
									outList.Clear();
									return false;
								}

								break;
							default:

								Core.WriteLine( "[Warning] MipGenerator: Unknown mip sample method (", Enum.GetName( typeof( MipSampleMethod ), inMethod ), ")" );
								outList.Clear();
								return false;
						}

						// Copy the calculated pixel into the output data
						Array.ConstrainedCopy( newPixel, 0, newMip, (int)( ( y * w * 4 ) + ( x * 4 ) ), 4 );

					}
				}

				// Take our new mip level and add it to the output list
				outList.Add( new GeneratedMip()
				{
					Data = newMip,
					Width = w,
					Height = h
				} );
			}

			return true;
		}

		

	}
}
