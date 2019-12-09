/*==================================================================================================
	Hyperion Engine
	Tests/StringTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"
#include "Hyperion/Core/Library/UTF16.hpp"

namespace Hyperion
{
namespace Tests
{
	void RunUTF16Tests()
	{
		std::cout << "\n---------------------------------------------------------------------------------------------\n[TEST] Running UTF-16 test...\n\n";
		{

			std::cout << "\n-------> Attempting to decode UTF-16 data into string.....\n";

			std::vector< byte > utfData =
			{
				0x00, 0x54, 0x00, 0x68, 
				0x00, 0x69, 0x00, 0x73, 
				0x00, 0x20, 0x00, 0x69,
				0x00, 0x73, 0x00, 0x20, 
				0x00, 0x61, 0x00, 0x20, 
				0x00, 0x74, 0x00, 0x65, 
				0x00, 0x73, 0x00, 0x74, 
				0x00, 0x20, 0xd8, 0x3d, 
				0xde, 0x03
			};

			std::cout << "\t----> Binary Data: \n";
			Binary::PrintBin( utfData.begin(), utfData.end() );

			std::cout << "\n----> Starting decode.....\n";

			std::vector< uint32 > CodePoints;
			if( !Encoding::UTF16::BinaryToCodes( utfData.begin(), utfData.end(), CodePoints ) )
			{
				std::cout << "\n\t----> Failed to decode!\n\n";
			}
			else
			{
				std::cout << "\n\t----> Decode passed!\n\n";
				std::cout << "\t----> Code Points:\n\n";

				int i = 0;
				for( auto It = CodePoints.begin(); It != CodePoints.end(); It++ )
				{
					if( i > 3 )
					{
						std::cout << std::endl;
						i = 0;
					}

					std::cout << "\t" << *It;
					i++;
				}

				std::cout << "\n\n";

				std::vector< byte > strData;
				Encoding::UTF8::CodesToBinary( CodePoints, strData );

				String str( strData, StringEncoding::UTF8 );
				std::cout << "\t----> Output: " << str << "\n\n";

				std::cout << "\n-------> Writing data back into UTF-16....\n";
				std::vector< byte > Rewrite;
				Encoding::UTF16::CodesToBinary( CodePoints, Rewrite );

				Binary::PrintBin( Rewrite.begin(), Rewrite.end() );
			}

		}
		std::cout << "\n----> UTF-16 test complete!\n---------------------------------------------------------------------------------------------\n\n";
	}

	void RunStringTests()
	{
		std::cout << "\n---------------------------------------------------------------------------------------------\n[TEST] Running string test...\n\n";
		{
			String testString( "this is a test!" );

			// Print out the string
			std::cout << "----> Printing a string...\n";
			std::cout << testString;
			std::cout << "\n";

			std::cout << "----> Changing string value...\n";
			testString = "this is another test!";
			std::cout << testString;
			std::cout << "\n";

			// Test String::IsLocalized
			std::cout << "----> Checking if string is localized...\n";
			if( testString.IsLocalized() )
			{
				std::cout << "----> TRUE\n";
			}
			else
			{
				std::cout << "----> FALSE\n";
			}

			// Create second string
			String otherString( "this is a second string!" );
			otherString = "penis toucher";

			std::cout << "----> String assignment test...\n";
			otherString = testString;

			std::cout << "----> Updated Value:\n\t";
			std::cout << otherString;
			std::cout << std::endl;

			std::cout << "----> Clearing Cache...\n";
		}

		{
			std::cout << "\n\n\n\n------------> Begining Localization Tests!\n\n";

			std::cout << "---> Creating languages and localization keys...\n";
			LanguageInfo newLang = { { 'T', 'e', 's', 't' }, "test" };
			auto langId = Localization::AddLanguage( newLang );
			if( langId == LANG_NONE )
			{
				std::cout << "---> Failed to create test language!\n";
			}

			// Now lets add our definitions
			String firstStr = "The first localization definition";
			auto firstStrData = firstStr.Data();

			String secondStr = "The second localization definition";
			auto secondStrData = secondStr.Data();

			bool bResult =
				Localization::CreateOrUpdateDefinition( langId, "test_key", firstStrData->begin(), firstStrData->end() ) &&
				Localization::CreateOrUpdateDefinition( langId, "other_key", secondStrData->begin(), secondStrData->end() );

			if( !bResult )
			{
				std::cout << "----> Failed to create new language definitions!\n";
			}

			// Now lets create two localized strings
			String localStr1	= String::GetLocalized( "test_key" );
			String localStr2	= String::GetLocalized( "other_key" );

			std::cout << "\n";
			std::cout << "---> Printing the two localized strings...\n";
			std::cout << "\t" << localStr1;
			std::cout << "\n\t" << localStr2;
			std::cout << "\n\n";

			// Now lets do some more complicated operations....
			// Were going to create a localized string with a key that doesnt exist, and see if the fallback value works as intended
			std::cout << "----> Performing fallback test on localized string...\n";
			String bullShit = String::GetLocalized( "fucked_key", "This is some bullshit!", StringEncoding::ASCII );

			std::cout << "---> Value: " << bullShit << "\n";

			std::cout << std::endl;
			Localization::DebugCache();

			std::cout << "\n\n----> Assignment Test...\n";
			std::cout << "----> Assigning localized string to non-localized string\n";
			firstStr = localStr1;
			std::cout << "====> IsLocalized: " << ( firstStr.IsLocalized() ? "YES" : "NO" ) << "\n";

			std::cout << "----> Value: " << firstStr << "\n";
			Localization::DebugCache();

			// What else can we test?
			// Iterators
			std::cout << "\n----> Iterator Testing...\n";
			String iterStr = "asdfghjkl";
			for( auto It = iterStr.begin(); It != iterStr.end(); It++ )
			{
				// Print out character
				std::cout << "\t" << *It << "\n";
			}

			std::cout << "\n";
			std::cout << "------> Testing null String::iterators\n";
			String nullStr;
			for( auto It = nullStr.begin(); It != nullStr.end(); It++ )
			{
				std::cout << "\t" << *It << "\n";
			}

			std::cout << "-----> Null string test complete... nothing should have printed!\n";

			std::cout << "\n----> Performing some tests on the string library\n";
			String asdf = "asdf";
			String otherAsdf = "asdf";

			if( asdf == otherAsdf )
				std::cout << "\t---> Equality test success!\n";
			else
				std::cout << "\t---> Equality test failed!\n";

			asdf = asdf + otherAsdf;
			std::cout << "\n------> Append test.. should print asdfasdf\n";
			std::cout << "\t---> " << asdf << "\n";

			std::cout << "\n Testing StartsWith...\n";
			if( asdf.StartsWith( String( "asd" ) ) )
			{
				std::cout << "\t---> Starts with working!\n";
			}
			else
			{
				std::cout << "\t---> Starts with is not working\n";
			}

			std::cout << "\n------> Testing EndsWith...\n";
			if( asdf.EndsWith( String( "df" ) ) )
			{
				std::cout << "\t---> EndsWith is working!\n";
			}
			else
			{
				std::cout << "\t---> EndsWith is not working!\n";
			}

			std::cout << "\n------> Performing prepend test.. test_asdfasdf should print\n";
			asdf = asdf.Prepend( "test_" );
			std::cout << "\t---> " << asdf << "\n";

			std::cout << "\n------> Performing character count test...\n";
			std::cout << "\t---> Result: " << asdf.Length() << "\n";

			std::cout << "\n------> Performing byte count test...\n";
			std::cout << "\t---> Result: " << asdf.ByteCount() << " bytes\n";

			std::cout << "\n------> Performing find test... finding first instance of 'asdf'\n";
			auto result = asdf.Find( "asdf" );
			auto resultDistance = result - asdf.begin();
			std::cout << "\t---> Found 'asdf' sequence in string at index " << resultDistance << "\n";

			std::cout << "\n------> Performing substring test.. getting substring at 3, length 5\n";
			std::cout << "\t---> Result: " << asdf.SubStr( 3, 5 ) << "\n";

		}

		std::cout << "\n----> String test complete!\n---------------------------------------------------------------------------------------------\n\n";
	}
}
}