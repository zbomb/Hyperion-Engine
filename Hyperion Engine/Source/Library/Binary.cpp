/*==================================================================================================
	Hyperion Engine
	Source/Core/Library/Binary.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Library/Binary.h"

#include <iostream>
#include <iomanip>
#include <bitset>
#include <algorithm>


namespace Hyperion
{

		/*
			Binary::IsLittleEndian
		*/
		bool Binary::IsLittleEndian()
		{
			static const uint32 n = 1;
			static const bool Output = ( *(char *)&n == 1 );
			return Output;
		}

		/*
			Binary::Print
		*/
		void Binary::PrintHex( const std::vector< byte >::const_iterator Begin, const std::vector< byte >::const_iterator End )
		{
			// TODO: Need to reimplement with new console system!
		}

		/*
			Binary::PrintBin
		*/
		void Binary::PrintBin( const std::vector< byte >::const_iterator Begin, const std::vector< byte >::const_iterator End )
		{
			// TODO: Need to reimplement with new console system
		}

		/*
			Binary::SerializeFloat
		*/
		void Binary::SerializeFloat( const float& In, std::vector< byte >& Output, bool bOutputAsLittleEndian /* = false */ )
		{
			/*
			// We can build these bytes manually, the order of the bytes is
			// [MSB] First, Second, Third, Fourth [LSB]
			// Sign Bit: 1 bit
			// Exponent: 8 Bits
			// Mantissa: 23 bits
			byte First, Second, Third, Fourth;
			First =
				Second =
				Third =
				Fourth =
				byte( 0b0000'0000 );

			// Check for NaN or +/- Inf
			if( isnan( In ) )
			{
				// NaN
				First	= byte( 0b0111'1111 );
				Second = 
					Third = 
					Fourth = 
					byte( 0b1111'1111 );
			}
			else if( isinf( In ) )
			{
				if( signbit( In ) )
				{
					// +Inf
					First	= byte( 0b0111'1111 );
					Second	= byte( 0b1000'0000 );
				}
				else
				{
					// -Inf
					First	= byte( 0b1111'1111 );
					Second	= byte( 0b1000'0000 );
				}
			}
			else
			{
				// Split the float into Mantissa, and exponent.
				// Maintissa Range: [(-1, -0.5], 0, [0.5, 1)]
				// Exponent is a base-2 exponent
				int32 Exponent;
				float Base = frexpf( In, &Exponent );

				// Figure out the sign
				bool bNegative = signbit( In );

				// Now, we want to have the base be in the range of [0,1]
				// This gives us more accuracy when storing this value in 23 bits
				float ScaledBase = ( fabs( Base ) * 2.f ) - 1.f;

				// Now, we want to turn this scalar into a 23 bit integer, so we can represent it in binary
				// We can do this is by multiplying it by the max value of 23-bits
				uint32 Mantissa = (uint32) truncf( ldexp( ScaledBase, 23 ) );

				// Check to ensure values are within expected ranges
				// The exponent has to be stored in a 8-bit unsigned integer, so lets check if we can do this
				static const auto maxExponent = std::numeric_limits< std::int8_t >::max();
				static const auto minExponent = std::numeric_limits< std::int8_t >::min();

				// Check for overflow
				if( Exponent > maxExponent )
				{
					if( !bNegative )
					{
						// +INF
						First	= byte( 0b0111'1111 );
						Second	= byte( 0b1000'0000 );
					}
					else
					{
						// -INF
						First = byte( 0b1111'1111 );
						Second = byte( 0b1000'0000 );
					}
				}
				else if( Exponent >= minExponent )
				{
					// The bottom two bytes of the output, are simply going to be the bottom two bytes of the mantissa
					Fourth	= (byte) ( ( Mantissa & 0b0000'0000'0000'0000'0000'0000'1111'1111 ) );
					Third	= (byte) ( ( Mantissa & 0b0000'0000'0000'0000'1111'1111'0000'0000 ) >> 8 );

					// The next byte is more complicated, we have 7 bits from the mantissa, and one bit from the exponent
					Second = (byte) ( ( Mantissa & 0b0000'0000'0111'1111'0000'0000'0000'0000 ) >> 16 );

					// Were going to take the exponent, shift the bits up by 7, so the bottom bit as at the top
					// then were going to do a bitwise or with the second bit, so set the top bit 
					Second |= (byte) ( ( Exponent << 7 ) );

					// Now, we need to shift the exponent to the right by 1, for the first byte
					First = (byte) ( ( (uint8)Exponent ) >> 1 );

					// Finally, set the very topmost bit based on the sign of the floating point value
					if( bNegative )
					{
						static const uint8 SignByte = 0b1000'0000;
						First |= SignByte;
					}
				}
			}

			// Now we have all 4 bytes constructed, we can pop them in the output byte vector
			if( bOutputAsLittleEndian )
			{
				Output.insert( Output.end(), { Fourth, Third, Second, First } );
			}
			else
			{
				Output.insert( Output.end(), { First, Second, Third, Fourth } );
			}
			*/
			HYPERION_VERIFY( sizeof( float ) == 4, "Size of float is unpexted" );
			
			const byte* dataPtr = reinterpret_cast<const byte*>( &In );
			if( bOutputAsLittleEndian == IsLittleEndian() )
			{
				Output.insert( Output.end(), dataPtr, dataPtr + 4 );
			}
			else
			{
				Output.insert( Output.end(), { dataPtr[ 3 ], dataPtr[ 2 ], dataPtr[ 1 ], dataPtr[ 0 ] } );
			}
		}


		/*
			Binary::DeserializeFloat
		*/
		bool Binary::DeserializeFloat( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, float& Output, bool bIsLittleEndian /* = false */  )
		{
			// Ensure we are being passed exactly 4 bytes
			if( std::distance( Begin, End ) != 4 )
			{
				Console::WriteLine( "[ERROR] Binary: Attempt to deserialize float, but was not passed exactly 4 bytes!" );
				return false;
			}

			/*
			// Read the bytes in from the vector, and take into account endianess now
			uint8 First, Second, Third, Fourth;

			if( bIsLittleEndian )
			{
				First	= *( Begin + 3 );
				Second	= *( Begin + 2 );
				Third	= *( Begin + 1 );
				Fourth	= *( Begin );
			}
			else
			{
				First	= *( Begin );
				Second	= *( Begin + 1 );
				Third	= *( Begin + 2 );
				Fourth	= *( Begin + 3 );
			}

			// First, we can extract the sign from the first bit of the first byte
			bool bIsNegative = (bool)( First & 0b1000'0000 );

			// Next, lets try and extraxt the exponent, from the first and second bytes
			uint8 Exponent = ( ( ( First & 0b0111'1111 ) << 1 ) | ( ( Second & 0b1000'0000 ) >> 7 ) ) - 127;

			// Finally, extract the mantissa from the second, third and fourth bytes
			uint32 Mantissa = ( (uint32) ( Second & 0b0111'1111 ) << 16 ) | (uint32)( Third << 8 ) | ( Fourth );
			float MantissaValue = 1.0f;
			uint32 Pos = 0;

			while( Pos < 21 )
			{
				if( ( Mantissa & ( 0x1 << ( 22 - Pos ) ) ) != 0 )
				{
					MantissaValue += ( 1.0f / powf( 2.0f, Pos + 1 ) );
				}

				Pos++;
			}

			// Now we can check for special cases
			if( Exponent == 0b0000'0000 )
			{
				// Zero
				Output = 0.f;
			}
			else if( Exponent == 0b1111'1111 && Mantissa == 0x00000000 )
			{
				if( bIsNegative )
				{
					// -INF
					Output = -std::numeric_limits< float >::infinity();

				}
				else
				{
					// +INF
					Output = std::numeric_limits< float >::infinity();
				}
			}
			else if( Exponent == 0b1111'1111 && Mantissa == 0b0000'0000'0111'1111'1111'1111'1111'1111 )
			{
				// NaN
				Output = std::numeric_limits< float >::quiet_NaN();
			}
			else
			{
				// Normal Number
				// We need to calculate the actual mantissa, we can do this by dividing the value we read by the max value of 23 bits
				// Then, we need to add 1.f to it, and divide by 2.f to get it back to the original range [0.5,1)
				//static const double MaxMantissa = pow( 2.0, 23 );
				//double RealMantissa = ( ( (double)Mantissa / MaxMantissa ) + 1.0 ) / 2.0;
				
				// Next, we need to calculate the output value
				//double FinalValue = RealMantissa * pow( 2.0, Exponent );
				double FinalValue = MantissaValue * pow( 2.0, Exponent );

				if( FinalValue > std::numeric_limits< float >::max() )
				{
					Output = std::numeric_limits< float >::infinity() * ( bIsNegative ? -1.0f : 1.0f );
				}
				else if( FinalValue < std::numeric_limits< float >::min() )
				{
					Output = 0.f;
				}
				else
				{
					// Value is in range....
					Output = (float)( bIsNegative ? -FinalValue : FinalValue );
				}
			}

			return true;
			*/

			HYPERION_VERIFY( sizeof( float ) == 4, "Size of float is unexpected" );

			if( bIsLittleEndian == IsLittleEndian() )
			{
				std::copy( Begin, End, reinterpret_cast<byte*>( &Output ) );
			}
			else
			{
				std::reverse_copy( Begin, End, reinterpret_cast<byte*>( &Output ) );
			}

			return true;
		}
		

		/*
			Binary::ReadSignedBinary
		*/
		int32 Binary::ReadSignedBinary( uint32 Input, uint8 BitCount )
		{
			// Basically, we can input 4 bytes of info (represented by uint32)
			// Then, specify how many bits the ACTUAL number is made from, and it can convert it into a int32 
			// with the proper value, taking into account twos compliment

			// We want to check if the top bit is set, meaning its a negative number, otherwise we can just push the
			// BitCount number of bits into an int32 and return it, as long as the BitCount is valid
			if( BitCount > 32 || BitCount < 2 )
			{
				Console::WriteLine( "[ERROR] Binary: ReadSignedBinary was called with an invalid BitCount!" );
				return 0;
			}

			// We need to convert 'BitCount' to a uint32 with the bit set in the proper position
			uint32 BitCheck = (uint32) pow( 2.f, BitCount - 1 );

			// Check if this number is negative or not, if so, were going to set the negative component to 2^(BitCount-1)
			int32 NegativeComponent = ( ( Input | BitCheck ) != 0 ) ? -( (int32)BitCheck ) : 0;

			// Remove the 'negative place' from the number, we can do this by XOR'ing Input and BitCheck
			int32 PositiveComponent = Input ^ BitCheck;

			// And we can produce the final output by summing the positive and negative components
			return NegativeComponent + PositiveComponent;
		}

		/*
			Binary::GenerateSignedBinary
		*/
		uint32 Binary::GenerateSignedBinary( int32 Input, uint8 BitCount )
		{
			// We want to create a signed number from a set number of bits
			// First though we want to make sure this number will actually fit in this number of bits
			// Calculate the max and min values that can fit in this number of bits
			int32 MaxValue = (int32) pow( 2.f, BitCount - 1 ) - 1;
			int32 MinValue = ( MaxValue * -1 ) - 1;

			if( BitCount > 32 || BitCount < 2 )
			{
				Console::WriteLine( "[ERROR] Binary: GenerateSignedBinary was called with an invalid BitCount (", (uint32)BitCount, "!" );
				return 0;
			}
			else if( Input > MaxValue || Input < MinValue )
			{
				Console::WriteLine( "[ERROR] Binary: GenerateSignedBinary was called with an input that doesnt match the bit count!\n\tWith a ", (uint32)BitCount, "-bit int.. Max=", MaxValue, " Min=", MinValue, "" );
				return 0;
			}

			// Check if were writing a positive or negative number
			if( Input >= 0 )
			{
				// We just need to store the number in the output and were done, we already checked the range
				return Input;
			}
			else
			{
				// Now, we need to create a uint32 with the BitCount'th bit set to 1
				uint32 TopBit = (uint32) pow( 2.f, BitCount - 1 );

				// Now, were going to store the positive component, flip the bits and perform a bitwise AND to clear unwanted set bits
				uint32 Output = abs( Input );
				Output = ~Output;

				uint32 BitMask = TopBit - 1;
				Output = Output & BitMask;

				// Add 1 to finish the twos compliment process
				Output += 1;

				// Finally, flip the top bit
				Output = Output | TopBit;

				return Output;
			}
		}

		void Binary::DeserializeFloat( std::vector< byte >::const_iterator Begin, float& Out, bool bReadAsLittleEndian )
		{
			DeserializeFloat( Begin, Begin + 4, Out, bReadAsLittleEndian );
		}

		/*
			Binary::SerializeDouble
		*/
		void Binary::SerializeDouble( const double& In, std::vector< byte >& Output, bool bOutputAsLittleEndian /* = false */  )
		{
			/*
			// Were going to manually build the 8 output bytes.. the order will be
			// [MSB] First, Second, Third, Fourth, Fifth, Sixth, Seventh, Eighth [LSB]
			// Sign Bit: 1
			// Exponent Bits: 11
			// Mantissa Bits: 52
			byte First, Second, Third, Fourth, Fifth, Sixth, Seventh, Eighth;
			First = Second = Third = Fourth = Fifth = Sixth = Seventh = Eighth = byte( 0b0000'0000 );

			// Check for NaN or INF
			if( isnan( In ) )
			{
				// NaN
				// For this situation, we want the exponent to be MAX and mantissa to be MAX, sign bit is 0
				First = byte( 0b0111'1111 );
				Second = Third = Fourth = Fifth = Sixth = Seventh = Eighth 
					= byte( 0b1111'1111 );
			}
			else if( isinf( In ) )
			{
				if( signbit( In ) )
				{
					// -INF
					// For this, we want the Exponent to be MAX, sign bit to be 0
					First = byte( 0b1111'1111 );
					Second = byte( 0b1111'0000 );
					Third = Fourth = Fifth = Sixth = Seventh = Eighth 
						= byte( 0b0000'0000 );
				}
				else
				{
					// +INF
					// For this, we want the exponent to be MAX, sign bit to be 1
					First = byte( 0b0111'1111 );
					Second = byte( 0b1111'0000 );
					Third = Fourth = Fifth = Sixth = Seventh = Eighth
						= byte( 0b0000'0000 );
				}
			}
			else
			{
				// Now we need to split the double into a mantissa, exponent and sign
				int32 Exponent;
				double Fraction = frexp( In, &Exponent );
				bool bIsNegative = signbit( In );

				// Next, we need to figure out the max and min values we can store in an 11-bit signed integer
				int32 MaxValue = (int32)pow( 2.f, 10 ) - 1;
				int32 MinValue = ( MaxValue * -1 ) - 1;

				if( Exponent > MaxValue )
				{
					// Overflow!
					// If we underflow, we will just leave the double as 0.0
					// For this case, were going to set to +/- INF
					if( bIsNegative )
					{
						First	= byte( 0b0111'1111 );
						Second	= byte( 0b1111'0000 );
						Third = Fourth = Fifth = Sixth = Seventh = Eighth 
								= byte( 0b0000'0000 );
					}
					else
					{
						First	= byte( 0b1111'1111 );
						Second	= byte( 0b1111'0000 );
						Third = Fourth = Fifth = Sixth = Seventh = Eighth
								= byte( 0b0000'0000 );
					}
				}
				else if( Exponent >= MinValue )
				{
					// The Fraction is in the range [0.5,1) and can be positive or negative
					// But, we want to ditch the sign, and change the range to [0,1]
					double Mantissa = ( abs( Fraction ) * 2.0 ) - 1.0;

					// Now we need to get the max value of a 52 bit unsigned int, and multiply against the scalar we created
					uint64 ScaledMantissa = (uint64) trunc( ldexp( Mantissa, 52 ) );

					// The bottom 6 bytes are just the mantissa
					Eighth	= (byte) ( ( ScaledMantissa & 0x00'00'00'00'00'00'00'FF ) );
					Seventh	= (byte) ( ( ScaledMantissa & 0x00'00'00'00'00'00'FF'00 )	>> 8 );
					Sixth	= (byte) ( ( ScaledMantissa & 0x00'00'00'00'00'FF'00'00 )	>> 16 );
					Fifth	= (byte) ( ( ScaledMantissa & 0x00'00'00'00'FF'00'00'00 )	>> 24 );
					Fourth	= (byte) ( ( ScaledMantissa & 0x00'00'00'FF'00'00'00'00 )	>> 32 );
					Third	= (byte) ( ( ScaledMantissa & 0x00'00'FF'00'00'00'00'00 )	>> 40 );

					// Now, its more complicated, the seventh byte has 4 bits from the mantissa, and 4 bits from the exponent
					Second = (byte) ( ( ScaledMantissa & 0x00'0F'00'00'00'00'00'00 )	>> 48 );

					// Now, we need to ensure the exponent is formatted as a int11 and not an int32
					uint32 ExponentData = GenerateSignedBinary( Exponent, 11 );
					Second |= (byte) ( ( ExponentData & 0b0000'0000'0000'0000'0000'0000'0000'1111 ) << 4 );

					// The eigth byte is the exponent, and the top bit indicating the sign
					// We want to ensure its turned off
					First = (byte) ( ( ExponentData & 0b0000'0000'0000'0000'0000'0111'1111'0000 ) >> 4 );

					static const byte SignByte = byte( 0b1000'0000 );

					if( bIsNegative )
					{
						First |= SignByte;
					}
					else
					{
						First &= ~SignByte;
					}
				}
			}

			if( bOutputAsLittleEndian )
			{
				Output.insert( Output.end(), { Eighth, Seventh, Sixth, Fifth, Fourth, Third, Second, First } );
			}
			else
			{
				Output.insert( Output.end(), { First, Second, Third, Fourth, Fifth, Sixth, Seventh, Eighth } );
			}
			*/
			HYPERION_VERIFY( sizeof( double ) == 8, "Size of double is unexpected" );

			const byte* dataPtr = reinterpret_cast<const byte*>( &Output );
			if( bOutputAsLittleEndian == IsLittleEndian() )
			{
				Output.insert( Output.end(), dataPtr, dataPtr + 8 );
			}
			else
			{
				Output.insert( Output.end(), { dataPtr[ 7 ], dataPtr[ 6 ], dataPtr[ 5 ], dataPtr[ 4 ], dataPtr[ 3 ], dataPtr[ 2 ], dataPtr[ 1 ], dataPtr[ 0 ] } );
			}

		}


		/*
			Binary::DeserilalizeDouble
		*/
		bool Binary::DeserializeDouble( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, double& Output, bool bReadAsLittleEndian /* = false */  )
		{
			/*
			// Set default output value
			Output = 0.0;

			// Validate the iterator distance
			if( std::distance( Begin, End ) != 8 )
			{
				Console::WriteLine( "[ERROR] Binary: Attempt to deserialize double.. but the distance between iterators is not 8!" );
				return false;
			}

			byte First, Second, Third, Fourth, Fifth, Sixth, Seventh, Eighth;
			First = Second = Third = Fourth = Fifth = Sixth = Seventh = Eighth 
				= byte( 0b0000'0000 );

			// Manually read the in bytes, take endianess into account
			if( bReadAsLittleEndian )
			{
				First		= *( Begin + 7 );
				Second		= *( Begin + 6 );
				Third		= *( Begin + 5 );
				Fourth		= *( Begin + 4 );
				Fifth		= *( Begin + 3 );
				Sixth		= *( Begin + 2 );
				Seventh		= *( Begin + 1 );
				Eighth		= *( Begin + 0 );
			}
			else
			{
				First		= *( Begin + 0 );
				Second		= *( Begin + 1 );
				Third		= *( Begin + 2 );
				Fourth		= *( Begin + 3 );
				Fifth		= *( Begin + 4 );
				Sixth		= *( Begin + 5 );
				Seventh		= *( Begin + 6 );
				Eighth		= *( Begin + 7 );
			}

			// Now we need to read in the double components from the bytes
			// The structure is as follows:
			// Sign: 1 bit
			// Exponent: 11 bits
			// Mantissa: 52 bits
			bool bIsNegative = (bool)( First & 0b1000'0000 );
			uint32 ExponentData = ( (uint32) ( First & 0b0111'1111 ) << 4 ) | ( ( Second & 0b1111'0000 ) >> 4 );
			uint64 MantissaData =	( (uint64) ( Second & 0b0000'1111 ) << 48 ) |
									( (uint64) Third << 40 ) |
									( (uint64) Fourth << 32 ) |
									( (uint64) Fifth << 24 ) |
									( (uint64) Sixth << 16 ) |
									( (uint64) Seventh << 8 ) |
									( Eighth );
			
			// Now, were going to check for 'special' values
			if( ExponentData == 0x00000000 )
			{
				// Zero
				return true;
			}
			else if( ExponentData == 0b0000'0000'0000'0000'0000'0111'1111'1111 && MantissaData == 0x0000000000000000 )
			{
				if( bIsNegative )
				{
					// -INF
					Output = -std::numeric_limits< double >::infinity();
				}
				else
				{
					// +INF
					Output = std::numeric_limits< double >::infinity();
				}
			}
			else if( ExponentData == 0b0000'0000'0000'0000'0000'0111'1111'1111 && 
				MantissaData == 0b0000'0000'0000'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111 )
			{
				// NaN
				Output = std::numeric_limits< double >::quiet_NaN();
			}
			else
			{
				// Normal Number
				// We need to calculate the actual components of the double from the data we have read already
				// First, we have a function to read the 11-bit signed int into an int32
				int32 Exponent = ReadSignedBinary( ExponentData, 11 );

				// Next, we need to calculate the fraction portion of the double, for this we need to divide Mantissa/MaxMantissa, add 1 and div by 2
				static const double MaxMantissa = pow( 2.0, 52 );
				double Mantissa = ( ( (double)MantissaData / MaxMantissa ) + 1.0 ) / 2.0;

				// Ensure Mantissa is within valid range
				if( Mantissa < 0.5 || Mantissa > 1.0 )
				{
					Console::WriteLine( "[ERROR] Binary: Failed to deserialize double.. mantissa was out of range!" );
					return false;
				}

				double FinalValue = Mantissa * pow( 2.0, Exponent );
				if( FinalValue > std::numeric_limits< double >::max() )
				{
					Output = std::numeric_limits< double >::infinity() * ( bIsNegative ? -1.0 : 1.0 );
				}
				else if( FinalValue < std::numeric_limits< double >::min() )
				{
					Output = 0.0;
				}
				else
				{
					Output = bIsNegative ? -FinalValue : FinalValue;
				}
			}

			return true;
			*/
			
			HYPERION_VERIFY( sizeof( double ) == 8, "Size of double is unexpected" );

			if( bReadAsLittleEndian == IsLittleEndian() )
			{
				std::copy( Begin, End, reinterpret_cast<byte*>( &Output ) );
			}
			else
			{
				std::reverse_copy( Begin, End, reinterpret_cast< byte* >( &Output ) );
			}

			return true;
		}

		void Binary::DeserializeDouble( std::vector< byte >::const_iterator Begin, double& Out, bool bReadAsLittleEndian )
		{
			DeserializeDouble( Begin, Begin + 8, Out, bReadAsLittleEndian );
		}

		/*
			Binary::SerializeUInt8
		*/
		void Binary::SerializeUInt8( const uint8& In, std::vector< byte >& Output )
		{
			Output.push_back( (byte) In );
		}

		/*
			Binary::DeserializeUInt8
		*/
		void Binary::DeserializeUInt8( std::vector< byte >::const_iterator Where, uint8& Output )
		{
			Output = (uint8) *Where;
		}

		/*
			Binary::SerializeInt8
		*/
		void Binary::SerializeInt8( const int8& In, std::vector< byte >& Output )
		{
			SerializeUInt8( (uint8&) In, Output );
		}

		/*
			Binary::DeserializeInt8
		*/
		void Binary::DeserializeInt8( std::vector< byte >::const_iterator Where, int8& Output )
		{
			DeserializeUInt8( Where, (uint8&) Output );
		}

		/*
			Binary::SerializeUInt16
		*/
		void Binary::SerializeUInt16( const uint16& In, std::vector< byte >& Output, bool bOutputAsLittleEndian /* = false */  )
		{
#ifdef HYPERION_SERIALIZE_INT_FAST

			if( IsLittleEndian() != bOutputAsLittleEndian )
			{
				std::reverse_copy(
					reinterpret_cast< const byte* >( &In ),
					reinterpret_cast< const byte* >( &In ) + 2,
					std::back_inserter( Output )
				);
			}
			else
			{
				std::copy(
					reinterpret_cast< const byte* >( &In ),
					reinterpret_cast< const byte* >( &In ) + 2,
					std::back_inserter( Output )
				);
			}
#elif
			byte Upper = (byte)( ( In & 0xFF00 ) >> 8 );
			byte Lower = (byte)( In & 0x00FF );

			if( bOutputAsLittleEndian )
			{
				Output.insert( Output.end(), { Lower, Upper } );
			}
			else
			{
				Output.insert( Output.end(), { Upper, Lower } );
			}
#endif
		}

		/*
			Binary::DeserializeUInt16
		*/
		bool Binary::DeserializeUInt16( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, uint16& Out, bool bReadAsLittleEndian /* = false */  )
		{
			Out = 0;

			if( std::distance( Begin, End ) != 2 )
			{
				Console::WriteLine( "[ERROR] Binary: Attempt to deserialize uint16, but given byte list is not equal to 2 in length!" );
				return false;
			}

			byte Upper, Lower;
			if( bReadAsLittleEndian )
			{
				Upper = *( Begin + 1 );
				Lower = *( Begin + 0 );
			}
			else
			{
				Upper = *( Begin + 0 );
				Lower = *( Begin + 1 );
			}

			// Now, were going to rebuild a uint16 from the two bytes we read
			Out = Upper << 8 | Lower;
			return true;
		}

		void Binary::DeserializeUInt16( std::vector< byte >::const_iterator Begin, uint16& Out, bool bReadAsLittleEndian )
		{
			DeserializeUInt16( Begin, Begin + 2, Out, bReadAsLittleEndian );
		}

		/*
			Binary::SerializeInt16
		*/
		void Binary::SerializeInt16( const int16& In, std::vector< byte >& Output, bool bOutputAsLittleEndian /* = false */  )
		{
			SerializeUInt16( (uint16&) In, Output, bOutputAsLittleEndian );
		}

		/*
			Binary::DeserializeInt16
		*/
		bool Binary::DeserializeInt16( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, int16& Out, bool bReadAsLittleEndian /* = false */  )
		{
			return DeserializeUInt16( Begin, End, (uint16&) Out, bReadAsLittleEndian );
		}

		void Binary::DeserializeInt16( std::vector< byte >::const_iterator Begin, int16& Out, bool bReadAsLittleEndian )
		{
			DeserializeInt16( Begin, Begin + 2, Out, bReadAsLittleEndian );
		}

		/*
			Binary::SerializeUInt32
		*/
		void Binary::SerializeUInt32( const uint32& In, std::vector< byte >& Output, bool bOutputAsLittleEndian /* = false */  )
		{
#ifdef HYPERION_SERIALIZE_INT_FAST

			if( IsLittleEndian() != bOutputAsLittleEndian )
			{
				std::reverse_copy(
					reinterpret_cast< const byte* >( &In ),
					reinterpret_cast< const byte* >( &In ) + 4,
					std::back_inserter( Output )
				);
			}
			else
			{
				std::copy(
					reinterpret_cast< const byte* >( &In ),
					reinterpret_cast< const byte* >( &In ) + 4,
					std::back_inserter( Output )
				);
			}
#elif
			// Generate the four bytes needed to represent this value
			byte First		= (byte)( ( In & 0xFF000000 ) >> 24 );
			byte Second		= (byte)( ( In & 0x00FF0000 ) >> 16 );
			byte Third		= (byte)( ( In & 0x0000FF00 ) >> 8 );
			byte Fourth		= (byte)( In & 0x000000FF );

			// Now we want to store them in the output vector, based on endian requirments
			if( bOutputAsLittleEndian )
			{
				Output.insert( Output.end(), { Fourth, Third, Second, First } );
			}
			else
			{
				Output.insert( Output.end(), { First, Second, Third, Fourth } );
			}
#endif
		}

		/*
			Binary::DeserializeUInt32
		*/
		bool Binary::DeserializeUInt32( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, uint32& Out, bool bReadAsLittleEndian /* = false */  )
		{
			Out = 0;

			if( std::distance( Begin, End ) != 4 )
			{
				Console::WriteLine( "[ERROR] Binary: Attempt to read 32-bit integer, but the distance between the iterators given was not 4 bytes!" );
				return false;
			}

			// Read the bytes from the vector taking into account endian order
			byte First, Second, Third, Fourth;
			if( bReadAsLittleEndian )
			{
				First	= *( Begin + 3 );
				Second	= *( Begin + 2 );
				Third	= *( Begin + 1 );
				Fourth	= *( Begin + 0 );
			}
			else
			{
				First	= *( Begin + 0 );
				Second	= *( Begin + 1 );
				Third	= *( Begin + 2 );
				Fourth	= *( Begin + 3 );
			}

			// Rebuild the uint32 from the bytes we read
			Out = First << 24 | Second << 16 | Third << 8 | Fourth;

			return true;
		}

		void Binary::DeserializeUInt32( std::vector< byte >::const_iterator Begin, uint32& Out, bool bReadAsLittleEndian )
		{
			DeserializeUInt32( Begin, Begin + 4, Out, bReadAsLittleEndian );
		}


		/*
			Binary::SerializeInt32
		*/
		void Binary::SerializeInt32( const int32& In, std::vector< byte >& Output, bool bOutputAsLittleEndian /* = false */  )
		{
			SerializeUInt32( (uint32&) In, Output, bOutputAsLittleEndian );
		}

		/*
			Binary::DeserializeInt32
		*/
		bool Binary::DeserializeInt32( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, int32& Out, bool bReadAsLittleEndian /* = false */  )
		{
			return DeserializeUInt32( Begin, End, (uint32&) Out, bReadAsLittleEndian );
		}

		void Binary::DeserializeInt32( std::vector< byte >::const_iterator Begin, int32& Out, bool bReadAsLittleEndian )
		{
			DeserializeInt32( Begin, Begin + 4, Out, bReadAsLittleEndian );
		}

		/*
			Binary::SerializeUInt64
		*/
		void Binary::SerializeUInt64( const uint64& In, std::vector< byte >& Output, bool bOutputAsLittleEndian /* = false */  )
		{
#ifdef HYPERION_SERIALIZE_INT_FAST
			
			if( IsLittleEndian() != bOutputAsLittleEndian )
			{
				std::reverse_copy(
					reinterpret_cast< const byte* >( &In ),
					reinterpret_cast< const byte* >( &In ) + 8,
					std::back_inserter( Output )
				);
			}
			else
			{
				std::copy(
					reinterpret_cast< const byte* >( &In ),
					reinterpret_cast< const byte* >( &In ) + 8,
					std::back_inserter( Output )
				);
			}
#elif
			// Read all of the bytes from the input long int
			byte First		= (byte)( ( In & 0xFF'00'00'00'00'00'00'00 ) >> 56 );
			byte Second		= (byte)( ( In & 0x00'FF'00'00'00'00'00'00 ) >> 48 );
			byte Third		= (byte)( ( In & 0x00'00'FF'00'00'00'00'00 ) >> 40 );
			byte Fourth		= (byte)( ( In & 0x00'00'00'FF'00'00'00'00 ) >> 32 );
			byte Fifth		= (byte)( ( In & 0x00'00'00'00'FF'00'00'00 ) >> 24 );
			byte Sixth		= (byte)( ( In & 0x00'00'00'00'00'FF'00'00 ) >> 16 );
			byte Seventh	= (byte)( ( In & 0x00'00'00'00'00'00'FF'00 ) >> 8 );
			byte Eighth		= (byte) ( In & 0x00'00'00'00'00'00'00'FF );

			if( bOutputAsLittleEndian )
			{
				Output.insert( Output.end(), { Eighth, Seventh, Sixth, Fifth, Fourth, Third, Second, First } );
			}
			else
			{
				Output.insert( Output.end(), { First, Second, Third, Fourth, Fifth, Sixth, Seventh, Eighth } );
			}
#endif
		}

		/*
			Binary::DeserializeUInt16
		*/
		bool Binary::DeserializeUInt64( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, uint64& Out, bool bReadAsLittleEndian /* = false */  )
		{
			Out = 0;

			// Check byte count
			if( std::distance( Begin, End ) != 8 )
			{
				Console::WriteLine( "[ERROR] Binary: Attempt to deserialize uint64 with the wrong number of bytes!" );
				return false;
			}

			// Read bytes from the vector, taking endian order into account
			byte First, Second, Third, Fourth, Fifth, Sixth, Seventh, Eighth;
			if( bReadAsLittleEndian )
			{
				First		= *( Begin + 7 );
				Second		= *( Begin + 6 );
				Third		= *( Begin + 5 );
				Fourth		= *( Begin + 4 );
				Fifth		= *( Begin + 3 );
				Sixth		= *( Begin + 2 );
				Seventh		= *( Begin + 1 );
				Eighth		= *( Begin + 0 );
			}
			else
			{
				First		= *( Begin + 0 );
				Second		= *( Begin + 1 );
				Third		= *( Begin + 2 );
				Fourth		= *( Begin + 3 );
				Fifth		= *( Begin + 4 );
				Sixth		= *( Begin + 5 );
				Seventh		= *( Begin + 6 );
				Eighth		= *( Begin + 7 );
			}

			// Finally, lets rebuilt the uint64
			Out = (uint64)First << 56 | (uint64)Second << 48 | (uint64)Third << 40 | (uint64)Fourth << 32 | (uint64)Fifth << 24 | (uint64)Sixth << 16 | (uint64)Seventh << 8 | (uint64)Eighth;
			return true;
		}

		void Binary::DeserializeUInt64( std::vector< byte >::const_iterator Begin, uint64& Out, bool bReadAsLittleEndian )
		{
			DeserializeUInt64( Begin, Begin + 8, Out, bReadAsLittleEndian );
		}

		/*
			Binary::SerializeInt64
		*/
		void Binary::SerializeInt64( const int64& In, std::vector< byte >& Output, bool bWriteAsLittleEndian /* = false */  )
		{
			SerializeUInt64( (uint64&) In, Output, bWriteAsLittleEndian );
		}

		/*
			Binary::DeserializeInt64
		*/
		bool Binary::DeserializeInt64( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End, int64& Out, bool bReadAsLittleEndian /* = false */  )
		{
			return DeserializeUInt64( Begin, End, (uint64&) Out, bReadAsLittleEndian );
		}

		void Binary::DeserializeInt64( std::vector< byte >::const_iterator Begin, int64& Out, bool bReadAsLittleEndian )
		{
			DeserializeInt64( Begin, Begin + 8, Out, bReadAsLittleEndian );
		}

		/*
			Binary::SerializeBoolean
		*/
		void Binary::SerializeBoolean( bool In, std::vector< byte >& Output )
		{
			Output.push_back( In ? 0x01 : 0x00 );
		}

		/*
			Binary::DeserializeBoolean
		*/
		void Binary::DeserializeBoolean( std::vector< byte >::const_iterator Where, bool& Out )
		{
			Out = ( *Where == 0x00 ) ? false : true;
		}

}