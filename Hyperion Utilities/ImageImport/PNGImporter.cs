using System;
using System.Collections.Generic;
using System.Linq;


namespace Hyperion
{
	static class PNGImporter
	{
		/////// Chunk Structure ////////
		private class PNGChunk
		{
			public uint Length;
			public string Type;
			public byte[] Data;
			public uint CRC;
		}

		//////// Color Types ////////
		private enum ColorType
		{
			GS          = 0b000,
			RGB         = 0b010,
			Indexed     = 0b011,
			GSA         = 0b100,
			RGBA        = 0b110
		}

		//////// Compression Type ////////
		private enum CompressionType
		{
			Deflate = 0
		}

		//////// Filter Type ////////
		private enum FilterType
		{
			Adaptive = 0
		}

		//////// Interlace ////////
		private enum InterlaceType
		{
			None = 0,
			Adam7 = 1
		}

		/////// Image Info /////// [CRITICAL] [MUST BE FIRST]
		private struct IHDR
		{
			public uint Width;
			public uint Height;
			public byte BitDepth;
			public ColorType Color;
			public CompressionType Compression;
			public FilterType Filter;
			public InterlaceType Interlace;
		}

		///////// Palettes /////// [CRITICAL IF INDEXED COLOR TYPE]
		private struct PLTEEntry
		{
			public byte Red;
			public byte Green;
			public byte Blue;
		}

		private struct PLTE
		{
			public List< PLTEEntry > Colors;
		};

		////////// Transparency Chunk ///////////
		private struct tRNsGrayscale
		{
			public UInt16 TransparencyValue;
		};

		private struct tRNsRGB
		{
			public UInt16 TransparencyRed;
			public UInt16 TransparencyGreen;
			public UInt16 TransparencyBlue;
		};

		private struct tRNsIndexed
		{
			public byte[] TransparencyIndexes;
		};

		/////////// Gamma //////////
		private struct gAMA
		{
			public uint GammaValue;
		};

		/////// Line Filter Enum ///////
		private enum LineFilterType
		{
			None = 0,
			Sub = 1,
			Up = 2,
			Avg = 3,
			Paeth = 4
		}

		/////// Pixel Position ///////
		private struct PixelPosition
		{
			public long Byte;
			public byte Bit;
		};

		/////// Raw Pixel ///////
		private struct RawPixel
		{
			public byte r;
			public byte g;
			public byte b;
			public byte a;
		}


		private static bool BuildChunks( byte[] inData, bool bLittleEndian, out List<PNGChunk> outChunks )
		{
			outChunks = new List<PNGChunk>();

			// We need to loop through the input data, as long as there is twelve bytes left in the list basically
			// C++ iterators made this a lot easier, we need a new way to loop through.. maybe a basic for loop would be ideal..
			// Also, we want to start at index 8, since those first 8 bytes are a header signature
			int i = 8; // TODO: Use long instead of int
			for( ; i <= inData.Length - 12; )
			{
				// Dont forget to iterate 'i'
				var newChunk =  new PNGChunk();

				// First, read the length
				newChunk.Length = Serialization.GetUInt32( inData, i, bLittleEndian );
				i += 4;

				// Now, read the type
				var letters = new char[ 4 ];
				letters[ 0 ] = ( char ) Serialization.GetUInt8( inData, i );
				letters[ 1 ] = ( char ) Serialization.GetUInt8( inData, i + 1 );
				letters[ 2 ] = ( char ) Serialization.GetUInt8( inData, i + 2 );
				letters[ 3 ] = ( char ) Serialization.GetUInt8( inData, i + 3 );

				i += 4;
				newChunk.Type = new string( letters );

				// Now, we are going to validate the length
				// The 'iterator' is sitting on the start of the data, and theres a CRC-32 after the data
				// hence the '+ 4', were ensuring theres enough bytes to read the desired data length
				if( newChunk.Length + 4 + i > inData.Length )
				{
					Core.WriteLine( "[Error] Failed to import PNG, chunk lenth was invalid (ran out of data)" );
					return false;
				}

				// Copy data into chunk structure
				newChunk.Data = new byte[ newChunk.Length ];
				Array.Copy( inData, i, newChunk.Data, 0, newChunk.Length );

				i += ( int ) newChunk.Length; // TODO: Need to use long for array indexing..

				// Read the CRC-32 code
				newChunk.CRC = Serialization.GetUInt32( inData, i, bLittleEndian );
				i += 4;

				// Add chunk to list
				outChunks.Add( newChunk );
			}

			if( i != inData.Length )
			{
				Core.WriteLine( "[Warning] During import of PNG, there was data remaining after reading chunks" );
			}

			return true;
		}


		private static bool PerformFirstPassValidation( List<PNGChunk> inChunks )
		{
			// At the very minimum there needs to be 3 chunks
			// IHDR, IDAT, IEND
			if( inChunks.Count < 3 )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG, there wasnt enough chunks in the file" );
				return false;
			}

			bool bHasData = false;
			bool bHasEnd = false;

			// We also need to account for ordering of chunks in the file
			// IHDR has to be the first in all cases 
			for( int i = 0; i < inChunks.Count; i++ )
			{
				if( i == 0 && inChunks[ i ].Type != "IHDR" )
				{
					Core.WriteLine( "[ERROR] Failed to import PNG, first chunk needs to be IHDR!" );
					return false;
				}
				else if( i > 0 )
				{
					if( inChunks[ i ].Type == "IHDR" )
					{
						Core.WriteLine( "[ERROR] Failed to import PNG, there are multiple IHDR chunks" );
						return false;
					}
					else if( inChunks[ i ].Type == "IDAT" )
					{
						bHasData = true;
					}
					else if( inChunks[ i ].Type == "IEND" )
					{
						// Ensure this is the last chunk
						if( i != inChunks.Count - 1 )
						{
							Core.WriteLine( "[ERROR] Failed to import PNG, the last chunk was not the end chunk" );
							return false;
						}

						bHasEnd = true;
					}
				}
			}

			if( !bHasData )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG, missing critical data chunk" );
				return false;
			}

			if( !bHasEnd )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG, missing critical end chunk" );
				return false;
			}

			return true;
		}


		private static bool ReadHeaderChunk( byte[] inData, out IHDR outChunk, bool bLittleEndian )
		{
			outChunk = new IHDR();

			// Ensure we have the proper number of bytes
			if( inData.Length != 13 )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG, header chunk has the wrong number of bytes" );
				return false;
			}

			// Deserialize a couple fields
			outChunk.Width = Serialization.GetUInt32( inData, 0, bLittleEndian );
			outChunk.Height = Serialization.GetUInt32( inData, 4, bLittleEndian );

			// Validate...
			if( outChunk.Width == 0 || outChunk.Height == 0 )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG, read invalid resolution from header chunk" );
				return false;
			}

			// Read bit depth
			outChunk.BitDepth = Serialization.GetUInt8( inData, 8 );
			if( outChunk.BitDepth == 0 || outChunk.BitDepth > 16 )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG, read invalid bit depth from header" );
				return false;
			}

			// Read color type
			byte colorValue = Serialization.GetUInt8( inData, 9 );
			if( !Enum.IsDefined( typeof( ColorType ), (int)colorValue ) )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG, read invalid color type from header" );
				return false;
			}

			outChunk.Color = ( ColorType ) colorValue;

			// Read the compression method
			byte compressionMethodValue = Serialization.GetUInt8( inData, 10 );
			if( compressionMethodValue != 0 )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG, invalid compression method read from header" );
				return false;
			}

			outChunk.Compression = CompressionType.Deflate;

			// Read the filter method
			byte filterMethodValue = Serialization.GetUInt8( inData, 11 );
			if( filterMethodValue != 0 )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG, read invalid filter method from header" );
				return false;
			}

			outChunk.Filter = FilterType.Adaptive;

			// Read the interlace method
			byte interlaceValue = Serialization.GetUInt8( inData, 12 );
			if( interlaceValue > 1 )
			{
				Core.WriteLine( "[ERROR] Failed to impot PNG, read invalid interlace method from header" );
				return false;
			}

			outChunk.Interlace = ( InterlaceType ) interlaceValue;

			// Do some validation of the combination of header values
			if( outChunk.Color == ColorType.GS )
			{
				if( outChunk.BitDepth != 1 && outChunk.BitDepth != 2 &&
					outChunk.BitDepth != 4 && outChunk.BitDepth != 8 &&
					outChunk.BitDepth != 16 )
				{
					Core.WriteLine( "[ERROR] Failed to import PNG, invalid bit depth for single color channel image" );
					return false;
				}
			}
			else if( outChunk.Color != ColorType.Indexed )
			{
				if( outChunk.BitDepth != 8 && outChunk.BitDepth != 16 )
				{
					Core.WriteLine( "[ERROR] Failed to import PNG, invalid bit depth for multi color channel image" );
					return false;
				}
			}
			else
			{
				if( outChunk.BitDepth != 1 && outChunk.BitDepth != 2 &&
					outChunk.BitDepth != 4 && outChunk.BitDepth != 8 )
				{
					Core.WriteLine( "[ERROR] Failed to import PNG, invalid bit depth for indexed image" );
					return false;
				}
			}

			return true;
		}


		private static bool PerformSecondPassValidation( ref IHDR inHeader, List<PNGChunk> inChunks )
		{
			// Check if there is a PLTE chunk
			bool bPLTE = false;
			bool bFoundData = false;

			for( int i = 0; i < inChunks.Count; i++ )
			{
				if( inChunks[ i ].Type == "PLTE" )
				{
					// Check for multiple PLTE chunks
					if( bPLTE )
					{
						Core.WriteLine( "[ERROR] Failed to import PNG image, there are multiple palette chunks" );
						return false;
					}

					// Check if PLTE came after data
					if( bFoundData )
					{
						Core.WriteLine( "[ERROR] Failed to import PNG image, the palette chunk needs to be before the data chunk" );
						return false;
					}

					// Validate the PLTE
					var size = inChunks[ i ].Data.Length;
					if( size < 3 || size > 768 )
					{
						Core.WriteLine( "[ERROR] Failed to import PNG image, the palette chunk is an invalid size" );
						return false;
					}

					bPLTE = true;
				}
				else if( inChunks[ i ].Type == "IDAT" )
				{
					bFoundData = true;
				}
			}

			// For grayscale (with or without alpha), there should be no PLTE
			// And for indexed, there needs to be a PLTE
			if( ( inHeader.Color == ColorType.GS || inHeader.Color == ColorType.GSA ) && bPLTE )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG image, there is a palette chunk but the color type doesnt support one" );
				return false;
			}
			else if( inHeader.Color == ColorType.Indexed && !bPLTE )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG image, there is no palette chunk but the color type requries one" );
				return false;
			}

			return true;
		}


		private static bool ReadPaletteChunk( List<PNGChunk> inChunks, ref IHDR inHeader, out PLTE outPalette )
		{
			outPalette = new PLTE()
			{
				Colors = new List<PLTEEntry>()
			};

			// Check if we can even have a palette in this image type
			if( inHeader.Color == ColorType.GS || inHeader.Color == ColorType.GSA )
			{
				return true;
			}

			// Next, seek out the palette chunk
			int plteIndex = inChunks.Count;
			for( int i = 0; i < inChunks.Count; i++ )
			{
				if( inChunks[ i ].Type == "PLTE" )
				{
					plteIndex = i;
					break;
				}
			}

			if( plteIndex == inChunks.Count )
			{
				// If we didnt find a palette, return false if the color type is indexed
				return inHeader.Color != ColorType.Indexed;
			}

			// Now, we need to read in the PLTE chunk to a structure
			for( int i = 0; i + 3 <= inChunks[ plteIndex ].Data.Length; )
			{
				var entry = new PLTEEntry
				{
					Red = Serialization.GetUInt8( inChunks[ plteIndex ].Data, i ),
					Green = Serialization.GetUInt8( inChunks[ plteIndex ].Data, i + 1 ),
					Blue = Serialization.GetUInt8( inChunks[ plteIndex ].Data, i + 2 )
				};

				outPalette.Colors.Add( entry );
				i += 3;
			}

			return outPalette.Colors.Count > 0 && outPalette.Colors.Count < 257;
		}


		private static void ReadGammaChunk( List<PNGChunk> inChunks, out gAMA outGamma, bool bLittleEndian )
		{
			outGamma = new gAMA();

			// Find the gamma chunk
			int gammaIndex = inChunks.Count;
			for( int i = 0; i < inChunks.Count; i++ )
			{
				if( inChunks[ i ].Type == "gAMA" )
				{
					gammaIndex = i;
					break;
				}
			}

			if( gammaIndex == inChunks.Count )
			{
				return;
			}

			if( inChunks[ gammaIndex ].Data.Length != 4 )
			{
				Core.WriteLine( "[Warning] Issue while importing PNG image, the gamma chunk was invalid" );
				return;
			}

			outGamma.GammaValue = Serialization.GetUInt32( inChunks[ gammaIndex ].Data, 0, bLittleEndian );
		}


		private static object ReadTransparencyChunk( List<PNGChunk> inChunks, ColorType inType, bool bLittleEndian )
		{
			// Check if the color type already includes transparency
			if( inType == ColorType.GSA || inType == ColorType.RGBA )
				return null;

			// Next, we need to find the proper chunk
			int index = inChunks.Count;
			for( int i = 0; i < inChunks.Count; i++ )
			{
				if( inChunks[ i ].Type == "tRNs" )
				{
					index = i;
					break;
				}
			}

			if( index == inChunks.Count )
				return null;

			// The chunk structure is different based on the image color type
			if( inType == ColorType.GS )
			{
				if( inChunks[ index ].Data.Length != 2 )
				{
					Core.WriteLine( "[ERROR] Failed to import PNG, transparency chunk invalid" );
					return null;
				}

				// Read the chunk in
				return new tRNsGrayscale()
				{
					TransparencyValue = Serialization.GetUInt16( inChunks[ index ].Data, 0, bLittleEndian )
				};
			}
			else if( inType == ColorType.RGB )
			{
				if( inChunks[ index ].Data.Length != 6 )
				{
					Core.WriteLine( "[ERROR] Failed to import PNG, transparnecy chunk invalid" );
					return null;
				}

				// Read the chunk in
				return new tRNsRGB()
				{
					TransparencyRed = Serialization.GetUInt16( inChunks[ index ].Data, 0, bLittleEndian ),
					TransparencyGreen = Serialization.GetUInt16( inChunks[ index ].Data, 2, bLittleEndian ),
					TransparencyBlue = Serialization.GetUInt16( inChunks[ index ].Data, 4, bLittleEndian )
				};
			}
			else if( inType == ColorType.Indexed )
			{
				var dataLen = inChunks[ index ].Data.Length;

				if( dataLen <= 0 )
				{
					Core.WriteLine( "[ERROR] Failed to import PNG, transparency chunk invalid" );
					return null;
				}

				var trnsChunk = new tRNsIndexed()
				{
					TransparencyIndexes = new byte[ dataLen ]
				};

				Array.Copy( inChunks[ index ].Data, trnsChunk.TransparencyIndexes, dataLen );
				return trnsChunk;
			}
			else return null;
		}


		private static bool AccumulateImageData( List<PNGChunk> inChunks, out byte[] outData )
		{
			outData = null;

			// We want to loop through and find all data sections, and accumulate them into a single byte array
			// But, we loop through the chunks twice, once to calculate the size of the output array, and once to copy data into the new array
			long finalLen = 0;

			for( int i = 0; i < inChunks.Count; i++ )
			{
				if( inChunks[ i ].Type == "IDAT" )
				{
					finalLen += inChunks[ i ].Data.LongLength;
				}
			}

			outData = new byte[ finalLen ];
			long copyOffset = 0;

			for( int i = 0; i < inChunks.Count; i++ )
			{
				if( inChunks[ i ].Type == "IDAT" )
				{
					var chunkLength = inChunks[ i ].Data.LongLength;
					Array.Copy( inChunks[ i ].Data, 0L, outData, copyOffset, chunkLength );

					copyOffset += chunkLength;
					inChunks[ i ].Data = null;
				}
			}

			// Ensure we have the data
			if( outData.Length <= 0 )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG, couldnt find image data!" );
				return false;
			}

			return true;
		}


		private static byte GetPixelSizeBits( ref IHDR inHeader )
		{
			byte channelSize = inHeader.BitDepth;

			switch( inHeader.Color )
			{
				case ColorType.GSA:
					return ( byte ) ( channelSize * 2 );
				case ColorType.RGB:
					return ( byte ) ( channelSize * 3 );
				case ColorType.RGBA:
					return ( byte ) ( channelSize * 4 );
				case ColorType.GS:
				case ColorType.Indexed:
				default:
					return channelSize;
			}
		}


		private static uint RoundUpBitsToBytes( uint inBits )
		{
			uint roundedBits = 0;
			if( inBits % 8 == 0 )
			{
				roundedBits = inBits;
			}
			else
			{
				roundedBits = ( 8 - inBits % 8 ) + inBits;
			}

			return roundedBits / 8;
		}


		private static byte PaethPredictor( byte a, byte b, byte c )
		{
			short p     = (short)( a + b - c );
			short pa_i  = (short)( p - a );
			byte pa     = (byte)( pa_i < 0 ? -pa_i : pa_i );
			short pb_i  = (short)( p - b );
			byte pb     = (byte)( pb_i < 0 ? -pb_i : pb_i );
			short pc_i  = (short)( p - c );
			byte pc     = (byte)( pc_i < 0 ? -pc_i : pc_i );

			if( pa <= pb && pa <= pc )
			{
				return a;
			}
			else if( pb <= pc )
			{
				return b;
			}
			else
			{
				return c;
			}
		}


		private static bool DefilterImageData( byte[] inData, ref IHDR inHeader )
		{
			// We need to calculate a few things before we start defiltering
			byte channelCount;
			switch( inHeader.Color )
			{
				case ColorType.GS:
				case ColorType.Indexed:
					channelCount = 1;
					break;
				case ColorType.GSA:
					channelCount = 2;
					break;
				case ColorType.RGB:
					channelCount = 3;
					break;
				case ColorType.RGBA:
					channelCount = 4;
					break;
				default:
					return false;
			}

			uint pixelSizeBits          = (uint) inHeader.BitDepth * channelCount;
			uint lineSizeBits           = 8 + ( pixelSizeBits * inHeader.Width );
			uint lineSizeBytesCeil      = RoundUpBitsToBytes( lineSizeBits );
			uint pixelSizeBytesCeil     = RoundUpBitsToBytes( pixelSizeBits );
			long expectedSize           = lineSizeBytesCeil * inHeader.Height;

			// Validate the array size
			if( inData.LongLength < expectedSize )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG, defiltering failed, input data was invaild" );
				return false;
			}

			// Now, we need to loop through the source data line by line
			for( uint y = 0; y < inHeader.Height; y++ )
			{
				long lineStartIndex = y * lineSizeBytesCeil;
				byte filterByte = inData[ lineStartIndex ];

				if( !Enum.IsDefined( typeof( LineFilterType ), (int) filterByte ) )
				{
					Core.WriteLine( "[ERROR] Failed to import PNG, defiltering failed, line had invalid filter type" );
					return false;
				}

				var filter = (LineFilterType) filterByte;

				// Now, loop through each byte in the line, just skip the filter byte
				for( uint x = 0; x < lineSizeBytesCeil - 1; x++ )
				{
					long thisIndex = lineStartIndex + x + 1;
					byte thisByte = inData[ thisIndex ];

					// Process filter type
					if( filter == LineFilterType.Sub )
					{
						// Sub( x ) = Raw( x ) - Raw( x  - bpp ) where x is the byte position in the scanline & bpp is the number of bytes per pixel, rounded up to 1
						// Therefore:
						// Raw( x ) = Sub( x ) + Raw( x - bpp )
						// If we are the first pixel in the scanline, the prev is zero

						byte rawPrev = 0;
						if( x >= pixelSizeBytesCeil )
						{
							rawPrev = inData[ thisIndex - pixelSizeBytesCeil ];
						}

						inData[ thisIndex ] = ( byte ) ( thisByte + rawPrev );
					}
					else if( filter == LineFilterType.Up )
					{
						// Up( x ) = Raw( x ) - Prior( x )
						// So.. Raw( x ) = Up( x ) + Prior( x ) where Prior( x ) is the same byte on the previous scanline
						// If were on the first scanline, prior is 0

						byte rawPrior = 0;
						if( y > 0 )
						{
							rawPrior = inData[ thisIndex - lineSizeBytesCeil ];
						}

						inData[ thisIndex ] = ( byte ) ( thisByte + rawPrior );
					}
					else if( filter == LineFilterType.Avg )
					{
						// Avg( x ) = Raw( x ) - floor( ( Raw( x - bpp ) + Prior( x ) ) / 2 )
						// Raw( x ) = Avg( x ) + floor( ( Raw( x - bpp ) + Prior( x ) ) / 2 )
						byte rawPrev = 0;
						byte rawPrior = 0;

						if( x >= pixelSizeBytesCeil )
						{
							rawPrev = inData[ thisIndex - pixelSizeBytesCeil ];
						}

						if( y > 0 )
						{
							rawPrior = inData[ thisIndex - lineSizeBytesCeil ];
						}

						inData[ thisIndex ] = ( byte ) ( thisByte + ( ( rawPrev + rawPrior ) / 2 ) );
					}
					else if( filter == LineFilterType.Paeth )
					{
						// Paeth( x ) = Raw( x ) - PaethPredictor( Raw( x - bpp ), Prior( x ), Prior( x - bpp ) )
						// Raw( x ) = Paeth( x ) + PaethPredictor( Raw( x - bpp ), Prior( x ), Prior( x - bpp ) )
						byte rawPrev = 0;
						byte rawPrior = 0;
						byte rawPrevPrior = 0;

						if( x >= pixelSizeBytesCeil )
						{
							rawPrev = inData[ thisIndex - pixelSizeBytesCeil ];
							if( y > 0 )
							{
								rawPrevPrior = inData[ thisIndex - lineSizeBytesCeil - pixelSizeBytesCeil ];
							}
						}

						if( y > 0 )
						{
							rawPrior = inData[ thisIndex - lineSizeBytesCeil ];
						}

						byte pp = PaethPredictor( rawPrev, rawPrior, rawPrevPrior );
						inData[ thisIndex ] = ( byte ) ( thisByte + pp );
					}
					else
					{
						// No filter!
					}
				}
			}

			return true;
		}


		private static PixelPosition GetPixelPosition( uint x, uint y, byte pixelSizeBits, uint lineSizeBytes, byte offsetBits )
		{
			// Figure out where the start of the line is in the source data
			// And then, figure out how far in the line the pixel is
			long lineStart          = y * lineSizeBytes;
			uint pixelStartOffset   = x * pixelSizeBits + 8 + offsetBits;

			// Turn the pixel start offset into bytes + bits
			uint pixelStartBytes    = pixelStartOffset / 8;
			byte pixelStartBits     = (byte)( pixelStartOffset % 8 );

			// Add line offset so we have the final position
			return new PixelPosition()
			{
				Byte = lineStart + pixelStartBytes,
				Bit = pixelStartBits
			};
		}


		private static bool ReadSingleBitChannel( byte inByteVal, byte inBit, out byte outValue )
		{
			outValue = 0;
			byte val = 0;

			switch( inBit )
			{
				case 0:
					val = ( byte ) ( inByteVal & 0b00000001 );
					break;
				case 1:
					val = ( byte ) ( inByteVal & 0b00000010 );
					break;
				case 2:
					val = ( byte ) ( inByteVal & 0b00000100 );
					break;
				case 3:
					val = ( byte ) ( inByteVal & 0b00001000 );
					break;
				case 4:
					val = ( byte ) ( inByteVal & 0b00010000 );
					break;
				case 5:
					val = ( byte ) ( inByteVal & 0b00100000 );
					break;
				case 6:
					val = ( byte ) ( inByteVal & 0b01000000 );
					break;
				case 7:
					val = ( byte ) ( inByteVal & 0b10000000 );
					break;
				default:
					Core.WriteLine( "[ERROR] Failed to import PNG, couldnt read single bit channel, offset invalid" );
					return false;
			}

			outValue = ( byte ) ( val >> ( 7 - inBit ) );
			return true;
		}


		private static bool ReadDoubleBitChannel( byte inByteVal, byte inStartBit, out byte outVal )
		{
			outVal = 0;
			byte val = 0;

			switch( inStartBit )
			{
				case 0:
					val = ( byte ) ( inByteVal & 0b00000011 );
					break;
				case 2:
					val = ( byte ) ( inByteVal & 0b00001100 );
					break;
				case 4:
					val = ( byte ) ( inByteVal & 0b00110000 );
					break;
				case 6:
					val = ( byte ) ( inByteVal & 0b11000000 );
					break;
				default:
					Core.WriteLine( "[ERROR] Failed to import PNG, couldnt read double bit channel, offset invalid" );
					return false;
			}

			outVal = ( byte ) ( val >> ( 6 - inStartBit ) );
			return true;
		}


		private static bool ReadQuadBitChannel( byte inByteVal, byte inStartBit, out byte outVal )
		{
			outVal = 0;
			byte val = 0;

			switch( inStartBit )
			{
				case 0:
					val = ( byte ) ( inByteVal & 0b00001111 );
					break;
				case 4:
					val = ( byte ) ( inByteVal & 0b11110000 );
					break;
				default:
					Core.WriteLine( "[ERROR] Failed to import PNG, couldnt read quad bit channel, invalid offset" );
					return false;
			}

			outVal = ( byte ) ( val >> ( 4 - inStartBit ) );
			return true;
		}


		private static bool ReadEightBitChannel( byte inByteVal, out byte outVal )
		{
			outVal = inByteVal;
			return true;
		}


		private static bool ReadSixteenBitChannelFull( byte firstByteVal, byte secByteVal, out ushort outVal, bool bLittleEndian )
		{
			// We need to take byte order into account
			byte upper = 0;
			byte lower = 0;

			if( bLittleEndian )
			{
				upper = secByteVal;
				lower = firstByteVal;
			}
			else
			{
				upper = firstByteVal;
				lower = secByteVal;
			}

			outVal = ( ushort ) ( upper << 8 );
			outVal |= lower;

			return true;
		}


		private static bool ReadChannelAs16Bit( byte[] inData, long inOffset, byte inBitDepth, byte inStartBit, bool bLittleEndian, out ushort outVal )
		{
			outVal = 0;

			if( inBitDepth <= 8 )
			{
				byte tempVal = 0;
				switch( inBitDepth )
				{
					case 1:
						if( !ReadSingleBitChannel( inData[ inOffset ], inStartBit, out tempVal ) ) return false;
						break;
					case 2:
						if( !ReadDoubleBitChannel( inData[ inOffset ], inStartBit, out tempVal ) ) return false;
						break;
					case 4:
						if( !ReadQuadBitChannel( inData[ inOffset ], inStartBit, out tempVal ) ) return false;
						break;
					case 8:
						if( inStartBit != 0 || !ReadEightBitChannel( inData[ inOffset ], out tempVal ) ) return false;
						break;
					default:
						return false;
				}

				outVal = tempVal;
				return true;
			}
			else if( inBitDepth == 16 )
			{
				if( inStartBit != 0 || !ReadSixteenBitChannelFull( inData[ inOffset ], inData[ inOffset + 1 ], out outVal, bLittleEndian ) ) return false;
				return true;
			}
			else return false;
		}


		private static byte ScaleSample( ushort inSample, byte inBitDepth )
		{
			switch( inBitDepth )
			{
				case 1:
					if( inSample > 1 )
					{
						Core.WriteLine( "[Warning] During PNG import, a sample was out of range during scale function.. clamped to bit depth of 1" );
						inSample = 1;
					}

					return ( byte ) ( inSample * 255 ); // Original Range 0 to 1.. new range 0 to 255

				case 2:
					if( inSample > 3 )
					{
						Core.WriteLine( "[Warning] During PNG import, a sample was out of range during scale function.. clamped to bit depth of 2" );
						inSample = 3;
					}

					return ( byte ) ( inSample * 85 );

				case 4:
					if( inSample > 15 )
					{
						Core.WriteLine( "[Warning] During PNG import, a sample was out of range during scale function.. clamped to bit depth of 4" );
						inSample = 15;
					}

					return ( byte ) ( inSample * 17 );

				case 8:
					if( inSample > 255 )
					{
						Core.WriteLine( "[Warning] During PNG import, a sample was out of range during scale function.. clamped to bit depth of 8" );
						inSample = 255;
					}

					return ( byte ) inSample;

				case 16:
					return ( byte ) ( inSample / 257 );

				default:
					Core.WriteLine( "[Warning] During PNG import, a sample couldnt be scaled due to an invalid bit depth" );
					return 0;
			}
		}


		private static void ApplyGamma( ref RawPixel inPixel, uint inGamma, float inScreenGamma )
		{
			// TODO: Need to work on gamma system
		}


		private static void WritePixel( ref RawPixel inPixel, long inOffset, ref RawImageData refData )
		{
			refData.Data[ inOffset ] = inPixel.r;
			refData.Data[ inOffset + 1 ] = inPixel.g;
			refData.Data[ inOffset + 2 ] = inPixel.b;
			refData.Data[ inOffset + 3 ] = inPixel.a;
		}


		/*-----------------------------------------------------------------------------------------------
		 *	Process Grayscale Images 
		-----------------------------------------------------------------------------------------------*/
		private static bool ProcessGrayscaleImage( byte[] inData, ref object inTransparencyChunk, ref IHDR inHeader, uint inGamma, out RawImageData outData, bool bLittleEndian, float inScreenGamma )
		{
			outData = new RawImageData();

			byte pixelBits      = inHeader.BitDepth;
			uint lineSizeBytes  = RoundUpBitsToBytes( 8+ inHeader.Width * pixelBits );

			// Set output image size
			outData = new RawImageData()
			{
				Width = inHeader.Width,
				Height = inHeader.Height,
				Data = new byte[ inHeader.Width * inHeader.Height * 4 ]
			};

			bool bApplyTransparency     = false;
			ushort transparencyValue    = 0;

			// Process transparency chunk
			if( inTransparencyChunk != null )
			{
				if( inTransparencyChunk.GetType() != typeof( tRNsGrayscale ) )
				{
					Core.WriteLine( "[ERROR] Failed to import PNG, transparency chunk wrong type" );
					return false;
				}

				var castedChunk     = (tRNsGrayscale) inTransparencyChunk;
				bApplyTransparency = true;
				transparencyValue = castedChunk.TransparencyValue;
			}

			for( uint y = 0; y < inHeader.Height; y++ )
			{
				for( uint x = 0; x < inHeader.Width; x++ )
				{
					// Read the input pixel, possible bit depths are 1, 2, 4, 8, or 16
					var memPos = GetPixelPosition( x, y, pixelBits, lineSizeBytes, 0 );

					// The largest possible channel size is 16-bit, so we will read it into a uint16 for now, then check against transparency
					ushort val;
					if( !ReadChannelAs16Bit( inData, memPos.Byte, inHeader.BitDepth, memPos.Bit, bLittleEndian, out val ) )
					{
						Core.WriteLine( "[ERROR] Failed to import PNG, couldnt read grayscale channel" );
						return false;
					}

					// Check for transparency
					bool bIsTransparent = ( bApplyTransparency && transparencyValue == val );

					// Scale sample to uint8 range, based on the bit depth of the source image
					byte scaledSample = ScaleSample( val, inHeader.BitDepth );

					var newPixel = new RawPixel()
					{
						r = scaledSample,
						g = scaledSample,
						b = scaledSample,
						a = bIsTransparent ? (byte)0 : (byte)255
					};

					// Apply gamma and write to the output
					// We also need to figure out where we should write in the output array
					// Which is, ( ( y * width ) + x ) * 4
					ApplyGamma( ref newPixel, inGamma, inScreenGamma );
					WritePixel( ref newPixel, ( ( y * inHeader.Width ) + x ) * 4, ref outData );
				}
			}

			Core.WriteLine( "=> PNGImporter: Processed grayscale image (no alpha) [", inHeader.Width, "x", inHeader.Height, "] and the output was ", outData.Data.LongLength / 1024L, "kb" );
			return true;
		}


		/*------------------------------------------------------------------------------------
		 *	Process Grayscale + Alpha Image
		------------------------------------------------------------------------------------*/
		private static bool ProcessGrayscaleAlphaImage( byte[] inData, ref IHDR inHeader, uint inGamma, out RawImageData outData, bool bLittleEndian, float inScreenGamma )
		{
			// Calculate some values needed
			byte pixelBits      = (byte)( inHeader.BitDepth * 2 );
			uint lineSizeBytes  = RoundUpBitsToBytes( 8 + inHeader.Width * pixelBits );

			// Set output image size
			outData = new RawImageData()
			{
				Width = inHeader.Width,
				Height = inHeader.Height,
				Data = new byte[ inHeader.Width * inHeader.Height * 4 ]
			};

			// Loop through each pixel in source data
			for( uint y = 0; y < inHeader.Height; y++ )
			{
				for( uint x = 0; x < inHeader.Width; x++ )
				{
					// Find and read the input pixel, and store each channel as an unsigned 16 bit integer
					var grayOffset      = GetPixelPosition( x, y, pixelBits, lineSizeBytes, 0 );
					var alphaOffset     = GetPixelPosition( x, y, pixelBits, lineSizeBytes, inHeader.BitDepth );

					ushort gray = 0;
					ushort alpha = 0;

					if( !ReadChannelAs16Bit( inData, grayOffset.Byte, inHeader.BitDepth, grayOffset.Bit, bLittleEndian, out gray ) ||
						!ReadChannelAs16Bit( inData, alphaOffset.Byte, inHeader.BitDepth, alphaOffset.Bit, bLittleEndian, out alpha ) )
					{
						Core.WriteLine( "[ERROR] Failed to import PNG, couldnt read grayscale & alpha pixel" );
						return false;
					}

					// Scale the values read back to 8-bit unsigned integers
					byte graySample     = ScaleSample( gray, inHeader.BitDepth );
					byte alphaSample    = ScaleSample( alpha, inHeader.BitDepth );

					// Create pixel structure and apply gamma
					var newPixel = new RawPixel()
					{
						r = graySample,
						g = graySample,
						b = graySample,
						a = alphaSample
					};

					ApplyGamma( ref newPixel, inGamma, inScreenGamma );
					WritePixel( ref newPixel, ( ( y * inHeader.Width ) + x ) * 4, ref outData );
				}
			}

			Core.WriteLine( "=> PNGImporter: Processed grayscale image (w/ alpha) [", inHeader.Width, "x", inHeader.Height, "] and the output was ", outData.Data.LongLength / 1024L, "kb" );
			return true;
		}


		/*----------------------------------------------------------------------------------------------
		 *	Process RGB (Truecolor) Image
		----------------------------------------------------------------------------------------------*/
		private static bool ProcessRGBImage( byte[] inData, ref object inTransparencyChunk, ref IHDR inHeader, uint inGamma, out RawImageData outData, bool bLittleEndian, float inScreenGamma )
		{
			// Calculate needed values
			byte pixelBits      = (byte)( inHeader.BitDepth * 3 );
			uint lineSizeBytes  = RoundUpBitsToBytes( 8 + inHeader.Width * pixelBits );

			// Set output image size
			outData = new RawImageData()
			{
				Width = inHeader.Width,
				Height = inHeader.Height,
				Data = new byte[ inHeader.Width * inHeader.Height * 4 ]
			};

			// Retrieve transparency values
			bool bApplyTransparency = false;
			ushort tR = 0;
			ushort tG = 0;
			ushort tB = 0;

			if( inTransparencyChunk != null )
			{
				if( inTransparencyChunk.GetType() != typeof( tRNsRGB ) )
				{
					Core.WriteLine( "[Warning] Issue while importing PNG, the transparency chunk was invalid (wrong type?), so transparency will not be applied" );
				}
				else
				{
					var castedTransparency = (tRNsRGB) inTransparencyChunk;
					tR = castedTransparency.TransparencyRed;
					tG = castedTransparency.TransparencyGreen;
					tB = castedTransparency.TransparencyBlue;

					bApplyTransparency = true;
				}
			}

			// Loop through each pixel in source data
			for( uint y = 0; y < inHeader.Height; y++ )
			{
				for( uint x = 0; x < inHeader.Width; x++ )
				{
					// Get the location of each channel in memory
					var redOffset       = GetPixelPosition( x, y, pixelBits, lineSizeBytes, 0 );
					var greenOffset     = GetPixelPosition( x, y, pixelBits, lineSizeBytes, inHeader.BitDepth );
					var blueOffset      = GetPixelPosition( x, y, pixelBits, lineSizeBytes, (byte)( inHeader.BitDepth * 2 ) );

					// Read in each channel as a unsigned 16-bit integer
					ushort r, g, b;
					if( !ReadChannelAs16Bit( inData, redOffset.Byte, inHeader.BitDepth, redOffset.Bit, bLittleEndian, out r ) ||
						!ReadChannelAs16Bit( inData, greenOffset.Byte, inHeader.BitDepth, greenOffset.Bit, bLittleEndian, out g ) ||
						!ReadChannelAs16Bit( inData, blueOffset.Byte, inHeader.BitDepth, blueOffset.Bit, bLittleEndian, out b ) )
					{
						Core.WriteLine( "[ERROR] Failed to import PNG, couldnt read RGB pixel channels" );
						return false;
					}

					// Check for transparency
					bool bIsTransparent = ( bApplyTransparency && tR == r && tG == g && tB == b );

					// Create the raw pixel structure, and scale samples to the uint8 range
					var newPixel = new RawPixel()
					{
						r = ScaleSample( r, inHeader.BitDepth ),
						g = ScaleSample( g, inHeader.BitDepth ),
						b = ScaleSample( b, inHeader.BitDepth ),
						a = bIsTransparent ? (byte)0 : (byte)255
					};

					// Apply gamma processing, then finally write the pixel to the output
					ApplyGamma( ref newPixel, inGamma, inScreenGamma );
					WritePixel( ref newPixel, ( ( y * inHeader.Width ) + x ) * 4, ref outData );
				}
			}

			Core.WriteLine( "=> PNGImporter: Processed RGB image [", inHeader.Width, "x", inHeader.Height, "] and the output was ", outData.Data.LongLength / 1024L, "kb" );
			return true;
		}


		/*--------------------------------------------------------------------------------------------------------
		 *	Process RGBA Image
		--------------------------------------------------------------------------------------------------------*/
		private static bool ProcessRGBAImage( byte[] inData, ref IHDR inHeader, uint inGamma, out RawImageData outData, bool bLittleEndian, float inScreenGamma )
		{
			// Calculate needed values
			byte pixelBits      = (byte)( inHeader.BitDepth * 4 );
			uint lineSizeBytes  = RoundUpBitsToBytes( 8 + inHeader.Width * pixelBits );

			// Set output image size
			outData = new RawImageData()
			{
				Width = inHeader.Width,
				Height = inHeader.Height,
				Data = new byte[ inHeader.Width * inHeader.Height * 4 ]
			};

			// Loop through each pixel in source data
			for( uint y = 0; y < inHeader.Height; y++ )
			{
				for( uint x = 0; x < inHeader.Width; x++ )
				{
					// Calculate offset for each channel
					var redOffset       = GetPixelPosition( x, y, pixelBits, lineSizeBytes, 0 );
					var greenOffset     = GetPixelPosition( x, y, pixelBits, lineSizeBytes, inHeader.BitDepth );
					var blueOffset      = GetPixelPosition( x, y, pixelBits, lineSizeBytes, (byte)( inHeader.BitDepth * 2 ) );
					var alphaOffset     = GetPixelPosition( x, y, pixelBits, lineSizeBytes, (byte)( inHeader.BitDepth * 3 ) );

					// Read in all channels as unsigned 16 bit values
					ushort r, g, b, a;
					if( !ReadChannelAs16Bit( inData, redOffset.Byte, inHeader.BitDepth, redOffset.Bit, bLittleEndian, out r ) ||
						!ReadChannelAs16Bit( inData, greenOffset.Byte, inHeader.BitDepth, greenOffset.Bit, bLittleEndian, out g ) ||
						!ReadChannelAs16Bit( inData, blueOffset.Byte, inHeader.BitDepth, blueOffset.Bit, bLittleEndian, out b ) ||
						!ReadChannelAs16Bit( inData, alphaOffset.Byte, inHeader.BitDepth, alphaOffset.Bit, bLittleEndian, out a ) )
					{
						Core.WriteLine( "[ERROR] Failed to import PNG, couldnt read RGBA pixel channels" );
						return false;
					}

					// Create raw pixel, and scale samples to uint8 range
					var newPixel = new RawPixel()
					{
						r = ScaleSample( r, inHeader.BitDepth ),
						g = ScaleSample( g, inHeader.BitDepth ),
						b = ScaleSample( b, inHeader.BitDepth ),
						a = ScaleSample( a, inHeader.BitDepth )
					};

					// Apply gamma and write pixel to the output
					ApplyGamma( ref newPixel, inGamma, inScreenGamma );
					WritePixel( ref newPixel, ( ( y * inHeader.Width ) + x ) * 4, ref outData );
				}
			}

			Core.WriteLine( "=> PNGImporter: Processed RGBA image [", inHeader.Width, "x", inHeader.Height, "] and the output was ", outData.Data.LongLength / 1024L, "kb" );
			return true;
		}


		/*---------------------------------------------------------------------------------------
		 *	Process Indexed Image
		---------------------------------------------------------------------------------------*/
		private static bool ProcessIndexedImage( byte[] inData, ref PLTE inPalette, ref object inTransparency, ref IHDR inHeader, uint inGamma, out RawImageData outData, bool bLittleEndian, float inScreenGamma )
		{
			// Calculate needed values
			byte pixelBits      = inHeader.BitDepth;
			uint lineSizeBytes  = RoundUpBitsToBytes( 8 + inHeader.Width * pixelBits );

			// Setup output structure
			outData = new RawImageData()
			{
				Width = inHeader.Width,
				Height = inHeader.Height,
				Data = new byte[ inHeader.Width * inHeader.Height * 4 ]
			};

			// Read in transparency data
			byte[] transparencyIndexList = null;

			if( inTransparency != null )
			{
				if( inTransparency.GetType() != typeof( tRNsIndexed ) )
				{
					Core.WriteLine( "[Warning] Issue while importing PNG, transparency chunk has the wrong type (for indexed image), so transparency wont be applied" );
				}
				else
				{
					var trns = (tRNsIndexed) inTransparency;
					transparencyIndexList = trns.TransparencyIndexes;
				}
			}

			// Loop through each pixel in source data
			for( uint y = 0; y < inHeader.Height; y++ )
			{
				for( uint x = 0; x < inHeader.Width; x++ )
				{
					// Calculate channel offset in memory
					var memOffset = GetPixelPosition( x, y, pixelBits, lineSizeBytes, 0 );

					// Read the index in
					ushort index;
					if( !ReadChannelAs16Bit( inData, memOffset.Byte, inHeader.BitDepth, memOffset.Bit, bLittleEndian, out index ) )
					{
						Core.WriteLine( "[ERROR] Failed to import PNG, couldnt read indexed pixel in" );
						return false;
					}

					// Check for transparency
					var newPixel = new RawPixel();
					newPixel.a = 255;

					if( transparencyIndexList != null && transparencyIndexList.Length > index )
					{
						newPixel.a = transparencyIndexList.ElementAt( index );
					}

					// Lookup the rest of the color
					if( inPalette.Colors.Count <= index )
					{
						Core.WriteLine( "[Warning] Issue while importing PNG, image is indexed and the index is out of range.. this pixel will default to black" );

						newPixel.r = 0;
						newPixel.g = 0;
						newPixel.b = 0;
					}
					else
					{
						newPixel.r = inPalette.Colors[ index ].Red;
						newPixel.g = inPalette.Colors[ index ].Green;
						newPixel.b = inPalette.Colors[ index ].Blue;
					}

					// Apply gamma and write to output
					ApplyGamma( ref newPixel, inGamma, inScreenGamma );
					WritePixel( ref newPixel, ( ( y * inHeader.Width ) + x ) * 4, ref outData );
				}
			}

			Core.WriteLine( "=> PNGImporter: Processed indexed image [", inHeader.Width, "x", inHeader.Height, "] and the output was ", outData.Data.LongLength / 1024L, "kb" );
			return true;
		}

		/*-----------------------------------------------------------------------------------
		 *	Load From Memory
		-----------------------------------------------------------------------------------*/
		public static bool Import( byte[] inData, float inScreenGamma, out RawImageData outData )
		{
			outData = new RawImageData()
			{
				Width = 0,
				Height = 0,
				Data = null,
				Format = ImageFormat.NONE
			};

			if( ( inData?.LongLength ?? 0L ) < 57L )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG, source data was not long enough" );
				return false;
			}

			// Read the first 8 bytes into the file signature
			var headerBytes     = Serialization.GetUInt64( inData, 0, false );
			bool bLittleEndian  = false;

			if( headerBytes == 0x89504E470D0A1A0A )
			{
				// Byte order is big endian, and the header was found
			}
			else if( headerBytes == 0x0A1A0A0D474E5089 )
			{
				// Byte order is little endian, and the header was found
				bLittleEndian = true;
			}
			else
			{
				// Header was not found!
				Core.WriteLine( "[ERROR] Failed to import PNG, header was invalid!" );
				return false;
			}

			// Next, we need to break the file out into the chunks
			if( !BuildChunks( inData, bLittleEndian, out List<PNGChunk> Chunks ) )
			{
				return false;
			}

			// Now, were going to perform the first validation pass
			if( !PerformFirstPassValidation( Chunks ) )
			{
				return false;
			}

			// Now we need to read in the header chunk
			if( !ReadHeaderChunk( Chunks[ 0 ].Data, out IHDR Header, bLittleEndian ) )
			{
				return false;
			}

			// Null out the header chunk data, so the GC can collect it
			Chunks[ 0 ].Data = null;

			// Perform a second pass validation now that we have the header data
			if( !PerformSecondPassValidation( ref Header, Chunks ) )
			{
				return false;
			}

			// Now read in the palette chunk
			if( !ReadPaletteChunk( Chunks, ref Header, out PLTE Palette ) )
			{
				return false;
			}

			// Now, were going to read in optional chunks that affect the image
			// First, the gamma chunk
			var gammaChunk = new gAMA()
			{
				GammaValue = 100000
			};

			ReadGammaChunk( Chunks, out gammaChunk, bLittleEndian );

			// Next, the transparency chunk
			var transparencyChunk = ReadTransparencyChunk( Chunks, Header.Color, bLittleEndian );

			// Now we have to read in the image data, and accumulate it into a single buffer
			if( !AccumulateImageData( Chunks, out byte[] ImageData ) )
			{
				return false;
			}

			// Now we have to decompress the image data using the inflate algorithm
			if( !Deflate.PerformInflate( ImageData, out byte[] DecompressedData ) )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG, couldnt decompress image data" );
				return false;
			}

			ImageData = null;

			// Defilter the image
			if( !DefilterImageData( DecompressedData, ref Header ) )
			{
				Core.WriteLine( "[ERROR] Failed to import PNG, couldnt defilter the image data" );
				return false;
			}

			// Now, perform the main read based on the color type
			switch( Header.Color )
			{
				case ColorType.GS:

					if( !ProcessGrayscaleImage( DecompressedData, ref transparencyChunk, ref Header, gammaChunk.GammaValue, out outData, bLittleEndian, inScreenGamma ) )
					{
						outData = null;
						return false;
					}

					outData.Format = ImageFormat.GS;
					break;

				case ColorType.GSA:

					if( !ProcessGrayscaleAlphaImage( DecompressedData, ref Header, gammaChunk.GammaValue, out outData, bLittleEndian, inScreenGamma ) )
					{
						outData = null;
						return false;
					}

					outData.Format = ImageFormat.GSA;
					break;

				case ColorType.RGB:

					if( !ProcessRGBImage( DecompressedData, ref transparencyChunk, ref Header, gammaChunk.GammaValue, out outData, bLittleEndian, inScreenGamma ) )
					{
						outData = null;
						return false;
					}

					outData.Format = ImageFormat.RGB;
					break;

				case ColorType.RGBA:

					if( !ProcessRGBAImage( DecompressedData, ref Header, gammaChunk.GammaValue, out outData, bLittleEndian, inScreenGamma ) )
					{
						outData = null;
						return false;
					}

					outData.Format = ImageFormat.RGBA;
					break;

				case ColorType.Indexed:

					if( !ProcessIndexedImage( DecompressedData, ref Palette, ref transparencyChunk, ref Header, gammaChunk.GammaValue, out outData, bLittleEndian, inScreenGamma ) )
					{
						outData = null;
						return false;
					}

					outData.Format = ImageFormat.RGB;
					break;

				default:

					Core.WriteLine( "[ERROR] Failed to import PNG, invalid color type??" );
					return false;
			}

			return true;
		}

	}
}