/*==================================================================================================
	Hyperion Engine
	Tests/PNGTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Tools/HTXWriter.h"
#include "Hyperion/Tools/HTXReader.h"


namespace Hyperion
{
	namespace Tests
	{

		void RunTextureTest()
		{
			Console::WriteLine( "\n================================================================================================" );
			Console::WriteLine( "--> Running HTX test...\n" );

			// Lets write a bullshit texture file
			FilePath path( String( "textures/test.htx" ), LocalPath::Content );
			std::unique_ptr< PhysicalFile > f;

			PhysicalFileSystem::DeleteFile( path ); // DEBUG

			if( !PhysicalFileSystem::FileExists( path ) )
			{
				f = PhysicalFileSystem::CreateFile( path, FileMode::Write, WriteMode::Append );
				if( !f || !f->IsValid() )
				{
					Console::WriteLine( "====> Failed to write test .htx! (couldnt create file)" );
				}

				HTXWriter::Input InputData;
				InputData.Format = TextureFormat::RGBA_8BIT_UNORM;
				InputData.LevelPadding = 0;
				
				HTXWriter::LODInfo LOD;
				LOD.Data.insert( LOD.Data.end(), 32 * 32 * 4, 0 );
				LOD.Width = 32;
				LOD.Height = 32;
				LOD.RowSize = 32;
				
				InputData.LODs.push_back( LOD );

				/*
				HTXWriter::LODInfo OtherLOD;
				OtherLOD.Data.insert( OtherLOD.Data.end(), 16 * 16 * 4, 0 );
				OtherLOD.Width = 16;
				OtherLOD.Height = 16;
				OtherLOD.RowSize = 16;

				InputData.LODs.push_back( OtherLOD );
				*/

				auto result = HTXWriter::Write( f, InputData );
				if( result != HTXWriter::Result::Success )
				{
					Console::WriteLine( "====> Failed to write test .htx (failed to write data ",(uint32) result, ")" );
				}
			}

			Console::WriteLine( "\n===============================================================================================" );
		}
	}
}