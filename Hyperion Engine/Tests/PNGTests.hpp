/*==================================================================================================
	Hyperion Engine
	Tests/PNGTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Tools/PNGReader.h"
#include "Hyperion/Core/AssetManager.h"
#include "Hyperion/Assets/TextureAsset.h"


namespace Hyperion
{
namespace Tests
{

	void RunPNGTest()
	{
		Console::WriteLine( "\n================================================================================================" );
		Console::WriteLine( "--> Running PNG reader test...\n" );

		/*
		// First lets open a PNG file
		Console::WriteLine( "----> Opening test png file..." );

		FilePath path( "Test/test_png.png", PathRoot::Game );
		auto file = IFileSystem::OpenFile( path, FileMode::Read );

		if( !file || !file->IsValid() )
		{
			Console::WriteLine( "------> Failed to open test_png.png.. ensure its placed in [exe dir]/Test/test_png.png" );
		}
		else
		{
			Console::WriteLine( "------> Opened test png file!" );

			// Read file into a vector
			DataReader reader( file );
			reader.SeekBegin();

			std::vector< byte > pngData;
			reader.ReadBytes( pngData, reader.Size() );

			Console::WriteLine( "------> File size: ", pngData.size(), " bytes" );

			// Now lets read it with the png reader
			Console::WriteLine( "\n--> Processing PNG image.." );
			std::shared_ptr< RawImageData > imgData = nullptr;

			if( Tools::PNGReader::LoadFromMemory( pngData.begin(), pngData.end(), imgData ) != Tools::PNGReader::Result::Success || !imgData )
			{
				Console::WriteLine( "------> Failed to read PNG!" );
			}
			else
			{
				Console::WriteLine( "------> PNG Read Successfully!" );
			}
			
		}
		*/


		Console::WriteLine( "\n--> PNG Test Complete!" );
		Console::WriteLine( "==================================================================================================\n" );
	}

}
}