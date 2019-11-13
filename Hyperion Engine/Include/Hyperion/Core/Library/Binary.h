/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Library/Binary.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include <vector>

// There are two serialization methods available.. were going to use the 'fast' method for now
// For integral types, were just going to copy the memory and correct the endianess, instead of 
// manually building and arranging the output bytes.. meaning faster execution
#define HYPERION_SERIALIZE_INT_FAST


namespace Hyperion
{
	class Binary
	{
	public:

		Binary() = delete;


		/*
			Binary::IsLittleEndian
			* Checks if the system is a little-endian system
		*/
		static bool IsLittleEndian();

		/*
			Binary::Print(
			* Writes a series of bytes to the console, in hexadecimal
		*/
		static void PrintHex( const std::vector< byte >::const_iterator Begin, const std::vector< byte >::const_iterator End );

		/*
			Binary::PrintBin
			* Writes a series of bytes to the console in binary
		*/
		static void PrintBin( const std::vector< byte >::iterator Begin, const std::vector< byte >::iterator End );

		/*
			Binary::SerializeFloat
			* Converts a float to a set of bytes using IEEE 754
			* This is totally platform independent
			* Stores the float using 4 bytes, appended to the end of the vector specified
			* Can store as either big-endian or little-endian
		*/
		static void SerializeFloat( const float& In, std::vector< byte >& Output, bool bOutputAsLittleEndian = false );


		/*
			Binary::DeserializeFloat
			* Takes in 4 bytes from a vector, and returns a float
			* The float must have been encoded using IEE 754
			* If the float is stored in memory using little-endian, set bIsLittleEndian to true
			* The SerializeFloat function stores as big-endian
		*/
		static bool DeserializeFloat( std::vector< byte >::iterator Begin, std::vector< byte >::iterator End, float& Output, bool bIsLittleEndian = false );
		
		/*
			Binary::ReadSignedBinary
			* Reads a signed integer that takes up an unusual number of bits
			* For example, this function can interpret the 4 bytes you pass in as an int11
		*/
		static int32 ReadSignedBinary( uint32 Input, uint8 BitCount );

		/*
			Binary::GenerateSignedBinary
			* Takes a signed integer, and stores it within the set number of bits
			* For example, it can write an int11
		*/
		static uint32 GenerateSignedBinary( int32 Input, uint8 BitCount );

		/*
			Binary::SerializeDouble
			* Takes a double, and serializes it into 8 bytes in the Output vector
			* The bytes are inserted at the end of the vector
		*/
		static void SerializeDouble( const double& In, std::vector< byte >& Output, bool bOutputAsLittleEndian = false );

		/*
			Binary::DeserilalizeDouble
			* Takes in 8 bytes from a byte vector, and returns a double
		*/
		static bool DeserializeDouble( std::vector< byte >::iterator Begin, std::vector< byte >::iterator End, double& Output, bool bReadAsLittleEndian = false );

		/*
			Binary::SerializeUInt8
			* Takes an 8 bit unsigned integer and adds a byte to the target byte vector representing this value
		*/
		static void SerializeUInt8( const uint8& In, std::vector< byte >& Output );

		/*
			Binary::DeserializeUInt8
			* Reads a uint8 from the vector position indicated
		*/
		static void DeserializeUInt8( std::vector< byte >::iterator Where, uint8& Output );

		/*
			Binary::SerializeInt8;
			* Serializes an 8 bit signed integer into the target byte vector
		*/
		static void SerializeInt8( const int8& In, std::vector< byte >& Output );

		/*
			Binary::DeserializeInt8
			* Takes a byte from the byte vector iterator, and returns it as an int8
		*/
		static void DeserializeInt8( std::vector< byte >::iterator Where, int8& Output );

		/*
			Binary::SerializeUInt16
			* Takes in a 16-bit unsigned integer, and writes it as 2 bytes into the output vector
			* Can write output as little endian, set last parameter to true
		*/
		static void SerializeUInt16( const uint16& In, std::vector< byte >& Output, bool bOutputAsLittleEndian = false );

		/*
			Binary::DeserializeUInt16
			* Takes in two bytes from a byte vector, and outputs an unsigned 16-bit integer
		*/
		static bool DeserializeUInt16( std::vector< byte >::iterator Begin, std::vector< byte >::iterator End, uint16& Out, bool bReadAsLittleEndian = false );

		/*
			Binary::SerializeInt16
			* Takes a signed 16-bit integer, and puts two bytes representing its value into the output vector
		*/
		static void SerializeInt16( const int16& In, std::vector< byte >& Output, bool bOutputAsLittleEndian = false );

		/*
			Binary::DeserializeInt16
			* Takles 2 bytes from the target byte vector, and reads it as a 16-bit signed integer
		*/
		static bool DeserializeInt16( std::vector< byte >::iterator Begin, std::vector< byte >::iterator End, int16& Out, bool bReadAsLittleEndian = false );

		/*
			Binary::SerializeUInt32
			* Takes a 32-bit unsigned integer, and inserts the 4 bytes representing its value to the end of the specified vector
		*/
		static void SerializeUInt32( const uint32& In, std::vector< byte >& Output, bool bOutputAsLittleEndian = false );

		/*
			Binary::DeserializeUInt32
		*/
		static bool DeserializeUInt32( std::vector< byte >::iterator Begin, std::vector< byte >::iterator End, uint32& Out, bool bReadAsLittleEndian = false );

		/*
			Binary::SerializeInt32
		*/
		static void SerializeInt32( const int32& In, std::vector< byte >& Output, bool bOutputAsLittleEndian = false );

		/*
			Binary::DeserializeInt32
		*/
		static bool DeserializeInt32( std::vector< byte >::iterator Begin, std::vector< byte >::iterator End, int32& Out, bool bReadAsLittleEndian = false );

		/*
			Binary::SerializeUInt64
		*/
		static void SerializeUInt64( const uint64& In, std::vector< byte >& Output, bool bOutputAsLittleEndian = false );

		/*
			Binary::DeserializeUInt16
		*/
		static bool DeserializeUInt64( std::vector< byte >::iterator Begin, std::vector< byte >::iterator End, uint64& Out, bool bReadAsLittleEndian = false );

		/*
			Binary::SerializeInt64
		*/
		static void SerializeInt64( const int64& In, std::vector< byte >& Output, bool bWriteAsLittleEndian = false );

		/*
			Binary::DeserializeInt64
		*/
		static bool DeserializeInt64( std::vector< byte >::iterator Begin, std::vector< byte >::iterator End, int64& Out, bool bReadAsLittleEndian = false );

		/*
			Binary::SerializeBoolean
		*/
		static void SerializeBoolean( bool In, std::vector< byte >& Output );

		/*
			Binary::DeserializeBoolean
		*/
		static void DeserializeBoolean( std::vector< byte >::iterator Where, bool& Out );

	};
}