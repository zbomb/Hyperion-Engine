// Asset Manifest Tool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <cwctype>
#include <sstream>
#include <map>
#include <fstream>

#include "UTF8.hpp"


uint32 ELFHash( std::vector< byte >::const_iterator inBegin, std::vector< byte >::const_iterator inEnd )
{
    uint32 output = 0;
    uint32 x = 0;

    for( auto It = inBegin; It != inEnd; It++ )
    {
        output = ( output << 4 ) + ( *It );
        if( ( x = output & 0xF0000000L ) != 0 )
        {
            output ^= ( x >> 24 );
        }

        output &= ~x;
    }

    return output;
}


int main()
{
    
    std::cout << "=====> Hyperion Asset Manifest Tool <=====\n";
    std::cout << "\n--> Finding content directory...\n";

    // First, get the content directory
    auto curDir = std::filesystem::current_path() / "content";
    std::cout << "----> Target directory: " << curDir << "\n";

    // Ensure directory exists
    if( !std::filesystem::is_directory( curDir ) )
    {
        std::cout << "------> Error! Directory doesnt exist! Ensure this is in the game directory!\n";
    }
    else
    {
        std::map< uint32, std::vector< byte > > hashTable;

        // Loop through all files in the content directory
        for( auto& f : std::filesystem::recursive_directory_iterator( curDir ) )
        {
            if( f.is_regular_file() )
            {
                auto fpath = f.path();
                std::wstring extStr( fpath.extension().c_str() );
                std::transform(
                    extStr.begin(),
                    extStr.end(),
                    extStr.begin(),
                    std::towlower
                );

                if( extStr != L".hht" )
                {
                    // We need to convert this to a path thats local to the content directory
                    bool bDo = false;
                    std::stringstream ss;

                    for( auto It = fpath.begin(); It != fpath.end(); It++ )
                    {
                        if( bDo )
                        {
                            ss << It->generic_string();
                            if( !It->has_extension() )
                            {
                                ss << '/';
                            }
                        }
                        else
                        {
                            std::wstring str( It->c_str() );
                            std::transform( str.begin(), str.end(), str.begin(), std::towlower );

                            if( str == L"content" )
                            {
                                bDo = true;
                            }
                        }
                    }

                    auto relPath = ss.str();
                    std::cout << "------> File Found: " << relPath << "\n";

                    // Now that we have the relative path, we need to convert it to a UTF-8 string
                    // We currently have the path as an ascii path, we will use our UTF-8 library from the engine for this
                    std::vector< uint32 > codePoints;
                    for( auto It = relPath.begin(); It != relPath.end(); It++ )
                    {
                        codePoints.push_back( (uint32) *It );
                    }

                    std::vector< byte > pathData;
                    if( !Hyperion::Encoding::UTF8::CodesToBinary( codePoints, pathData ) )
                    {
                        std::cout << "-------> Failed to convert path to UTF-8\n";
                    }
                    else
                    {
                        // Next we need to hash the string data
                        auto hashCode = ELFHash( pathData.begin(), pathData.end() );

                        std::cout << "----------> Hash: " << hashCode << "\n";

                        // Now, we need to add it to the list
                        // Check for collisions
                        auto entry = hashTable.find( hashCode );
                        if( entry != hashTable.end() )
                        {
                            std::wcout << "---------> Collision Detected! Hash code: " << hashCode << " for path \"" << relPath.c_str() << "\"\n";
                        }
                        else
                        {
                            hashTable.emplace( hashCode, pathData );
                        }
                    }
                }
            }
        }

        std::cout << "\n";
        std::cout << "---> " << hashTable.size() << " entries in the hash table\n";
        std::cout << "---> Writing to file...\n";

        std::filesystem::path manifestPath = curDir / "manifest.hht";
        bool bError = false;

        if( std::filesystem::is_regular_file( manifestPath ) )
        {
            std::cout << "------> Existing manifest file found.. going to overwrite..\n";
            if( !std::filesystem::remove( manifestPath ) )
            {
                std::cout << "------> Failed to delete old manifest file! Aborting..\n";
                bError = true;
            }
        }

        if( !bError )
        {
            std::ofstream file( manifestPath, std::ios::out );
            
            // Now we need to write out the file
            // The first 8-bytes are a header sequence, and the following four are unused
            std::vector< byte > fileData =
            {
                0x1A, 0xA1, 0xFF, 0x28,
                0x9D, 0xD9, 0x00, 0x01,
                0x00, 0x00, 0x00, 0x00
            };

            // Determine system endianess
            static const uint32 n = 1;
            static const bool bLittleEndian = ( *(char*) &n == 1 );

            std::cout << "---> Local system is " << ( bLittleEndian ? "Little Endian" : "Big Endian" ) << "\n";

            // Next loop through all entries
            for( auto& e : hashTable )
            {
                // Ensure data length isnt over the limit
                if( std::numeric_limits< uint16_t >::max() < e.second.size() )
                {
                    std::cout << "-------> Failed to write entry.. data too long!\n";
                }
                else
                {
                    std::cout << "---------------> Hash: " << e.first << "\n";
                    std::cout << "---------------> Len: " << e.second.size() << "\n";
                    
                    // First, write the 4-byte hash code big-endian
                    if( bLittleEndian )
                    {
                        std::reverse_copy(
                            reinterpret_cast<const byte*>( &e.first ),
                            reinterpret_cast<const byte*>( &e.first ) + 4,
                            std::back_inserter( fileData )
                        );
                    }
                    else
                    {
                        std::copy(
                            reinterpret_cast<const byte*>( &e.first ),
                            reinterpret_cast<const byte*>( &e.first ) + 4,
                            std::back_inserter( fileData )
                        );
                    }

                    // Next, write the data length out
                    uint16_t len = (uint16_t) e.second.size();
                    if( bLittleEndian )
                    {
                        std::reverse_copy(
                            reinterpret_cast<const byte*>( &len ),
                            reinterpret_cast<const byte*>( &len ) + 2,
                            std::back_inserter( fileData )
                        );
                    }
                    else
                    {
                        std::copy(
                            reinterpret_cast<const byte*>( &len ),
                            reinterpret_cast<const byte*>( &len ) + 2,
                            std::back_inserter( fileData )
                        );
                    }

                    // Finally, write the data
                    std::copy(
                        e.second.begin(),
                        e.second.end(),
                        std::back_inserter( fileData )
                    );
                }
            }

            // Now that we have the file data built, lets write it out
            file.write( (char*)fileData.data(), fileData.size() );
            std::cout << "---> Wrote manifest to file!\n";
        }
    }

    std::cout << "--> Complete! Press the 'enter' key to exit...\n";
    std::cin.get();

    return 0;

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
