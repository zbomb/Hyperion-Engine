/*==================================================================================================
	Hyperion Engine
	Source/Core/Library/UTF16.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Library/Binary.h"
#include <vector>


namespace Hyperion
{
namespace Encoding
{
	class UTF16
	{

	private:

		enum class ProcessResult
		{
			Null,
			Invalid,
			LittleEndian,
			BigEndian
		};

		inline static uint16 ReadSurrogate( std::vector< byte >::const_iterator Iter, bool bFlipByteOrder = false )
		{
			// Were going to assume that the iterator is in a valid position, and the next position is also valid 
			// This should be checked by the caller before calling
			if( bFlipByteOrder )
				return ( ( *( Iter + 1 ) << 8 ) | ( *Iter ) );
			else
				return ( ( *Iter << 8 ) | ( *( Iter + 1 ) ) );
		}

		enum class SurrogateType
		{
			Invalid,
			Single,
			Pair
		};

		inline static SurrogateType ValidateSurrogate( uint16 inValue )
		{
			if( inValue <= 0xD7FF || ( inValue >= 0xE000 && inValue <= 0xFFFF ) )
			{
				return SurrogateType::Single;
			}
			else if( inValue >= 0xD800 && inValue <= 0xDFFF )
			{
				return SurrogateType::Pair;
			}
			else
			{
				return SurrogateType::Invalid;
			}
		}

		inline static SurrogateType ValidateCodePoint( uint32 CodePoint )
		{
			if( CodePoint >= 0x0000 && CodePoint <= 0xD7FF )
			{
				return SurrogateType::Single;
			}
			else if( CodePoint >= 0xE000 && CodePoint <= 0xFFFF )
			{
				return SurrogateType::Single;
			}
			else if( CodePoint >= 0x010000 && CodePoint <= 0x10FFFF )
			{
				return SurrogateType::Pair;
			}
			else
			{
				return SurrogateType::Invalid;
			}
		}

		static uint32 ReadSurrogatePair( uint16 UpperHalf, uint16 LowerHalf )
		{
			// A surrogate pair has some control bits set on it, so we need to extract the significant bits from the two values
			uint32 SigUpperBits = UpperHalf & 0b00000011'11111111;
			uint32 SigLowerBits = LowerHalf & 0b00000011'11111111;

			// Finally, perform an add and return the result
			return ( ( SigUpperBits << 10 ) | SigLowerBits ) + 0x10000;
		}

		static std::pair< uint16, uint16 > WriteSurrogatePair( uint32 inCode )
		{
			auto Output = std::make_pair< uint16, uint16 >( 0, 0 );

			// First, we need to validate the code point
			if( inCode < 0x10000 || inCode > 0x10FFFF )
				return Output;

			// Next, were going to subtract 0x10000 from the code point to get U'
			uint32 uPrime = inCode - 0x10000;

			// Now, were going to encode the bits into a 4 byte sequence
			Output.first	= ( ( uPrime & 0b00000000'00001111'11111100'00000000 ) >> 10 );
			Output.second	= ( ( uPrime & 0b00000000'00000000'00000011'11111111 ) );

			// Now, we need to add in the formatting
			Output.first	|= 0b11011000'00000000;
			Output.second	|= 0b11011100'00000000;

			return Output;
		}

		static std::pair< uint8, uint8 > SerializeSurrogate( uint16 In )
		{
			auto Output = std::make_pair< uint8, uint8 >( 0, 0 );

			Output.first = ( ( In & 0b11111111'00000000 ) >> 8 );
			Output.second = ( In & 0b00000000'11111111 );

			return Output;
		}


	public:

		UTF16() = delete;

		static ProcessResult ProcessBinary( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
		{
			// Ensure we are not null
			if( std::distance( Begin, End ) < 2 )
				return ProcessResult::Null;

			// Read in the first surrogate, and try and see if its a BOM
			uint16 FirstSurrogate = ( ( *Begin ) << 8 ) | *( Begin + 1 );

			if( FirstSurrogate == 0xFEFF )
			{
				return ProcessResult::BigEndian;
			}
			else if( FirstSurrogate == 0xFFFE )
			{
				return ProcessResult::LittleEndian;
			}

			// Since we dont have a valid BOM, we now need to try and read through all of the surrogates in the string, and see
			// how often the top and bottom bytes of surrogates are 0, and we can make a good guess on the endianess
			uint32 TopNull						= 0;
			uint32 LowerNull					= 0;
			uint32 ReadCount					= 0;
			static const uint32 MaxPairToRead	= 100;

			for( auto It = Begin;; )
			{
				// First, check if we hit the end of the string, to read enough characters
				if( It == End || It + 1 == End || ReadCount >= MaxPairToRead )
					break;

				// Read in the next two bytes
				uint8 topByte = *It;
				uint8 lowerByte = *( It + 1 );

				// Check if either are null, but not both
				if( topByte != lowerByte )
				{
					if( topByte == 0x00 )		TopNull++;
					if( lowerByte == 0x00 )		LowerNull++;
				}

				ReadCount++;

				// Incremement iterator
				std::advance( It, 2 );
			}

			// To have confidence in the endian selection.. the following must be true:
			// 1. At least 5% of bytes are null
			// 2. One byte (upper/lower) is null twice as often as the other
			auto TotalNulls = TopNull + LowerNull;
			if( TotalNulls > 0 && ReadCount > 0 )
			{
				float NullPercent = static_cast< float >( TotalNulls ) / static_cast< float >( ReadCount * 2 );
				if( NullPercent > 0.05 )
				{
					float NullBias = fabs( static_cast< float >( TopNull - LowerNull ) ) / static_cast< float >( TotalNulls );

					if( NullBias >= 0.5f )
					{
						// We can confidently say that we can determin endian order
						return TopNull > LowerNull ? ProcessResult::BigEndian : ProcessResult::LittleEndian;
					}
				}
			}

			// Since none of that worked.. were going to default to the endian order of the host OS
			return Binary::IsLittleEndian() ? ProcessResult::LittleEndian : ProcessResult::BigEndian;
		}

		/*
			Encoding::UTF16::IsValidCode
		*/
		static bool IsValidCode( uint32 In )
		{
			// Ensure code point is under the max encodable value
			if( In > 0x10FFFF )
				return false;

			// Ensure we arent a BOM
			if( In == 0xFEFF || In == 0xFFFE )
				return false;

			// Code Points U+D800 - U+DFFF
			// These are reserved because of the UTF-16 encoding scheme
			if( In >= 0xD800 && In <= 0xDFFF )
				return false;

			return true;
		}

		/*
			Encoding::UTF16::BinaryToCodes
		*/
		static bool BinaryToCodes( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, std::vector< uint32 >& outCodes, bool bIsLittleEndian )
		{
			// We dont need to call 'ProcessBinary' since were assuming the byte order the caller specified.. so we need 
			// to check for null strings, since normally that is done in that function
			if( std::distance( Begin, End ) < 2 )
				return true;

			// Next, we need to iterate through the string, read in the surrogates using the correct endian order.. validate them and insert into the outCodes vector
			for( auto It = Begin;; )
			{
				// Check for the end of the string
				if( It == End || It + 1 == End )
					break;

				// Read in the surrogate, taking endian order into account, and determine which type of value it is
				auto Surrogate	= ReadSurrogate( It, bIsLittleEndian );
				auto Result		= ValidateSurrogate( Surrogate );

				if( Result == SurrogateType::Single )
				{
					// We can use the raw uint16 as the value
					uint32 codePoint = static_cast< uint32 >( Surrogate );
					if( IsValidCode( codePoint ) )
					{
						outCodes.push_back( codePoint );
					}
				}
				else if( Result == SurrogateType::Pair )
				{
					// In this case, its a bit more complicated
					// We need to seek further ahread and read in the following surrogate
					if( It + 2 == End || It + 3 == End )
					{
						Console::WriteLine( "[ERROR] UTF-16 Decoder: Found surrogate half.. but hit the end of stream before reading the following half!" );
						break;
					}

					auto NextSurrogate	= ReadSurrogate( It + 2, bIsLittleEndian );
					auto NextResult		= ValidateSurrogate( NextSurrogate );

					if( NextResult == SurrogateType::Invalid )
					{
						Console::WriteLine( "[ERROR] UTF-16 Decoder: Found surrogate half.. but next surrogate was invalid!" );

						// Advance It by and extra two places, so we skip over this next surrogate on the next iteration
						std::advance( It, 2 );
					}
					else if( NextResult == SurrogateType::Single )
					{
						Console::WriteLine( "[ERROR] UTF-16 Decoder: Found surrogate half.. but next surrogate was not a pair!" );
						// In this case, were not going to advance, so we can read in the single on the next iteration
					}
					else if( NextResult == SurrogateType::Pair )
					{
						// Now.. we need to decode a surrogate pair into a code point
						uint32 codePoint = ReadSurrogatePair( Surrogate, NextSurrogate );
						if( IsValidCode( codePoint ) )
						{
							outCodes.push_back( codePoint );
						}

						// Advance the iterator by 2 so we dont read the lower half surrogate again
						std::advance( It, 2 );
					}
				}

				std::advance( It, 2 );
			}

			return true;
		}

		/*
			Encoding::UTF16::BinaryToCodes
		*/
		static bool BinaryToCodes( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, std::vector< uint32 >& outCodes )
		{
			// We need to call 'ProcessBinary' to auto-detect endian order
			auto Result = ProcessBinary( Begin, End );

			if( Result == ProcessResult::Null )					return true;
			else if( Result == ProcessResult::Invalid )			return false;
			else if( Result == ProcessResult::BigEndian )		return BinaryToCodes( Begin, End, outCodes, false );
			else if( Result == ProcessResult::LittleEndian )	return BinaryToCodes( Begin, End, outCodes, true );
			else												return false;
		}

		/*
			Encoding::UTF16::BinaryToCodes
		*/
		inline static bool BinaryToCodes( const std::vector< byte >& In, std::vector< uint32 >& outCodes )
		{
			return BinaryToCodes( In.begin(), In.end(), outCodes );
		}

		/*
			Encoding::UTF16::BinaryToCodes
		*/
		inline static bool BinaryToCodes( const std::vector< byte >& In, std::vector< uint32 >& outCodes, bool bIsLittleEndian )
		{
			return BinaryToCodes( In.begin(), In.end(), outCodes, bIsLittleEndian );
		}

		/*
			Encoding::UTF16::CodesToBinary
		*/
		static bool CodesToBinary( std::vector< uint32 >::const_iterator Begin, std::vector< uint32 >::const_iterator End, std::vector< byte >& outData )
		{
			if( Begin == End )
				return true;

			// Lets reserve some space in the vector for the data we will write
			// Were going to assume all characters are 2 bytes for this operation.. also 2 bytes for the BOM
			outData.reserve( outData.capacity() + ( std::distance( Begin, End ) * 2 ) + 2 );

			// Write out the BOM, which will be the BOM for Big-Endian systems
			outData.insert( outData.end(), 
				{
					0xFE,
					0xFF
				} );

			for( auto It = Begin; It != End; It++ )
			{
				// Validate the code point, and determine the method we need to use to write this to the stream
				auto Type = ValidateCodePoint( *It );

				if( Type == SurrogateType::Single )
				{
					// Serialize surrogates to two bytes and write into the output stream
					auto Bytes = SerializeSurrogate( *It );
					outData.insert( outData.end(), { Bytes.first, Bytes.second } );
				}
				else if( Type == SurrogateType::Pair )
				{
					// Create surrogate pair, and serialize both into the output stream
					auto Pair = WriteSurrogatePair( *It );
					auto First = SerializeSurrogate( Pair.first );
					auto Second = SerializeSurrogate( Pair.second );

					outData.insert( outData.end(), { First.first, First.second, Second.first, Second.second } );
				}
				else
				{
					// Invalid Code Point!
					continue;
				}
			}

			return true;
		}

		/*
			Encoding::UTF16::CodesToBinary
		*/
		inline static bool CodesToBinary( const std::vector< uint32 >& inCodes, std::vector< byte >& outData )
		{
			return CodesToBinary( inCodes.begin(), inCodes.end(), outData );
		}

		static bool CodesToU16String( std::vector< uint32 >::const_iterator Begin, std::vector< uint32 >::const_iterator End, std::u16string& outStr, bool bIncludeBOM = false )
		{
			if( Begin == End )
				return false;

			// Create vector for the output data and reserve needed space
			std::vector< char16_t > u16Chars;
			u16Chars.reserve( std::distance( Begin, End ) / 2 );

			// Were going to write out a BOM.. TODO: Is this needed?
			if( bIncludeBOM )
			{
				u16Chars.push_back( 0xFEFF );
			}

			for( auto It = Begin; It != End; It++ )
			{
				auto Type = ValidateCodePoint( *It );
				if( Type == SurrogateType::Single )
				{
					// Serialize surrogate to single char16_t
					auto bytes = SerializeSurrogate( *It );
					u16Chars.push_back( ( bytes.first << 8 ) | ( bytes.second ) );
				}
				else if( Type == SurrogateType::Pair )
				{
					// Create surrogate pair, and write both to the char vector
					auto pair = WriteSurrogatePair( *It );
					auto first = SerializeSurrogate( pair.first );
					auto second = SerializeSurrogate( pair.second );

					u16Chars.push_back( ( first.first << 8 ) | first.second );
					u16Chars.push_back( ( second.first << 8 ) | second.second );
				}
				else
				{
					// Invalid code point
					continue;
				}
			}

			// Build the output string
			outStr = std::u16string( u16Chars.data(), u16Chars.size() );
			return true;
		}

		/*
			Encoding::UTF16::ReadNextCode
		*/
		static bool ReadNextCode( const std::vector< byte >& inData, std::vector< byte >::const_iterator& Position, bool bLittleEndian, uint32& outCode )
		{
			// inData: The source vector
			// Position: The start of the next character (one past the last read character)
			// * This will be updated to the start of the character after the one thats read
			// bLittleEndian: true if the source data is using little endian
			// outCode: the code we read from the next charcter (0 if none is read)
			outCode = 0;

			if( inData.size() % 2 != 0 )
			{
				Position = inData.end();
				return false;
			}

			// Basically.. we want to iterate until we find the start of a valid character.. read it.. set Position to the start of the next character (or where it should be)
			// But.. if we have a BOM, we need to be able to skip over that gracefully
			// For simplicity.. were just going to ignore BOM sequences
			for( ;; )
			{
				// Check for end of data
				if( std::distance( Position, inData.end() ) < 2 )
					break;

				// Read in the surrogate
				uint16 surrogate = ReadSurrogate( Position, bLittleEndian );

				// If we have a BOM, skip to the next surrogate
				if( surrogate != 0xFEFF && surrogate != 0xFFFE )
				{
					// Check what type of surrogate it is
					auto Result = ValidateSurrogate( surrogate );
					if( Result == SurrogateType::Single )
					{
						// Check if this code point is valid
						uint32 CodePoint = static_cast< uint32 >( surrogate );

						if( IsValidCode( CodePoint ) )
						{
							outCode = CodePoint;
							std::advance( Position, 2 );
							return true;
						}
					}
					else if( Result == SurrogateType::Pair )
					{
						// Advance the iterator to the upper half surrogate
						std::advance( Position, 2 );

						// Check if we have enough data left to read this in
						if( std::distance( Position, inData.end() ) < 2 )
							break;
					
						// Now we want to ensure this second half is valid
						auto nextSurrogate = ReadSurrogate( Position, bLittleEndian );
						auto nextResult = ValidateSurrogate( nextSurrogate );

						if( nextResult == SurrogateType::Pair )
						{
							// Read in the entire code point
							uint32 CodePoint = ReadSurrogatePair( surrogate, nextSurrogate );
							if( IsValidCode( CodePoint ) )
							{
								outCode = CodePoint;
								std::advance( Position, 2 );
								return true;
							}
						}
					}
				}

				// If we get here.. the current surrogate isnt valid.. so lets advance and try again
				std::advance( Position, 2 );
			}

			// If we get here.. we broke out of the loop without finding a valid character
			Position = inData.end();
			return false;
		}



	};
}
}