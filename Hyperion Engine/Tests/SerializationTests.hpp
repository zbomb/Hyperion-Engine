/*==================================================================================================
	Hyperion Engine
	Tests/SerializationTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

// Engine Includes
#include "Hyperion/Core/Library/Binary.h"
#include "Hyperion/Core/Library/Serialization.h"

// STL Includes
#include <chrono>
#include <iostream>


namespace Hyperion
{
namespace Tests
{
	
	void RunArchiveTest()
	{
		std::cout << "\n---------------------------------------------------------------------------------------------\n[TEST] Running archive serialization test...\n";

		// Create a buffer to hold the outputted data
		std::vector< byte > Buffer;
		uint32 HeaderCount = 0;
		uint32 BodyCount = 0;
		{
			auto Ar = ArchiveWriter( Buffer );

			// First, lets try and write 3 uint32 values and inspect the output
			Ar.WriteUInt32( 0x020A10FF );
			Ar.WriteUInt32( 0x0120000F );
			Ar.WriteUInt32( 0x000FA123 );

			HeaderCount = Ar.GetHeaderSize();
			BodyCount = Ar.GetContentSize();

			Ar.Flush();
		}

		// Now, lets write out the data we have....
		std::cout << "----> Archive Basic Test:\n\tHeader Count: ";
		std::cout << HeaderCount;
		std::cout << "\n\tContent Count: ";
		std::cout << BodyCount;
		std::cout << "\n";
		Hyperion::Binary::PrintBin( Buffer.begin(), Buffer.end() );

		std::cout << std::endl;
		std::cout << "----> Archive Serialization Test Complete!\n";
		std::cout << "---------------------------------------------------------------------------------------------\n";
	}

	void RunFloatingPointSerializationTest()
	{
		std::cout << "\n---------------------------------------------------------------------------------------------\n[TEST] Running floating point serialization test...\n";
		
		double TestValue = 123.33;
		std::cout << "----> Testing Double Serialization:\n\tTest Value: " << TestValue << "\n";
		std::cout << "Serializing...\n";

		std::vector< byte > DoubleData;
		DoubleData.reserve( 8 );

		// Serialize Double
		std::chrono::time_point< std::chrono::high_resolution_clock > DoubleEncStart = std::chrono::high_resolution_clock::now();
		Hyperion::Binary::SerializeDouble( TestValue, DoubleData );
		std::chrono::time_point< std::chrono::high_resolution_clock > DoubleEncEnd = std::chrono::high_resolution_clock::now();

		std::cout << "Serialized Double Data:\n";
		Hyperion::Binary::PrintBin( DoubleData.begin(), DoubleData.end() );

		std::cout << "\n----> Testing Double Deserialization:\n";
		double Output = 0.0;

		// Deserialize Double
		std::chrono::time_point< std::chrono::high_resolution_clock > DoubleDecStart = std::chrono::high_resolution_clock::now();
		bool bResult = Hyperion::Binary::DeserializeDouble( DoubleData.begin(), DoubleData.end(), Output );
		std::chrono::time_point< std::chrono::high_resolution_clock > DoubleDecEnd = std::chrono::high_resolution_clock::now();

		std::cout << "\tSuccess?: " << ( bResult ? "Yes" : "No" ) << "\n";
		if( bResult )
		{
			std::cout << "\tOutput: " << Output << std::endl;

			// Check if the input and output are the same
			std::cout << "\tEquivalent?: " << ( TestValue == Output ? "Yes" : "No" );
		}

		std::cout << std::endl << std::endl;

		// 32-bit Floating Point Tests
		float TestFloat = 123.33f;

		std::cout << "----> Testing Float Serialization:\n\tTest Value: " << TestFloat << "\n";
		std::cout << "Serializing...\n";

		std::vector< byte > FloatData;
		FloatData.reserve( 4 );

		// Serialize Float
		std::chrono::time_point< std::chrono::high_resolution_clock > FloatEncStart = std::chrono::high_resolution_clock::now();
		Hyperion::Binary::SerializeFloat( TestFloat, FloatData );
		std::chrono::time_point< std::chrono::high_resolution_clock > FloatEncEnd = std::chrono::high_resolution_clock::now();

		std::cout << "Serialized Float Data:\n";
		Hyperion::Binary::PrintBin( FloatData.begin(), FloatData.end() );

		std::cout << "\n---->Testing Float Deserialization:\n";
		float OutFloat = 0.f;

		// Deserialize Float
		std::chrono::time_point< std::chrono::high_resolution_clock > FloatDecStart = std::chrono::high_resolution_clock::now();
		bResult = Hyperion::Binary::DeserializeFloat( FloatData.begin(), FloatData.end(), OutFloat );
		std::chrono::time_point< std::chrono::high_resolution_clock > FloatDecEnd = std::chrono::high_resolution_clock::now();

		std::cout << "\tSuccess?: " << ( bResult ? "Yes" : "No" ) << std::endl;
		if( bResult )
		{
			std::cout << "\tOutput: " << OutFloat << std::endl;

			// Check if output is the same as input
			std::cout << "\tEquivalent?: " << ( TestFloat == OutFloat ? "Yes" : "No" );
		}

		std::cout << std::endl << std::endl;
		std::cout << "----> Performance Data:\n\tFloat Encode: ";
		std::cout << std::chrono::duration_cast< std::chrono::microseconds >( FloatEncEnd - FloatEncStart ).count() << " microsecs\n";
		std::cout << "\tFloat Decode: ";
		std::cout << std::chrono::duration_cast< std::chrono::microseconds >( FloatDecEnd - FloatDecStart ).count() << " microsecs\n";
		std::cout << "\tDouble Encode: ";
		std::cout << std::chrono::duration_cast< std::chrono::microseconds >( DoubleEncEnd - DoubleEncStart ).count() << " microsecs\n";
		std::cout << "\tDouble Decode: ";
		std::cout << std::chrono::duration_cast< std::chrono::microseconds >( DoubleDecEnd - DoubleDecStart ).count() << " microsecs\n";

		std::cout << std::endl;
		std::cout << "----> Floating Point Serialization Test Complete!\n";
		std::cout << "---------------------------------------------------------------------------------------------\n";
	}

}
}