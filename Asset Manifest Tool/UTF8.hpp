/*==================================================================================================
	Hyperion Engine
	Source/Core/Library/UTF8.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <stdint.h>

typedef uint8_t byte;
typedef uint8_t uint8;
typedef uint32_t uint32;

#include <vector>


namespace Hyperion
{
	class String;

namespace Encoding
{
	class UTF8
	{

	private:

		friend class Hyperion::String;

		/*-----------------------------------------------------
			Helper Functions
		------------------------------------------------------*/
		static bool CheckForCharStart( byte In, uint8& outLen )
		{
			if( ( In & 0b11000000 ) == 0b10000000 )
			{
				outLen = 0;
				return false;
			}
			else if( ( In & 0b10000000 ) == 0b00000000 )
			{
				outLen = 1;
				return true;
			}
			else if( ( In & 0b11100000 ) == 0b11000000 )
			{
				outLen = 2;
				return true;
			}
			else if( ( In & 0b11110000 ) == 0b11100000 )
			{
				outLen = 3;
				return true;
			}
			else if( ( In & 0b11111000 ) == 0b11110000 )
			{
				outLen = 4;
				return true;
			}
			else
			{
				outLen = 0;
				return false;
			}
		}

		static bool CheckIfContinuationByte( byte In )
		{
			return ( In & 0b11000000 ) == 0b10000000;
		}

		static bool CheckForCharStartFast( byte In )
		{
			if( ( In & 0b10000000 ) == 0b00000000 )			return true;
			else if( ( In & 0b11000000 ) == 0b11000000 )	return true;
			else											return false;
		}

		static bool ReadChar( std::vector< byte >::const_iterator CharStart, std::vector< byte >::const_iterator CharEnd, uint32& outChar )
		{
			// Here, we need to gather the bytes, remove the encoding bits, read it as a uint32
			if( std::distance( CharStart, CharEnd ) <= 0 )
			{
				outChar = 0;
				return false;
			}

			uint8 Index			= 0;
			uint32 FinalValue	= 0;

			for( auto It = CharStart; It != CharEnd; )
			{
				uint8 charByte = *It;

				// Ensure this byte is valid
				if( !CheckByte( charByte ) )
				{
					outChar = 0;
					return false;
				}

				// If were reading the first character, were going to look for character start encodings
				if( Index == 0 )
				{
					if( ( charByte & 0b10000000 ) == 0b00000000 )
					{
						// Read seven bits
						FinalValue = charByte & 0b01111111;
					}
					else if( ( charByte & 0b11100000 ) == 0b11000000 )
					{
						// Read 5 bits
						FinalValue = charByte & 0b00011111;
					}
					else if( ( charByte & 0b11110000 ) == 0b11100000 )
					{
						// Read 4 bits
						FinalValue = charByte & 0b00001111;
					}
					else if( ( charByte & 0b11111000 ) == 0b11110000 )
					{
						// Read 3 bits
						FinalValue = charByte & 0b00000111;
					}
					else
					{
						// Error!
						return false;
					}
				}
				else
				{
					// Check for continuation byte mark
					if( ( charByte & 0b11000000 ) == 0b10000000 )
					{
						// Push current value over 6 bits, and insert these 6 bits
						FinalValue = ( FinalValue << 6 ) | ( charByte & 0b00111111 );
					}
				}

				// Increment index and iter
				It++;
				Index++;
			}

			// TODO: Perform Checks
			outChar = FinalValue;
			return true;
		}

		static bool CheckByte( uint8 inByte )
		{
			return( inByte != 0xC0 && inByte != 0xC1 && inByte < 0xF5 );
		}

		static void EncodePoint( uint32 inCode, std::vector< byte >& outData )
		{
			// Special case for invalid char
			if( inCode == 0 )
			{
				outData.insert( outData.end(), { 0b11101111, 0b10111111, 0b10111101 } );
			}
			else if( inCode < 0x80 )
			{
				// Single-Byte character
				// 7-bits in the byte
				outData.push_back( ( (uint8) inCode ) & 0b01111111 );
			}
			else if( inCode < 0x800 )
			{
				// Double-Byte Character
				// 5-bits in top byte, 6-bits in bottom byte
				uint8 topByte = ( ( inCode & 0b00000000'00000000'00000111'11000000 ) >> 6 );
				uint8 botByte = (   inCode & 0b00000000'00000000'00000000'00111111 );

				// Now we just have to add in the special bits
				topByte |= 0b11000000;
				botByte |= 0b10000000;

				outData.insert( outData.end(), { topByte, botByte } );
			}
			else if( inCode < 0x10000 )
			{
				// Triple-Byte Character
				// 4-bits in top byte, 6-bits in second byte, 6-bits in third byte
				uint8 topByte = ( ( inCode & 0b00000000'00000000'11110000'00000000 ) >> 12 );
				uint8 midByte = ( ( inCode & 0b00000000'00000000'00001111'11000000 ) >> 6 );
				uint8 botByte = ( ( inCode & 0b00000000'00000000'00000000'00111111 ) );

				// Add in special bits
				topByte |= 0b11100000;
				midByte |= 0b10000000;
				botByte |= 0b10000000;

				outData.insert( outData.end(), { topByte, midByte, botByte } );
			}
			else if( inCode < 0x10FFFF )
			{
				// Quad-Byte Character
				// 3-bits in top byte, 6-bits in the rest (x3)
				uint8 topByte = ( ( inCode & 0b00000000'00011100'00000000'00000000 ) >> 18 );
				uint8 tmdByte = ( ( inCode & 0b00000000'00000011'11110000'00000000 ) >> 12 );
				uint8 bmdByte = ( ( inCode & 0b00000000'00000000'00001111'11000000 ) >> 6 );
				uint8 botByte = ( ( inCode & 0b00000000'00000000'00000000'00111111 ) );

				// Add in format bits
				topByte |= 0b11110000;
				tmdByte |= 0b10000000;
				bmdByte |= 0b10000000;
				botByte |= 0b10000000;

				outData.insert( outData.end(), { topByte, tmdByte, bmdByte, botByte } );
			}
			else
			{
				// Invalid character, write our invalid char sequence
				EncodePoint( 0, outData );
			}

		}


	public:

		UTF8() = delete;

		/*-----------------------------------------------------
			Public Functions
		------------------------------------------------------*/

		static bool AutoDetect( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, const uint32 MaxBytesToCheck = 120 )
		{
			// Try to determine if the byte sequence provided is UTF-8
			// First, we can check for the unofficial UTF-8 BOM sequence
			auto codePoints = std::distance( Begin, End );
			if( codePoints >= 3 )
			{
				auto First		= *( Begin + 0 );
				auto Second		= *( Begin + 1 );
				auto Third		= *( Begin + 2 );

				if( First == 0xEF && Second == 0xBB && Third == 0xBF )
				{
					return true;
				}
			}

			// The next best solution would be to check for valid encoding scheme.. unfortunatley we need to read through
			// the whole string to determine.. maybe we could read until we hit a certain number of valid code points
			uint32 ValidCodePoints = 0;
			uint32 InvalidCodePoints = 0;

			for( auto It = Begin; It != End; )
			{
				// Check if we have enough data to determine if the string is valid
				if( ValidCodePoints + InvalidCodePoints > MaxBytesToCheck )
				{
					break;
				}

				// Check if the code point is within valid range
				uint8 codePoint = *It;

				if( !CheckByte( codePoint ) )
				{
					InvalidCodePoints++;
					std::advance( It, 1 );
					continue;
				}

				// Check if this is the start of a character
				uint8 charLength;
				if( CheckForCharStart( codePoint, charLength ) )
				{
					// Count this as a valid code point
					ValidCodePoints++;

					// Check to ensure the next 'charLength' code points are continuation bytes
					if( std::distance( It, End ) < charLength )
					{
						// Not enough bytes to form this character, so break out of the loop
						// But first, count the rest of the code points as invalid
						InvalidCodePoints += ( charLength - 1 );
						break;
					}

					// Get iterators to the start and end of the character
					auto charEnd	= It + charLength;
					auto charBegin	= It;

					// Advance the 'main' iterator to the start of the next character
					std::advance( It, charLength );

					for( auto FIter = charBegin + 1; FIter != charEnd; FIter++ )
					{
						// Check if this byte is a continuation byte
						if( !CheckIfContinuationByte( *FIter ) )
						{
							InvalidCodePoints++;
						}
						else
						{
							ValidCodePoints++;
						}
					}

				}
				else
				{
					// Ran into a character that doesnt begin with a valid sequence
					InvalidCodePoints++;
					std::advance( It, 1 );
				}
			}

			// At this point, we either hit the end of the string, or checked the max number of bytes
			// So, we need to determine if we have enough valid code points to call this a UTF-8 string
			// For now, we only allow 1 invalid code point per 100 valid code points
			uint32 maxInvalid = ValidCodePoints < 40 ? 0 : ( ( ValidCodePoints / 100 ) + 1 );
			return InvalidCodePoints <= maxInvalid;
		}

		static bool IsValidCode( uint32 inCode )
		{
			// Ensure code point is under the max encodable value
			if( inCode > 0x10FFFF )
				return false;

			// Code Points U+D800 - U+DFFF
			// These are reserved because of the UTF-16 encoding scheme
			if( inCode >= 0xD800 && inCode <= 0xDFFF )
				return false;

			return true;
		}

		static bool BinaryToCodes( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, std::vector< uint32 >& outCodes )
		{
			bool bError = false;

			// Check for BOM
			auto codePoints = std::distance( Begin, End );
			if( codePoints >= 3 )
			{
				auto First		= *( Begin + 0 );
				auto Second		= *( Begin + 1 );
				auto Third		= *( Begin + 2 );

				if( First == 0xEF && Second == 0xBB && Third == 0xBF )
				{
					// BOM Detected
					std::advance( Begin, 3 );
				}
			}

			for( auto It = Begin; It != End; )
			{
				// Check if this is the start of a character
				uint8 charLength = 0;
				if( CheckByte( *It ) && CheckForCharStart( *It, charLength ) )
				{
					std::vector< byte >::const_iterator CharBegin	= It;
					std::vector< byte >::const_iterator CharEnd		= It;

					// We want to look forward until we find the start of the next character
					for( auto FIter = It + 1;; )
					{
						// Check if we hit the end of the string, or the start of another character
						if( FIter == End || !CheckIfContinuationByte( *FIter ) )
						{
							CharEnd = FIter;
							break;
						}

						FIter++;
					}

					// Check for missing bytes
					auto iterDistance = std::distance( CharBegin, CharEnd );
					if( iterDistance < charLength )
					{
						// This character will be a question byte
						// Use a question byte for each byte
						for( int i = 0; i < iterDistance; i++ )
						{
							outCodes.push_back( 0 );
						}

						bError = true;
					}
					else
					{
						// Were only going to use the bytes we actually need
						auto RealCharEnd = CharBegin + charLength;
						uint32 newChar = 0;

						if( ReadChar( CharBegin, RealCharEnd, newChar ) )
						{
							outCodes.push_back( newChar );
						}
						else
						{
							// Were going to use a question mark for each byte
							for( int i = 0; i < charLength; i++ )
							{
								outCodes.push_back( 0 );
							}

							bError = true;
						}

						// Now, if theres extra bytes in this character, were going to use a question byte for each extra byte
						if( iterDistance > charLength )
						{
							auto extraBytes = iterDistance - charLength;
							for( int i = 0; i < extraBytes; i++ )
							{
								outCodes.push_back( 0 );
							}

							bError = true;
						}
					}

					// Were going to set the iterator to the start of the next character
					It = CharEnd;
				}
				else
				{
					// Were going to put a question byte here
					bError = true;
					outCodes.push_back( 0 );
					It++;
				}
			}

			return !bError;
		}

		static bool BinaryToCodes( const std::vector< byte >& inData, std::vector< uint32 >& outCodes )
		{
			return BinaryToCodes( inData.begin(), inData.end(), outCodes );
		}

		static bool CodesToBinary( std::vector< uint32 >::const_iterator Begin, std::vector< uint32 >::const_iterator End, std::vector< byte >& outData )
		{
			bool bError = false;
			for( auto It = Begin; It != End; It++ )
			{
				if( IsValidCode( *It ) )
				{
					EncodePoint( *It, outData );
				}
				else
				{
					EncodePoint( 0, outData );
					bError = true;
					continue;
				}
			}

			return !bError;
		}

		static bool CodesToBinary( const std::vector< uint32 >& inCodes, std::vector< byte >& outData )
		{
			return CodesToBinary( inCodes.begin(), inCodes.end(), outData );
		}


		static bool ReadNextCode( const std::vector< byte >& inData, std::vector< byte >::const_iterator& Position, uint32& outCode )
		{
			// Basically.. this function does a few things
			// 1) Reads in the next code point.. starting at 'Position' in the 'inData' vector
			// 2) Updates 'Position' to the start of the next character
			// 3) Outputs the code as 'outCode'
			// 4) Returns false if we hit the end of 'inData' while seeking the next character

			outCode = 0;

			// UTF-8 BOMs are a little more complicated than UTF-16
			// We are going to assume that a BOM will only occur at the begining of a buffer
			if( Position == inData.begin() && inData.size() >= 3 )
			{
				byte First		= *( Position + 0 );
				byte Second		= *( Position + 1 );
				byte Third		= *( Position + 2 );

				if( First == 0xEF && Second == 0xBB && Third == 0xBF )
				{
					// BOM Detected, skip these bytes before seeking next character
					std::advance( Position, 3 );
				}
			}

			for( ;; )
			{
				// Check if we hit the end of the data
				if( Position == inData.end() )
					break;

				// Read this code point in
				uint8 thisCode		= *Position;
				uint8 charLength	= 0;
				
				// Check if this is valid and the start of a character
				if( CheckByte( thisCode ) && CheckForCharStart( thisCode, charLength ) )
				{
					// Now that we found the start of a character.. we need to find the end of the character
					auto charBegin	= Position;
					auto charEnd	= charBegin;

					for( auto FIter = charBegin + 1;; )
					{
						// Check if we hit the end of the string, or the start of another character
						if( FIter == inData.end() || !CheckIfContinuationByte( *FIter ) )
						{
							charEnd = FIter;
							break;
						}

						FIter++;
					}

					// Check if the distance is as expected
					auto iterDistance = std::distance( charBegin, charEnd );
					if( iterDistance < charLength )
					{
						// Skip to the start of the next character
						// If we hit the end, on next iteration we will break out of the main loop
						Position = charEnd;
						continue;
					}
					else
					{
						// We have enough bytes to read the character in
						// Were going to use the character length dictated by the encoding though
						auto realEnd = charBegin + charLength;

						// Update 'Position' to the start of the next character
						Position = charEnd;

						// Lets try and read in this character
						uint32 CharCode;
						if( ReadChar( charBegin, realEnd, CharCode ) )
						{
							outCode = CharCode;
							return true;
						}
					}
				}
			}

			// If we hit this point.. we couldnt find a valid character
			Position = inData.end();
			return false;
		}
	};
}
}