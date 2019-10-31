/*==================================================================================================
	Hyperion Engine
	Tests/EncodingTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <iostream>

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Library/UTF8.hpp"
#include "Hyperion/Core/Library/Binary.h"



namespace Hyperion
{
namespace Tests
{

	void RunEncodingTests()
	{
		std::cout << "--------------------------------------------------------------------------------------------------\n---> Running encoding tests!\n\n";

		// Basically, were going to just test the private encoding functions
		// The idea is.. these functions are used internally by the string library to perform encoding switches
		// Our encoding system is fairly simple, the encoding functions convert strings to a list of uint32 code points
		// we can then feed this list into encoders to get a different string encoding
		// The idea here is, were going to come up with various strings encoded differently, and check if the output code points match

		/*
		// Invalid Characters (Continuation Bytes Only)
		std::vector< byte > utf8Test = 
		{
			0x80, 0x81, 0x82, 0x83, 
			0x84, 0x85, 0x86, 0x87,
			0x89, 0x8A, 0x8B, 0x8C, 
			0x8D, 0x8E, 0x8F, 0x90,
			0x91, 0x92, 0x93, 0x94,
			0x95, 0x96, 0x97, 0x98,
			0x99, 0x9A, 0x9B, 0x9C,
			0x9D, 0x9E, 0x9F, 0xA0,
			0xA1, 0xA2, 0xA3, 0xA4,
			0xA5, 0xA6, 0xA7, 0xA8,
			0xA9, 0xAA, 0xAB, 0xAC,
			0xAD, 0xAE, 0xAF, 0xB0,
			0xB1, 0xB2, 0xB3, 0xB4, 
			0xB5, 0xB6, 0xB7, 0xB8, 
			0xB9, 0xBA, 0xBB, 0xBC, 
			0xBD, 0xBE, 0xBF
		};
		*/

		// Some strange other language characters
		std::vector< byte > utf8Test =
		{
			0x2A, 0xC4, 0x80, 0x7B,
			0x2D, 0xC4, 0x8F, 0x73,
			0x66, 0x6E, 0xC4, 0x96,
			0xC8, 0x87, 0x64, 0xE1,
			0xBA, 0x85, 0x66, 0x64
		};

		std::cout << "----> Input Data:\n";
		Binary::PrintBin( utf8Test.begin(), utf8Test.end() );
		std::cout << "\n";

		std::vector< uint32 > codePoints;
		bool bResult = Encoding::UTF8::BinaryToCodes( utf8Test, codePoints );
		std::cout << "\n";

		if( !bResult )
		{
			std::cout << "---> UTF-8 decoding failed!\n";
		}
		else
		{
			std::cout << "---> UTF-8 decoding passed!\n";

			int counter = 0;
			for( auto It = codePoints.begin(); It != codePoints.end(); It++ )
			{
				std::cout << "\t" << *It;

				if( counter > 3 )
				{
					std::cout << "\n";
					counter = 0;
				}
				else
				{
					counter++;
				}
			}

			std::cout << "\n\n";

			// Now we want to convert this back to a byte vector, and check if its the same
			std::cout << "----> Encoding code-points back into UTF-8...\n";
			std::vector< byte > ReEncodedData;

			Encoding::UTF8::CodesToBinary( codePoints, ReEncodedData );

			std::cout << "\n----> Result: \n";
			Binary::PrintBin( ReEncodedData.begin(), ReEncodedData.end() );

			std::cout << "\n\n----> Checking if the input and output binary are equal...\n";
			std::cout << "\t";

			if( std::equal( utf8Test.begin(), utf8Test.end(), ReEncodedData.begin(), ReEncodedData.end() ) )
			{
				std::cout << "TRUE\n";
			}
			else
			{
				std::cout << "FALSE\n";
			}
		}


		std::cout << "\n---> Encoding test complete!\n";
		std::cout << "--------------------------------------------------------------------------------------------------\n\n";

	}

}
}