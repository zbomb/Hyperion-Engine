/*==================================================================================================
	Hyperion Engine
	Tests/TextureTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Tools/HTXWriter.h"
#include "Hyperion/Tools/HTXReader.h"
#include "Hyperion/Tools/HHTReader.h"
#include "Hyperion/Tools/HHTWriter.h"
#include "Hyperion/Core/AssetManager.h"
#include "Hyperion/Library/Crypto.h"
#include "Hyperion/Library/UTF8.hpp"
#include "Hyperion/Library/UTF16.hpp"


namespace Hyperion
{
	namespace Tests
	{

		void RunTextureTest()
		{
			Console::WriteLine( "\n================================================================================================" );
			Console::WriteLine( "--> Running HTX test...\n" );

			// Below is code to write a single LOD, 32 x 32 black texture
			/*
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
				

				auto result = HTXWriter::Write( f, InputData );
				if( result != HTXWriter::Result::Success )
				{
					Console::WriteLine( "====> Failed to write test .htx (failed to write data ",(uint32) result, ")" );
				}
			}
			*/

			// .htx Reader Test
			/*
			auto f = PhysicalFileSystem::OpenFile( FilePath( "textures/test.htx", LocalPath::Content ), FileMode::Read );
			if( !f || !f->IsValid() )
			{
				Console::WriteLine( "====> Test failed! textures/test.htx doesnt exist!" );
			}
			else
			{
				Console::WriteLine( "==> Reading texture file..." );
				HTXReader Reader( *f );
				
				TextureHeader header;
				if( Reader.ReadHeader( header ) != HTXReader::Result::Success )
				{
					Console::WriteLine( "====> Failed to read header!" );
				}
				else
				{
					Console::WriteLine( "====> Header was read in!" );
					Console::WriteLine( "======> Format: ", (uint32)header.Format );
					Console::WriteLine( "======> LOD Count: ", header.LODs.size() );

					auto& lod = header.LODs.at( 0 );
					Console::WriteLine( "======> Width: ", lod.Width );
					Console::WriteLine( "======> Height: ", lod.Height );
					Console::WriteLine( "======> Offset: ", lod.FileOffset );
					Console::WriteLine( "======> Size: ", lod.LODSize );

				}
			}*/

			/* Write out a test .hht file

			Console::WriteLine( "\n===> Writing hash table test..." );

			FilePath manifestPath( "manifest.hht", LocalPath::Content );
			auto f = PhysicalFileSystem::CreateFile( manifestPath, FileMode::Write, WriteMode::Overwrite );
			if( f && f->IsValid() )
			{
				HHTWriter Writer( *f, true );

				// The hash code for an asset is the path (relative to content folder) (lowercase) in UTF-8, hashed with ELF hashing algorithm
				// Then, we will store the 
				std::vector< byte > pathData;
				{
					String pathStr( "textures/test.htx" );
					pathStr.CopyData( pathData, StringEncoding::UTF8 );
				}

				uint32 hashCode = Crypto::ELFHash( pathData );

				// Now, we can write our entry
				Writer.AddEntry( hashCode, pathData );
				auto result = Writer.Flush();

				if( result != HHTWriter::Result::Success )
				{
					Console::WriteLine( "=======> Hash table write test failed! Couldnt flush writer! Code: ", (uint32) result );
				}
				else
				{
					Console::WriteLine( "======> Successfully wrote hash table!" );
				}
			}
			else
			{
				Console::WriteLine( "=======> Hash table write test failed! Couldnt open the file!" );
			}
			*/
			

			
			Console::WriteLine( "===> Running asset test..." );
			auto identifier = AssetManager::GetAssetIdentifier( "textures/test.htx" );

			Console::WriteLine( "=====> Get Identifier: ", identifier );

			auto path = AssetManager::GetAssetPath( identifier );
			Console::WriteLine( "=====> Get Asset Path: ", path );

			auto ptr = AssetManager::Get< TextureAsset >( identifier );
			Console::WriteLine( "=====> Get By Identifier: ", ptr ? "VALID" : "INVLAID" );

			auto sptr = AssetManager::Get< TextureAsset >( "textures/test.htx" );
			Console::WriteLine( "=====> Get By Path: ", sptr ? "VALID" : "INVALID" );


			Console::WriteLine( "\n===============================================================================================" );
		}
	}
}