/*==================================================================================================
	Hyperion Engine
	Source/Tools/PNGReader.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Tools/PNGReader.h"
#include "Hyperion/Core/Library/Binary.h"
#include "Hyperion/Tools/Deflate.h"

#include <any>


namespace Hyperion
{
namespace Tools
{

	/////// Chunk Structure ////////
	struct PNGChunk
	{
		uint32 Length;
		std::string Type;
		std::vector< uint8 > Data;
		uint32 CRC;
	};

	//////// Color Types ////////
	enum class ColorType
	{
		GS			= 0b000,
		RGB			= 0b010,
		Indexed		= 0b011,
		GSA			= 0b100,
		RGBA		= 0b110
	};

	//////// Compression Type ////////
	enum class CompressionType
	{
		Deflate = 0
	};

	//////// Filter Type ////////
	enum class FilterType
	{
		Adaptive = 0
	};

	//////// Interlace ////////
	enum class InterlaceType
	{
		None = 0,
		Adam7 = 1
	};

	/////// Image Info /////// [CRITICAL] [MUST BE FIRST]
	struct IHDR
	{
		uint32 Width;
		uint32 Height;
		uint8 BitDepth;
		ColorType Color;
		CompressionType Compression;
		FilterType Filter;
		InterlaceType Interlace;
	};

	///////// Palettes /////// [CRITICAL IF INDEXED COLOR TYPE]
	struct PLTEEntry
	{
		uint8 Red;
		uint8 Green;
		uint8 Blue;
	};

	struct PLTE
	{
		std::vector< PLTEEntry > Colors;
	};

	//////// IDAT //////// [CRITICAL] [IMAGE DATA]
	struct IDAT
	{
		std::vector< uint8 > Data;
	};

	//////// IEND //////// [CRITICAL] [END OF FILE]
	struct IEND
	{

	};

	////////// Transparency Chunk ///////////
	struct tRNsGrayscale
	{
		tRNsGrayscale()
			: TransparencyValue( 0 )
		{}

		// Copy constructor so we can use std::any
		tRNsGrayscale( const tRNsGrayscale& Other )
			: TransparencyValue( Other.TransparencyValue )
		{}

		uint16 TransparencyValue;
	};

	struct tRNsRGB
	{
		tRNsRGB()
			: TransparencyRed( 0 ), 
			TransparencyGreen( 0 ), 
			TransparencyBlue( 0 )
		{}

		// Copy constructor so we can use std::any
		tRNsRGB( const tRNsRGB& Other )
			: TransparencyRed( Other.TransparencyRed ),
			TransparencyGreen( Other.TransparencyGreen ),
			TransparencyBlue( Other.TransparencyBlue )
		{}

		uint16 TransparencyRed;
		uint16 TransparencyGreen;
		uint16 TransparencyBlue;
	};

	struct tRNsIndexed
	{
		tRNsIndexed()
		{}

		// Copy constructor so we can use std::any
		tRNsIndexed( const tRNsIndexed& Other )
			: TransparencyIndexes( Other.TransparencyIndexes )
		{}

		std::vector< uint8 > TransparencyIndexes;
	};

	/////////// Gamma //////////
	struct gAMA
	{
		uint32 GammaValue;
	};


	//////// File Header ////////
	struct PNGHeader
	{
		uint64 Data;
	};

	
	bool BuildChunks( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, std::vector< PNGChunk >& outChunks, bool bFlipBytes )
	{
		std::vector< byte >::const_iterator Pos = Begin;
		while( std::distance( Pos, End ) >= 12 )
		{
			PNGChunk Chunk;

			// Read length
			Binary::DeserializeUInt32( Pos, Chunk.Length, bFlipBytes );
			std::advance( Pos, 4 );

			// Read type
			char Letters[ 5 ];
			Binary::DeserializeUInt8( Pos, reinterpret_cast<uint8&>( Letters[ 0 ] ) );
			Binary::DeserializeUInt8( Pos + 1, reinterpret_cast<uint8&>( Letters[ 1 ] ) );
			Binary::DeserializeUInt8( Pos + 2, reinterpret_cast<uint8&>( Letters[ 2 ] ) );
			Binary::DeserializeUInt8( Pos + 3, reinterpret_cast<uint8&>( Letters[ 3 ] ) );
			std::advance( Pos, 4 );

			Letters[ 4 ] = '\0';
			Chunk.Type = std::string( Letters );

			// Validate the length parameter
			// Iterator is currently sitting on the start of the data, chunk layout looks like...
			// [4-byte length][4-byte type]['X'-byte data][4-byte CRC]
			// So if length + 4 is greater than remaining data, the length must be invalid
			if( Chunk.Length + 4 > std::distance( Pos, End ) )
			{
				Console::WriteLine( "[DEBUG] PNGReader: Failed to read chunk.. length was deserialized, but appears invalid" );
				return false;
			}

			// Copy data into chunk vector
			Chunk.Data.insert( Chunk.Data.end(), Pos, Pos + Chunk.Length );
			std::advance( Pos, Chunk.Length );

			// Read CRC-32 code
			Binary::DeserializeUInt32( Pos, Chunk.CRC, bFlipBytes );
			std::advance( Pos, 4 );

			// TODO: Validate CRC-32?

			outChunks.push_back( Chunk );
		}

		if( End - Pos > 0 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: After reading all chunks in.. there was ", ( End - Pos ), " bytes left in the file..." );
		}

		return true;
	}


	bool PerformFirstPassValidation( const std::vector< PNGChunk >& inChunks )
	{
		// In all cases, we need to have at least 3 chunks
		// IHDR, IDAT, IEND
		if( inChunks.size() < 3 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: There are not enough chunks in this PNG file!" );
			return false;
		}

		bool bHasData = false;
		bool bHasEnd = false;

		// The IHDR chunk has to be the first chunk in the png file
		for( uint32 i = 0; i < inChunks.size(); i++ )
		{
			if( i == 0 && inChunks[ i ].Type != "IHDR" )
			{
				Console::WriteLine( "[DEBUG] PNGReader: First chunk in PNG file is not the header chunk!" );
				return false;
			}
			else if( i > 0 )
			{
				// If there is a second IHDR chunk then the file is invalid
				if( inChunks[ i ].Type == "IHDR" )
				{
					Console::WriteLine( "[DEBUG] PNGReader: There are multiple IHDR chunks!" );
					return false;
				}
				else if( inChunks[ i ].Type == "IDAT" )
				{
					bHasData = true;
				}
				else if( inChunks[ i ].Type == "IEND" )
				{
					if( i != ( inChunks.size() - 1 ) )
					{
						Console::WriteLine( "[DEBUG] PNGReader: Last chunk in PNG file is not the end chunk!" );
						return false;
					}

					bHasEnd = true;
				}
			}
		}

		if( !bHasData )
		{
			Console::WriteLine( "[DEBUG] PNGReader: PNG file is missing critical data chunk!" );
			return false;
		}

		if( !bHasEnd )
		{
			Console::WriteLine( "[DEBUG] PNGReader: PNG file is missing critical end chunk!" );
			return false;
		}

		return true;
	}

	bool ReadHeaderChunk( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, IHDR& Out, bool bFlipBytes )
	{
		// Check length of data
		if( std::distance( Begin, End ) != 13 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Invalid IHDR chunk! Not enough data to read needed values" );
			return false;
		}

		// Deserialize everything
		auto Pos = Begin;

		// Deserialize width
		Binary::DeserializeUInt32( Pos, Out.Width, bFlipBytes );
		std::advance( Pos, 4 );

		if( Out.Width == 0 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Invalid IHDR chunk! Width is zero" );
			return false;
		}

		// Deserialize height
		Binary::DeserializeUInt32( Pos, Out.Height, bFlipBytes );
		std::advance( Pos, 4 );

		if( Out.Height == 0 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Invalid IHDR chunk! Height is zero" );
			return false;
		}

		// Deserialize bit depth
		Binary::DeserializeUInt8( Pos, Out.BitDepth );
		std::advance( Pos, 1 );

		if( Out.BitDepth == 0 || Out.BitDepth > 16 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Invalid IHDR chunk! Bit depth is invalid!" );
			return false;
		}

		// Deserialize color type
		uint8 colorType;
		Binary::DeserializeUInt8( Pos, colorType );
		std::advance( Pos, 1 );

		if( colorType > 6 || colorType == 5 || colorType == 1 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Invalid IHDR chunk! Color type is invalid!" );
			return false;
		}

		Out.Color = static_cast< ColorType >( colorType );

		// Deserialize compression method
		uint8 compressionMethod;
		Binary::DeserializeUInt8( Pos, compressionMethod );
		std::advance( Pos, 1 );

		if( compressionMethod != 0 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Invalid IHDR chunk! Compression method is invalid!" );
			return false;
		}

		Out.Compression = CompressionType::Deflate;

		// Deserialize filter method
		uint8 filterMethod;
		Binary::DeserializeUInt8( Pos, filterMethod );
		std::advance( Pos, 1 );

		if( filterMethod != 0 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Invalid IDHR chunk! Filter method is invalid!" );
			return false;
		}

		Out.Filter = FilterType::Adaptive;

		// Deserialize interlace method
		uint8 interlaceMethod;
		Binary::DeserializeUInt8( Pos, interlaceMethod );
		std::advance( Pos, 1 );

		if( interlaceMethod > 1 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Invalid IHDR chunk! Interlace method is invalid" );
			return false;
		}

		Out.Interlace = static_cast< InterlaceType >( interlaceMethod );

		// Validate bith depth
		if( Out.Color == ColorType::GS )
		{
			if( Out.BitDepth != 1 &&
				Out.BitDepth != 2 &&
				Out.BitDepth != 4 &&
				Out.BitDepth != 8 &&
				Out.BitDepth != 16 )
			{
				Console::WriteLine( "[DEBUG] PNGReader: Invalid bit depth for single-channel pixel!" );
				return false;
			}
		}
		else if( Out.Color == ColorType::GSA || Out.Color == ColorType::RGB || Out.Color == ColorType::RGBA )
		{
			if( Out.BitDepth != 8 &&
				Out.BitDepth != 16 )
			{
				Console::WriteLine( "[DEBUG] PNGReader: Invalid bit depth for multi-channel pixels!" );
				return false;
			}
		}
		else if( Out.Color == ColorType::Indexed )
		{
			if( Out.BitDepth != 1 &&
				Out.BitDepth != 2 &&
				Out.BitDepth != 4 &&
				Out.BitDepth != 8 )
			{
				Console::WriteLine( "[DEBUG] PNGReader: Invalid bit depth for indexed pixels!" );
				return false;
			}
		}

		return true;
	}

	bool PerformSecondPassValidation( IHDR& inHeader, const std::vector< PNGChunk >& inChunks )
	{
		// Check if there is a PLTE chunk
		bool bPLTE = false;
		bool bFoundData = false;

		for( uint32 i = 0; i < inChunks.size(); i++ )
		{
			if( inChunks[ i ].Type == "PLTE" )
			{
				// Check for multiple PLTE chunks
				if( bPLTE )
				{
					Console::WriteLine( "[DEBUG] PNGReader: There are multiple PLTE chunks in this png file!" );
					return false;
				}

				// Check if PLTE came after data
				if( bFoundData )
				{
					Console::WriteLine( "[DEBUG] PNGReader: The PLTE chunk doesnt come before the data chunk!" );
					return false;
				}

				// Validate PLTE
				auto size = inChunks[ i ].Data.size();
				if( size < 3 || size > 768 )
				{
					Console::WriteLine( "[DEBUG] PNGReader: The PLTE chunk data is an invalid size!" );
					return false;
				}

				bPLTE = true;
			}
			else if( inChunks[ i ].Type == "IDAT" )
			{
				bFoundData = true;
			}
		}

		// For Grayscale (with or without alpha), there should be no PLTE
		// And for Indexed, there needs to be a PLTE
		if( ( inHeader.Color == ColorType::GS || inHeader.Color == ColorType::GSA ) && bPLTE )
		{
			Console::WriteLine( "[DEBUG] PNGReader: There is a PLTE chunk in a grayscale image! This is invalid" );
			return false;
		}
		else if( inHeader.Color == ColorType::Indexed && !bPLTE )
		{
			Console::WriteLine( "[DEBUG] PNGReader: There is no PLTE chunk and the color type is indexed!" );
			return false;
		}

		// There is only a single PLTE chunk (if required), and appears before the IDAT chunk
		return true;
	}


	bool ReadPaletteChunk( std::vector< PNGChunk >& inChunks, const IHDR& inHeader, PLTE& outPalette )
	{
		// If we arent allowed to have a palette, just return, we know there isnt one already
		if( inHeader.Color == ColorType::GS || inHeader.Color == ColorType::GSA )
			return true;

		std::vector< PNGChunk >::iterator plte = inChunks.end();
		for( auto It = inChunks.begin(); It != inChunks.end(); It++ )
		{
			if( It->Type == "PLTE" )
			{
				plte = It;
				break;
			}
		}

		// This should never be true, weve already checked this ocne
		if( plte == inChunks.end() && inHeader.Color == ColorType::Indexed )
			return false;
		else if( plte == inChunks.end() )
			return true;

		// Now, we have a plte chunk and know which one it is, so we need to read it in
		// Were going to steal the data from the chunk
		std::vector< byte > PLTEData;
		PLTEData.swap( plte->Data );

		// Loop through while theres remaining data, and read palette entries
		auto Pos = PLTEData.begin();
		auto End = PLTEData.end();

		while( Pos + 3 < End )
		{
			PLTEEntry entry;
			Binary::DeserializeUInt8( Pos, entry.Red );
			std::advance( Pos, 1 );

			Binary::DeserializeUInt8( Pos, entry.Green );
			std::advance( Pos, 1 );

			Binary::DeserializeUInt8( Pos, entry.Blue );
			std::advance( Pos, 1 );

			outPalette.Colors.push_back( entry );
		}

		auto count = outPalette.Colors.size();
		return count > 0 && count < 257;
	}


	void ReadGammaChunk( std::vector< PNGChunk >& inChunks, gAMA& outGamma, bool bFlipBytes )
	{
		// Attempt to find this chunk
		std::vector< PNGChunk >::iterator gammaChunk = inChunks.end();
		for( auto It = inChunks.begin(); It != inChunks.end(); It++ )
		{
			if( It->Type == "gAMA" )
			{
				gammaChunk = It;
				break;
			}
		}

		if( gammaChunk == inChunks.end() )
			return;

		// Read in the 4-byte gamma value
		if( gammaChunk->Data.size() != 4 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Gamma chunk found in png file.. but its data field was invalid" );
			return;
		}

		Binary::DeserializeUInt32( gammaChunk->Data.begin(), outGamma.GammaValue, bFlipBytes );

		// Steal the data
		std::vector< byte >().swap( gammaChunk->Data );
	}


	std::any ReadTransparencyChunk( std::vector< PNGChunk >& inChunks, ColorType color, bool bFlipBytes )
	{
		// Lets check for color types where this isnt needed
		if( color == ColorType::GSA || color == ColorType::RGBA )
			return std::any();

		// First, lets try and find the chunk
		std::vector< PNGChunk >::iterator trnsChunk = inChunks.end();
		for( auto It = inChunks.begin(); It != inChunks.end(); It++ )
		{
			if( It->Type == "tRNs" )
			{
				trnsChunk = It;
				break;
			}
		}

		if( trnsChunk == inChunks.end() )
			return std::any();

		// Now, we need to read the structure in based on color type
		if( color == ColorType::GS )
		{
			// Validate size of data
			if( trnsChunk->Data.size() != sizeof( tRNsGrayscale ) )
			{
				Console::WriteLine( "[DEBUG] PNGReader: Transparency info chunk invalid, doesnt match size of structure (Grayscale)" );
				return std::any();
			}

			// Now we need to read in the values
			tRNsGrayscale t;
			
			Binary::DeserializeUInt16( trnsChunk->Data.begin(), t.TransparencyValue, bFlipBytes );

			std::vector< byte >().swap( trnsChunk->Data );

			return t;
		}
		else if( color == ColorType::RGB )
		{
			// Validate size of data
			if( trnsChunk->Data.size() != sizeof( tRNsRGB ) )
			{
				Console::WriteLine( "[DEBUG] PNGReader: Transparency info chunk invalid, doesnt match size of structure (RGB)" );
				return std::any();
			}

			// Read in values
			tRNsRGB t;

			auto Pos = trnsChunk->Data.begin();
			Binary::DeserializeUInt16( Pos, t.TransparencyRed, bFlipBytes );
			std::advance( Pos, 2 );

			Binary::DeserializeUInt16( Pos, t.TransparencyGreen, bFlipBytes );
			std::advance( Pos, 2 );

			Binary::DeserializeUInt16( Pos, t.TransparencyBlue, bFlipBytes );

			std::vector< byte >().swap( trnsChunk->Data );

			return t;
		}
		else if( color == ColorType::Indexed )
		{
			// We cant fully validate size, just that it needs to be greater than 0
			if( trnsChunk->Data.size() == 0 )
			{
				Console::WriteLine( "[DEBUG] PNGReader: Transparency info chunk invalid.. no data! (Indexed)" );
				return std::any();
			}

			// Each index is only a single byte, so we just need to loop through each byte and read it in
			tRNsIndexed t;

			for( auto It = trnsChunk->Data.begin(); It != trnsChunk->Data.end(); It++ )
			{
				t.TransparencyIndexes.push_back( *It );
			}

			std::vector< byte >().swap( trnsChunk->Data );

			return t;
		}
		else return std::any();

	}



	bool AccumulateImageData( std::vector< PNGChunk >& inChunks, std::vector< byte >& outData )
	{
		for( auto It = inChunks.begin(); It != inChunks.end(); It++ )
		{
			if( It->Type == "IDAT" )
			{
				// Move the data into the output vector and ensure the data in the chunk gets deleted
				outData.reserve( outData.size() + It->Data.size() );
				std::move( It->Data.data(), It->Data.data() + It->Data.size(), std::back_inserter( outData ) );
				std::vector< byte >().swap( It->Data );
			}
		}

		if( outData.size() == 0 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Failed to accumulate image data!" );
			return false;
		}

		return true;
	}

	uint8 GetPixelSizeBits( IHDR& HeaderChunk )
	{
		uint8 channelSize = HeaderChunk.BitDepth;

		switch( HeaderChunk.Color )
		{
		case ColorType::GSA:
			return channelSize * 2;
		case ColorType::RGB:
			return channelSize * 3;
		case ColorType::RGBA:
			return channelSize * 4;
		case ColorType::GS:
		case ColorType::Indexed:
		default:
			return channelSize;
			
		}
	}


	enum class LineFilterType
	{
		None = 0,
		Sub = 1,
		Up = 2,
		Avg = 3,
		Paeth = 4
	};

	uint32 RoundUpBitsToBytes( uint32 inBits )
	{
		uint32 roundedBits;
		if( inBits % 8 == 0 )
		{
			roundedBits = inBits;
		}
		else
		{
			roundedBits = ( 8 - inBits % 8 ) + inBits;
		}

		return inBits / 8;
	}


	uint8 PaethPredictor( uint8 a, uint8 b, uint8 c )
	{
		// Since we can hit negative numbers, use int16 
		int16 p = (int16)a + (int16)b - (int16)c;
		int16 pa_i = p - (int16) a;
		uint8 pa = (uint8) ( pa_i < 0 ? -pa_i : pa_i );
		int16 pb_i = p - (int16) b;
		uint8 pb = (uint8) ( pb_i < 0 ? -pb_i : pb_i );
		int16 pc_i = p - (int16) c;
		uint8 pc = (uint8) ( pc_i < 0 ? -pc_i : pc_i );

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



	bool DefilterImageData( std::vector< byte >& inData, IHDR& Header )
	{
		// We need to calculate a few values before we can start defiltering
		uint8 channelCount;
		switch( Header.Color )
		{
		case ColorType::GS:
		case ColorType::Indexed:
			channelCount = 1;
			break;
		case ColorType::GSA:
			channelCount = 2;
			break;
		case ColorType::RGB:
			channelCount = 3;
			break;
		case ColorType::RGBA:
			channelCount = 4;
			break;
		}

		uint32 pixelSizeBits	= Header.BitDepth * channelCount;
		uint32 lineSizeBits		= 8 + ( pixelSizeBits * Header.Width );

		uint32 lineSizeBytesCeil	= RoundUpBitsToBytes( lineSizeBits );
		uint32 pixelSizeBytesCeil	= RoundUpBitsToBytes( pixelSizeBits );

		// Validate vector size
		uint32 expectedSize = lineSizeBytesCeil * Header.Height;
		if( inData.size() < expectedSize )
		{
			Console::WriteLine( "[DEBUG] PNGReader: There is less data in the image than expected!" );
			return false;
		}

		uint8* dataStart = inData.data();

		// Now, we need to loop through our data line by line
		for( uint32 y = 0; y < Header.Height; y++ )
		{
			// Read filter type
			uint8 filterByte = *( dataStart + ( y * lineSizeBytesCeil ) );
			if( filterByte > ( uint8 ) LineFilterType::Paeth )
			{
				Console::WriteLine( "[DEBUG] PNGReader: There is an invalid filter byte on line ", y + 1, " (lines start at 1)" );
				return false;
			}

			LineFilterType filter = (LineFilterType) filterByte;

			// Loop through each byte in every line, skipping the line filter byte
			for( uint32 x = 0; x < lineSizeBytesCeil - 1; x++ )
			{
				uint8* thisByte = dataStart + ( y * lineSizeBytesCeil ) + x + 1;

				// Process filter
				if( filter == LineFilterType::Sub )
				{
					// Sub( x ) = Raw( x ) - Raw( x  - bpp ) where x is the byte position in the scanline & bpp is the number of bytes per pixel, rounded up to 1
					// Therefore:
					// Raw( x ) = Sub( x ) + Raw( x - bpp )

					// If were the first pixel in the scanline, prev is 0
					uint8 rawPrev = 0;

					if( x >= pixelSizeBytesCeil )
					{
						rawPrev = *( thisByte - pixelSizeBytesCeil );
					}

					*thisByte = *thisByte + rawPrev;
				}
				else if( filter == LineFilterType::Up )
				{
					// Up( x ) = Raw( x ) - Prior( x )
					// So.. Raw( x ) = Up( x ) + Prior( x ) where Prior( x ) is the same byte on the previous scanline

					// If were on the first scanline, prior is 0
					uint8 rawPrior = 0;

					if( y > 0 )
					{
						rawPrior = *( thisByte - lineSizeBytesCeil );
					}

					*thisByte = *thisByte + rawPrior;
				}
				else if( filter == LineFilterType::Avg )
				{
					// Avg( x ) = Raw( x ) - floor( ( Raw( x - bpp ) + Prior( x ) ) / 2 )
					// Raw( x ) = Avg( x ) + floor( ( Raw( x - bpp ) + Prior( x ) ) / 2 )
					uint8 rawPrev = 0;
					uint8 rawPrior = 0;

					if( x >= pixelSizeBytesCeil )
					{
						rawPrev = *( thisByte - pixelSizeBytesCeil );
					}

					if( y > 0 )
					{
						rawPrior = *( thisByte - lineSizeBytesCeil );
					}

					*thisByte = *thisByte + ( ( rawPrev + rawPrior ) / 2 );
				}
				else if( filter == LineFilterType::Paeth )
				{
					// Paeth( x ) = Raw( x ) - PaethPredictor( Raw( x - bpp ), Prior( x ), Prior( x - bpp ) )
					// Raw( x ) = Paeth( x ) + PaethPredictor( Raw( x - bpp ), Prior( x ), Prior( x - bpp ) )
					uint8 rawPrev = 0;
					uint8 rawPrior = 0;
					uint8 rawPrevPrior = 0;

					if( x >= pixelSizeBytesCeil )
					{
						rawPrev = *( thisByte - pixelSizeBytesCeil );

						if( y > 0 )
						{
							rawPrevPrior = *( thisByte - lineSizeBytesCeil - pixelSizeBytesCeil );
						}
					}

					if( y > 0 )
					{
						rawPrior = *( thisByte - lineSizeBytesCeil );
					}

					auto pp = PaethPredictor( rawPrev, rawPrior, rawPrevPrior );
					*thisByte = *thisByte + pp;
				}
				else
				{
					// No filter
				}
			}
		}

		// Complete!
		return true;
	}


	struct PixelPosition
	{
		uint32 Byte;
		uint8 Bit;
	};

	PixelPosition GetPixelPosition( uint32 X, uint32 Y, uint8 pixelSizeBits, uint32 lineSizeBytes, uint8 offsetBits )
	{
		// Figure out where the start of the line is in the source data
		// And then, figure out how far in the line this pixel is
		uint32 lineStart			= Y * lineSizeBytes;
		uint32 pixelStartOffset		= X * pixelSizeBits + 8 + offsetBits;

		// Next, we need to turn the pixelStartOffset from bits to bytes
		uint32 pixelStartBytes	= pixelStartOffset / 8;
		uint8 pixelStartBits	= pixelStartOffset % 8;

		// Add line offset to get final position
		PixelPosition Output;
		Output.Byte = lineStart + pixelStartBytes;
		Output.Bit = pixelStartBits;

		return Output;

	}

	bool ReadSingleBitChannel( uint8* in_byte, uint8 bit, uint8& out )
	{
		if( bit > 7 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Failed to read single bit channel, bit number out of range" );
			return false;
		}

		static const uint8 bit_masks[ 8 ] =
		{
			0b00000001,
			0b00000010,
			0b00000100,
			0b00001000,
			0b00010000,
			0b00100000,
			0b01000000,
			0b10000000
		};

		uint8 val = *in_byte & bit_masks[ bit ];
		out = val >> ( 7 - bit );

		return true;
	}

	bool ReadDoubleBitChannel( uint8* in_byte, uint8 start_bit, uint8& out )
	{
		if( start_bit != 0 && start_bit != 2 && start_bit != 4 && start_bit != 6 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Failed to read dual bit channel, bit number out of range" );
			return false;
		}

		static const uint8 bit_masks[ 4 ] =
		{
			0b00000011,
			0b00001100,
			0b00110000,
			0b11000000
		};

		uint8 bitIndex = start_bit / 2;
		uint8 val = *in_byte & bit_masks[ bitIndex ];
		out = val >> ( 6 - start_bit );

		return true;
	}

	bool ReadQuadBitChannel( uint8* in_byte, uint8 start_bit, uint8& out )
	{
		if( start_bit != 0 && start_bit != 4 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Failed to read quad bit channel, bit number out of range" );
			return false;
		}

		static const uint8 bit_masks[ 2 ] =
		{
			0b00001111,
			0b11110000
		};

		uint8 bitIndex = start_bit / 4;
		uint8 val = *in_byte & bit_masks[ bitIndex ];
		out = val >> ( 4 - start_bit );

		return true;
	}

	bool ReadEightBitChannel( uint8* in_byte, uint8& out )
	{
		out = *in_byte;
		return true;
	}

	bool ReadSixteenBitChannelFull( uint8* in_byte, uint16& out, bool bFlipBytes )
	{
		// Read bytes into uint16 taking into account byte order
		uint8 upper;
		uint8 lower;

		if( bFlipBytes )
		{
			lower = *( in_byte );
			upper = *( in_byte + 1 );
		}
		else
		{
			upper = *( in_byte );
			lower = *( in_byte + 1 );
		}

		uint16 fullValue = (uint16) upper << 8;
		fullValue |= (uint16) lower;

		out = fullValue;
		return true;
	}

	bool ReadChannelAs16Bit( uint8* in_byte, uint8 bit_depth, uint8 start_bit, uint16& out, bool bFlipBytes )
	{
		if( bit_depth <= 8 )
		{
			uint8 value;

			switch( bit_depth )
			{
			case 1:
				if( !ReadSingleBitChannel( in_byte, start_bit, value ) ) return false;
				break;
			case 2:
				if( !ReadDoubleBitChannel( in_byte, start_bit, value ) ) return false;
				break;
			case 4:
				if( !ReadQuadBitChannel( in_byte, start_bit, value ) ) return false;
				break;
			case 8:
				if( start_bit != 0 ) return false;
				if( !ReadEightBitChannel( in_byte, value ) ) return false;
				break;
			default:
				return false;
			}

			// Cast result to uint16 without scaling
			out = (uint16) value;

			return true;
		}
		else if( bit_depth == 16 )
		{
			if( start_bit != 0 ) return false;
			return ReadSixteenBitChannelFull( in_byte, out, bFlipBytes );
		}
		else return false;
	}


	uint8 ScaleSample( uint16 inSample, uint8 bitDepth )
	{
		switch( bitDepth )
		{
		case 1:
			if( inSample > 1 )
			{
				Console::WriteLine( "[DEBUG] PNGReader: WARNING, sample was out of range during scale function! Clamped to bit depth of 1" );
				inSample = 1;
			}

			return inSample * 255; // Orig range, 0 - 1, * 255 = 0 - 255
		case 2:

			if( inSample > 3 )
			{
				Console::WriteLine( "[DEBUG] PNGReader: WARNING, sample was out of range during scale function! Clamped to bit depth of 2" );
				inSample = 3;
			}

			return inSample * 85; // Orig range: 0 - 3, * 85 = 0 - 255
		case 4:

			if( inSample > 15 )
			{
				Console::WriteLine( "[DEBUG] PNGReader: WARNING, sample was out of range during scale function! Clamped to bit depth of 4" );
				inSample = 15;
			}

			return inSample * 17; // Orig range: 0 - 15, * 17 = 0 - 255
		case 8:

			if( inSample > 255 )
			{
				Console::WriteLine( "[DEBUG] PNGReader: WARNING, sample was out of range during scale function! Clamped to bit depth of 8" );
				inSample = 255;
			}

			return (uint8)inSample;
		case 16:
			return inSample / 257;
		default:
			Console::WriteLine( "[DEBUG] PNGReader: Failed to scale sample.. invalid bit depth" );
			return 0;
		}
	}


	struct RawPixel
	{
		uint8 r, g, b, a;
	};



	void ApplyGamma( RawPixel& inPixel, uint32 inGamma, float ScreenGamma )
	{
		// Do nothing for now...
	}

	void WritePixel( const RawPixel& inPixel, RawImageData& outData )
	{
		// Write each channel as a single byte to the output data
		Binary::SerializeUInt8( inPixel.r, outData.Data );
		Binary::SerializeUInt8( inPixel.g, outData.Data );
		Binary::SerializeUInt8( inPixel.b, outData.Data );
		Binary::SerializeUInt8( inPixel.a, outData.Data );
	}


	/*--------------------------------------------------------------------------------------------
		Process GS Image
	--------------------------------------------------------------------------------------------*/
	bool ProcessGrayscaleImage( std::vector< byte >& inData, std::any& TransparencyChunk, IHDR& HeaderChunk, uint32 Gamma, RawImageData& outData, bool bFlipBytes, float ScreenGamma )
	{
		uint8 pixelBits			= HeaderChunk.BitDepth;
		uint32 lineSizeBytes	= RoundUpBitsToBytes( 8 + HeaderChunk.Width * pixelBits );

		uint8* dataStart	= inData.data();
		auto dataSize		= inData.size();

		bool bApplyTransparency		= false;
		uint16 TransparencyValue	= 0;

		if( TransparencyChunk.has_value() )
		{
			if( TransparencyChunk.type() != typeid( tRNsGrayscale ) )
			{
				Console::WriteLine( "[DEBUG] PNGReader: Grayscale image.. but the transparency chunk wasnt in grayscale format?" );
				return false;
			}

			auto t = std::any_cast<tRNsGrayscale>( TransparencyChunk );

			bApplyTransparency	= true;
			TransparencyValue	= t.TransparencyValue;
		}

		// Set output image size
		outData.Width	= HeaderChunk.Width;
		outData.Height	= HeaderChunk.Height;

		// Reserve space in output data for the number of pixels * 4 bytes
		outData.Data.reserve( HeaderChunk.Width * HeaderChunk.Height * 4 );

		for( uint32 y = 0; y < HeaderChunk.Height; y++ )
		{
			for( uint32 x = 0; x < HeaderChunk.Width; x++ )
			{
				// Read pixel at position
				// For grayscale, we can have bits depths of 1, 2, 4, 8 or 16
				auto memPos = GetPixelPosition( x, y, pixelBits, lineSizeBytes, 0 );

				// The largest possible channel size is 16-bit, so we will read it into a uint16 for now, and check against the transparency key
				uint16 value;
				if( !ReadChannelAs16Bit( dataStart + memPos.Byte, HeaderChunk.BitDepth, memPos.Bit, value, bFlipBytes ) )
				{
					Console::WriteLine( "[DEBUG] PNGReader: Failed to read grayscale channel!" );
					return false;
				}

				// Check for transparency
				bool bIsTransparent = false;
				if( bApplyTransparency )
				{
					if( TransparencyValue == value )
					{
						bIsTransparent = true;
					}
				}

				// Scale sample to full uint8 range based on bit-depth
				uint8 scaledSample = ScaleSample( value, HeaderChunk.BitDepth );

				// Create pixel and apply gamma
				RawPixel pixel;

				pixel.r		= scaledSample;
				pixel.g		= scaledSample;
				pixel.b		= scaledSample;
				pixel.a		= bIsTransparent ? 0 : 255;

				// Apply gamma and write to the output
				ApplyGamma( pixel, Gamma, ScreenGamma );
				WritePixel( pixel, outData );
			}
		}
	
		Console::WriteLine( "[DEBUG] PNGReader: Processed grayscale image (no alpha) [", outData.Width, "x", outData.Height, "] and the output data size was ", outData.Data.size(), " bytes" );
		return true;
	}

	/*--------------------------------------------------------------------------------------------
		Process GSA Image
	--------------------------------------------------------------------------------------------*/
	bool ProcessGrayscaleAlphaImage( std::vector< byte >& inData, IHDR& HeaderChunk, uint32 Gamma, RawImageData& outData, bool bFlipBytes, float ScreenGamma )
	{
		// Calculate needed values
		uint8 pixelBits			= HeaderChunk.BitDepth * 2;
		uint32 lineSizeBytes	= RoundUpBitsToBytes( 8 + HeaderChunk.Width * pixelBits );

		uint8* dataStart	= inData.data();
		auto dataSize		= inData.size();

		// Set output image size
		outData.Width	= HeaderChunk.Width;
		outData.Height	= HeaderChunk.Height;

		// Reserve space in output data vector
		outData.Data.reserve( outData.Width * outData.Height * 4 );

		// Loop through each pixel in the source image data
		for( uint32 y = 0; y < HeaderChunk.Height; y++ )
		{
			for( uint32 x = 0; x < HeaderChunk.Width; x++ )
			{
				// Calculate position in memory
				auto pixelStart = GetPixelPosition( x, y, pixelBits, lineSizeBytes, 0 );

				auto gPos = GetPixelPosition( x, y, pixelBits, lineSizeBytes, 0 );
				auto aPos = GetPixelPosition( x, y, pixelBits, lineSizeBytes, HeaderChunk.BitDepth );

				// Read both channels into 16-bit uints regardless of source bit-depth
				uint16 g, a;
				if( !ReadChannelAs16Bit( dataStart + gPos.Byte, HeaderChunk.BitDepth, gPos.Bit, g, bFlipBytes ) ||
					!ReadChannelAs16Bit( dataStart + aPos.Byte, HeaderChunk.BitDepth, aPos.Bit, a, bFlipBytes ) )
				{
					Console::WriteLine( "[DEBUG] PNGReader: Failed to read GSA pixel from source data!" );
					return false;
				}

				// Scale these to uint8's
				uint8 gFinal	= ScaleSample( g, HeaderChunk.BitDepth );
				uint8 aFinal	= ScaleSample( a, HeaderChunk.BitDepth );

				// Create raw pixel and apply gamma
				RawPixel pixel;

				pixel.r		= gFinal;
				pixel.g		= gFinal;
				pixel.b		= gFinal;
				pixel.a		= aFinal;

				// Apply gamma and write to the output
				ApplyGamma( pixel, Gamma, ScreenGamma );
				WritePixel( pixel, outData );
			}
		}

		Console::WriteLine( "[DEBUG] PNGReader: Processed grayscale image (with alpha) [", outData.Width, "x", outData.Height, "] and the output data size was ", outData.Data.size(), " bytes" );
		return true;
	}

	/*--------------------------------------------------------------------------------------------
		Process RGB Image
	--------------------------------------------------------------------------------------------*/
	bool ProcessRGBImage( std::vector< byte >& inData, std::any& TransparencyChunk, IHDR& HeaderChunk, uint32 Gamma, RawImageData& outData, bool bFlipBytes, float ScreenGamma )
	{
		// Calculate needed values
		uint8 pixelBits = HeaderChunk.BitDepth * 3;
		uint32 lineSizeBytes = RoundUpBitsToBytes( 8 + HeaderChunk.Width * pixelBits );

		uint8* dataStart = inData.data();
		auto dataSize = inData.size();

		// Set output image size
		outData.Width = HeaderChunk.Width;
		outData.Height = HeaderChunk.Height;

		// Reserve space in output data vector
		outData.Data.reserve( outData.Width * outData.Height * 4 );

		// Calculate transparency
		bool bApplyTransparency = false;
		uint16 trnsR, trnsG, trnsB;
		if( TransparencyChunk.has_value() )
		{
			if( TransparencyChunk.type() != typeid( tRNsRGB ) )
			{
				Console::WriteLine( "[DEBUG] PNGReader: RGB Image has a transparency chunk.. but the type is not correct?" );
				return false;
			}

			auto t = std::any_cast<tRNsRGB>( TransparencyChunk );
			
			bApplyTransparency = true;

			trnsR	= t.TransparencyRed;
			trnsG	= t.TransparencyGreen;
			trnsB	= t.TransparencyBlue;
		}

		// Loop through each pixel in source data
		for( uint32 y = 0; y < HeaderChunk.Height; y++ )
		{
			for( uint32 x = 0; x < HeaderChunk.Width; x++ )
			{
				// Calculate the position of the pixel in memory
				auto rPos = GetPixelPosition( x, y, pixelBits, lineSizeBytes, 0 );
				auto gPos = GetPixelPosition( x, y, pixelBits, lineSizeBytes, HeaderChunk.BitDepth );
				auto bPos = GetPixelPosition( x, y, pixelBits, lineSizeBytes, HeaderChunk.BitDepth * 2 );

				if( rPos.Bit != 0 || gPos.Bit != 0 || bPos.Bit != 0 )
				{
					Console::WriteLine( "[DEBUG] PNGReader: RGB pixel channel doesnt align with byte boundries!" );
					return false;
				}

				// Read in pixel channels as uint-16's
				uint16 r, g, b;
				if( !ReadChannelAs16Bit( dataStart + rPos.Byte, HeaderChunk.BitDepth, 0, r, bFlipBytes ) ||
					!ReadChannelAs16Bit( dataStart + gPos.Byte, HeaderChunk.BitDepth, 0, g, bFlipBytes ) ||
					!ReadChannelAs16Bit( dataStart + bPos.Byte, HeaderChunk.BitDepth, 0, b, bFlipBytes ) )
				{
					Console::WriteLine( "[DEBUG] PNGReader: Failed to read RGB pixel from source data!" );
					return false;
				}

				// Check for transparency
				bool bIsTransparent = false;
				if( bApplyTransparency )
				{
					if( trnsR == r &&
						trnsG == g &&
						trnsB == b )
					{
						bIsTransparent = true;
					}
				}

				// Convert to full range uint8
				RawPixel pixel;

				pixel.r		= ScaleSample( r, HeaderChunk.BitDepth );
				pixel.g		= ScaleSample( g, HeaderChunk.BitDepth );
				pixel.b		= ScaleSample( b, HeaderChunk.BitDepth );
				pixel.a		= bIsTransparent ? 0 : 255;

				// Apply gamma to this pixel and write it to the output
				ApplyGamma( pixel, Gamma, ScreenGamma );
				WritePixel( pixel, outData );

			}
		}

		Console::WriteLine( "[DEBUG] PNGReader: Processed rgb image (no alpha) [", outData.Width, "x", outData.Height, "] and the output data size was ", outData.Data.size(), " bytes" );

		return true;
	}

	/*--------------------------------------------------------------------------------------------
		Process RGBA Image
	--------------------------------------------------------------------------------------------*/
	bool ProcessRGBAImage( std::vector< byte >& inData, IHDR& HeaderChunk, uint32 Gamma, RawImageData& outData, bool bFlipBytes, float ScreenGamma )
	{
		// Calculate needed values
		uint8 pixelBits			= HeaderChunk.BitDepth * 4;
		uint32 lineSizeBytes	= RoundUpBitsToBytes( 8 + HeaderChunk.Width * pixelBits );

		uint8* dataStart	= inData.data();
		auto dataSize		= inData.size();

		// Set output image size
		outData.Width		= HeaderChunk.Width;
		outData.Height		= HeaderChunk.Height;

		// Reserve space in output data vector
		outData.Data.reserve( outData.Width * outData.Height * 4 );

		// Loop through each pixel in source data
		for( uint32 y = 0; y < HeaderChunk.Height; y++ )
		{
			for( uint32 x = 0; x < HeaderChunk.Width; x++ )
			{
				// Calculate the position of the pixel in memory
				auto rPos = GetPixelPosition( x, y, pixelBits, lineSizeBytes, 0 );
				auto gPos = GetPixelPosition( x, y, pixelBits, lineSizeBytes, HeaderChunk.BitDepth );
				auto bPos = GetPixelPosition( x, y, pixelBits, lineSizeBytes, HeaderChunk.BitDepth * 2 );
				auto aPos = GetPixelPosition( x, y, pixelBits, lineSizeBytes, HeaderChunk.BitDepth * 3 );

				if( rPos.Bit != 0 || gPos.Bit != 0 || bPos.Bit != 0 || aPos.Bit != 0 )
				{
					Console::WriteLine( "[DEBUG] PNGReader: RGBA pixel channel doesnt align with byte boundries!" );
					return false;
				}

				// Read in pixel channels as uint-16's
				uint16 r, g, b, a;
				if( !ReadChannelAs16Bit( dataStart + rPos.Byte, HeaderChunk.BitDepth, 0, r, bFlipBytes ) ||
					!ReadChannelAs16Bit( dataStart + gPos.Byte, HeaderChunk.BitDepth, 0, g, bFlipBytes ) ||
					!ReadChannelAs16Bit( dataStart + bPos.Byte, HeaderChunk.BitDepth, 0, b, bFlipBytes ) ||
					!ReadChannelAs16Bit( dataStart + aPos.Byte, HeaderChunk.BitDepth, 0, a, bFlipBytes ) )
				{
					Console::WriteLine( "[DEBUG] PNGReader: Failed to read RGBA pixel from source data!" );
					return false;
				}

				// Convert to full range uint8
				RawPixel pixel;

				pixel.r		= ScaleSample( r, HeaderChunk.BitDepth );
				pixel.g		= ScaleSample( g, HeaderChunk.BitDepth );
				pixel.b		= ScaleSample( b, HeaderChunk.BitDepth );
				pixel.a		= ScaleSample( a, HeaderChunk.BitDepth );

				// Apply gamma and write pixel
				ApplyGamma( pixel, Gamma, ScreenGamma );
				WritePixel( pixel, outData );
			}
		}

		Console::WriteLine( "[DEBUG] PNGReader: Processed rgba image (with alpha) [", outData.Width, "x", outData.Height, "] and the output data size was ", outData.Data.size(), " bytes" );
		return true;
	}

	/*--------------------------------------------------------------------------------------------
		Process Indexed Image 
	--------------------------------------------------------------------------------------------*/
	bool ProcessIndexedImage( std::vector< byte >& inData, PLTE& PaletteChunk, std::any& TransparencyChunk, IHDR& HeaderChunk, uint32 Gamma, RawImageData& outData, bool bFlipBytes, float ScreenGamma )
	{
		// Calculate needed values
		uint8 pixelBits = HeaderChunk.BitDepth;
		uint32 lineSizeBytes = RoundUpBitsToBytes( 8 + HeaderChunk.Width * pixelBits );

		uint8* dataStart = inData.data();
		auto dataSize = inData.size();

		// Set output image size
		outData.Width = HeaderChunk.Width;
		outData.Height = HeaderChunk.Height;

		// Reserve space in output data vector
		outData.Data.reserve( outData.Width * outData.Height * 4 );

		// Read in transparency
		bool bHasTransparency = false;
		std::vector< uint8 > TransparencyIndexes;

		if( TransparencyChunk.has_value() )
		{
			if( TransparencyChunk.type() != typeid( tRNsIndexed ) )
			{
				Console::WriteLine( "[DEBUG] PNGReader: Indexed image has transparency chunk, but the type is invalid" );
				return false;
			}

			auto t = std::any_cast<tRNsIndexed>( TransparencyChunk );

			bHasTransparency = true;
			TransparencyIndexes = t.TransparencyIndexes;
		}

		// Loop through each pixel in source data
		for( uint32 y = 0; y < HeaderChunk.Height; y++ )
		{
			for( uint32 x = 0; x < HeaderChunk.Width; x++ )
			{
				// Calculate pixel position in memory
				auto pixelPos = GetPixelPosition( x, y, pixelBits, lineSizeBytes, 0 );

				// Read it in 
				uint16 index;
				if( !ReadChannelAs16Bit( dataStart + pixelPos.Byte, HeaderChunk.BitDepth, pixelPos.Bit, index, bFlipBytes ) )
				{
					Console::WriteLine( "[DEBUG] PNGReader: Failed to read index from indexed image!" );
					return false;
				}

				// Check for transparency
				uint8 alpha = 255;
				if( bHasTransparency )
				{
					if( TransparencyIndexes.size() > index )
					{
						alpha = TransparencyIndexes.at( index );
					}
				}

				// Lookup the rest of the color
				if( PaletteChunk.Colors.size() <= index )
				{
					Console::WriteLine( "[DEBUG] PNGReader: Failed to read indexed images.. index out of range!" );
					return false;
				}

				PLTEEntry& colors = PaletteChunk.Colors.at( index );

				// Create raw pixel
				RawPixel pixel;

				pixel.r		= colors.Red;
				pixel.g		= colors.Green;
				pixel.b		= colors.Blue;
				pixel.a		= alpha;

				// Apply gamma and write to output
				ApplyGamma( pixel, Gamma, ScreenGamma );
				WritePixel( pixel, outData );
			}
		}

		Console::WriteLine( "[DEBUG] PNGReader: Processed indexed image [", outData.Width, "x", outData.Height, "]  (", PaletteChunk.Colors.size(), " palette colors) and the output data size was ", outData.Data.size(), " bytes" );
		return true;
	}






	PNGReader::Result PNGReader::LoadFromMemory( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, std::shared_ptr< RawImageData >& Out, float ScreenGamma )
	{
		// First, we need to ensure there is enough data for a PNG file
		// We need...
		// Header: 8 bytes
		// IDHR Chunk: 13 + 12 = 25 bytes
		// IDAT Chunk: 12 bytes (at least for empty image)
		// IEND: 12 bytes
		// Minimum: 57 bytes

		Out.reset();

		auto fileSize = std::distance( Begin, End );
		if( fileSize < 57 )
		{
			Console::WriteLine( "[Warning] PNGReader: Failed to load png file.. not enough data!" );
			return Result::Failed;
		}

		// Read the first 8 bytes into the png file signature
		uint64 headerByte;
		Binary::DeserializeUInt64( Begin, headerByte, false );

		bool bFlipBytes = false;
		if( headerByte == 0x89'50'4E'47'0D'0A'1A'0A )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Header byte found! No flipping of bytes needed" );
		}
		else if( headerByte == 0x0A'1A'0A'0D'47'4E'50'89 )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Header byte found! Byte flipping needed!" );
			bFlipBytes = true;
		}
		else
		{
			Console::WriteLine( "[DEBUG] PNGReader: Header byte wasnt found! Not a PNG file!" );
			return Result::NotPNG;
		}

		// Next, we need to break the rest of the file out into chunks
		std::vector< PNGChunk > Chunks;
		if( !BuildChunks( Begin + 8, End, Chunks, bFlipBytes ) )
		{
			return Result::BadChunks;
		}

		// Perform first pass validation on the info we have so far
		if( !PerformFirstPassValidation( Chunks ) )
		{
			return Result::InvalidChunks;
		}

		// The first chunk is always the header chunk, so lets read that one in first
		IHDR HeaderChunk;
		if( !ReadHeaderChunk( Chunks[ 0 ].Data.begin(), Chunks[ 0 ].Data.end(), HeaderChunk, bFlipBytes ) )
		{
			return Result::InvalidIHDR;
		}

		// Now that were done with that chunk, clear the data
		std::vector< byte >().swap( Chunks[ 0 ].Data );

		// Perform second pass validation now that we know the color type
		if( !PerformSecondPassValidation( HeaderChunk, Chunks ) )
		{
			return Result::InvalidChunks;
		}

		// Now, we can get down to buisness
		// If we have a PLTE chunk, then were going to read it in
		PLTE PaletteChunk;
		if( !ReadPaletteChunk( Chunks, HeaderChunk, PaletteChunk ) )
		{
			return Result::InvalidPLTE;
		}

		// Next, lets read in any other image affecting chunks
		// Start with gamma
		gAMA GammaChunk;
		GammaChunk.GammaValue = 100'000;

		ReadGammaChunk( Chunks, GammaChunk, bFlipBytes );

		// We want to read in transparency info, but we have a different structure for each type
		std::any tRNsChunk = ReadTransparencyChunk( Chunks, HeaderChunk.Color, bFlipBytes );

		// Now, lets read in the image data
		// The image data might be split into multiple chunks, so what we will do is decompress each of them, and combine them into one large
		// buffer, which we will use to build the raw image data from
		std::vector< byte > CombinedBuffer;
		if( !AccumulateImageData( Chunks, CombinedBuffer ) )
		{
			return Result::InvalidIDAT;
		}

		// Decompress the image data
		std::vector< byte > DecompressedData;
		if( !Deflate::PerformInflate( CombinedBuffer, DecompressedData ) )
		{
			Console::WriteLine( "[DEBUG] PNGReader: Failed to decompress image data!" );
			return Result::InvalidIDAT;
		}

		// Clear original data
		std::vector< byte >().swap( CombinedBuffer );

		// Now, we have the data in the format we need to go through
		Out = std::make_shared< RawImageData >();
		Out->Width = HeaderChunk.Width;
		Out->Height = HeaderChunk.Height;

		// Before we process the final image, we need to defiler the data
		if( !DefilterImageData( DecompressedData, HeaderChunk ) )
		{
			Out.reset();
			return Result::InvalidIDAT;
		}

		// Now we need to read in the image data and write it out in raw format to the output structure
		switch( HeaderChunk.Color )
		{
		case ColorType::GS:

			if( !ProcessGrayscaleImage( DecompressedData, tRNsChunk, HeaderChunk, GammaChunk.GammaValue, *Out, bFlipBytes, ScreenGamma ) )
			{
				Out.reset();
				return Result::InvalidIDAT;
			}
			
			break;

		case ColorType::GSA:

			if( !ProcessGrayscaleAlphaImage( DecompressedData, HeaderChunk, GammaChunk.GammaValue, *Out, bFlipBytes, ScreenGamma ) )
			{
				Out.reset();
				return Result::InvalidIDAT;
			}

			break;

		case ColorType::RGB:

			if( !ProcessRGBImage( DecompressedData, tRNsChunk, HeaderChunk, GammaChunk.GammaValue, *Out, bFlipBytes, ScreenGamma ) )
			{
				Out.reset();
				return Result::InvalidIDAT;
			}

			break;

		case ColorType::RGBA:

			if( !ProcessRGBAImage( DecompressedData, HeaderChunk, GammaChunk.GammaValue, *Out, bFlipBytes, ScreenGamma ) )
			{
				Out.reset();
				return Result::InvalidIDAT;
			}

			break;

		case ColorType::Indexed:

			if( !ProcessIndexedImage( DecompressedData, PaletteChunk, tRNsChunk, HeaderChunk, GammaChunk.GammaValue, *Out, bFlipBytes, ScreenGamma ) )
			{
				Out.reset();
				return Result::InvalidIDAT;
			}

			break;

		default:

			Out.reset();
			return Result::Failed;
		}

		// At this point, everything is done!
		return Result::Success;
	}

}
}
