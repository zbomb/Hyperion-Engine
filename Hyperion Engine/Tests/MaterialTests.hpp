/*==================================================================================================
	Hyperion Engine
	Tests/MaterialTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/File/PhysicalFileSystem.h"
#include "Hyperion/Tools/HMATWriter.h"
#include "Hyperion/Tools/HMATReader.h"
#include "Hyperion/Assets/MaterialAsset.h"
#include "Hyperion/Core/AssetManager.h"


namespace Hyperion
{
	namespace Tests
	{
		void RunMaterialTests()
		{
			Console::WriteLine( "\n================================================================================================" );
			Console::WriteLine( "--> Running HMAT test...\n" );

			auto asdf = AssetManager::Get< MaterialAsset >( 263475652 );
			if( asdf )
			{
				Console::WriteLine( "---------> Got material asset from disk!" );
				Console::WriteLine( "\n----> Getting properties..." );

				for( auto It = asdf->ValuesBegin(); It != asdf->ValuesEnd(); It++ )
				{
					if( It->second.has_value() )
					{
						Console::Write( "-------> Key: ", It->first, "  " );
						auto& t = It->second.type();
						if( t == typeid( bool ) )
						{
							Console::WriteLine( "Boolean: ", std::any_cast<bool>( It->second ) );
						}
						else if( t == typeid( int32 ) )
						{
							Console::WriteLine( "Int: ", std::any_cast<int32>( It->second ) );
						}
						else if( t == typeid( uint32 ) )
						{
							Console::WriteLine( "UInt: ", std::any_cast<uint32>( It->second ) );
						}
						else if( t == typeid( float ) )
						{
							Console::WriteLine( "Float: ", std::any_cast<float>( It->second ) );
						}
						else if( t == typeid( String ) )
						{
							Console::WriteLine( "String: ", std::any_cast<String>( It->second ) );
						}
						else
						{
							Console::WriteLine( "Unknown Value Type" );
						}
					}
					else
					{
						Console::WriteLine( "-------> Key: ", It->first, "  Null Value" );
					}
				}

				for( auto It = asdf->TexturesBegin(); It != asdf->TexturesEnd(); It++ )
				{
					if( It->second )
					{
						Console::WriteLine( "-------> Key: ", It->first, "  Value: Texture [", It->second->GetPath().ToString(), "]" );
					}
					else
					{
						Console::WriteLine( "-------> Key: ", It->first, "   Value: Null Texture" );
					}
				}
			}

			Console::WriteLine( "\n" );

			// First, were going to write a test HMAT file with a couple entries of basic types
			Console::WriteLine( "---> Creating material file (content/materials/test.hmat)" );

			FilePath path( "materials/test2.hmat", LocalPath::Content );
			PhysicalFileSystem::DeleteFile( path );
			auto f = PhysicalFileSystem::CreateFile( path, FileMode::Write );
			if( !f || !f->IsValid() )
			{
				Console::WriteLine( "----> Failed to create \"content/materials/test2.hmat\"" );
			}
			else
			{
				Console::WriteLine( "----> Created file!" );

				Console::WriteLine( "\n---> Writing .hmat file..." );

				HMATWriter::Result res = HMATWriter::Result::Failed;
				{
					HMATWriter writer( *f );
					writer.AddEntry( "test1", true );
					writer.AddEntry( "test2", (int32) 55 );
					writer.AddEntry( "test3", 4.4f );
					writer.AddEntry( "test4", (uint32) 69 );
					writer.AddEntry( "test5", String( "this is a test" ) );
					writer.AddEntry( "test6", AssetManager::Get< TextureAsset >( 5367464 ) );

					res = writer.Flush();
				}

				if( res != HMATWriter::Result::Success )
				{
					Console::WriteLine( "-----> Failed to write to file!" );
				}
				else
				{
					Console::WriteLine( "-----> Wrote file successfully!" );

					// Now, were going to try and read the file
					f.reset();
					
					Console::WriteLine( "\n---> Reopening file in read only mode..." );
					auto f = PhysicalFileSystem::OpenFile( path, FileMode::Read );
					if( !f || !f->IsValid() )
					{
						Console::WriteLine( "-----> Failed to reopen file!" );
					}
					else
					{
						Console::WriteLine( "-----> Reopened file!" );

						// Now, we want to try and read the entries back out
						HMATReader reader( *f );

						Console::WriteLine( "\n---> Reading header...." );
						uint16 count = 0;
						auto res = reader.GetEntryCount( count );

						if( res != HMATReader::Result::Success )
						{
							Console::WriteLine( "-----> Failed to read entry count!" );
						}
						else
						{
							Console::WriteLine( "-----> Entry Count: ", count );
							Console::WriteLine( "\n---> Reading value list..." );

							// Next, we want to read all entries in
							reader.Begin();
							while( reader.Next() )
							{
								String key;
								std::any value;
								auto res = reader.ReadEntry( key, value );
								if( res != HMATReader::Result::Success || !value.has_value() )
								{
									Console::WriteLine( "-----> Failed to read entry! Error Code: ", (uint32) res );
									break;
								}
								else
								{
									Console::WriteLine( "\n-----> Key: ", key );
									
									auto& t = value.type();
									if( t == typeid( bool ) )
									{
										Console::WriteLine( "-------> Type: Boolean" );
										Console::WriteLine( "-------> Value: ", std::any_cast<bool>( value ) ? "TRUE" : "FALSE" );
									}
									else if( t == typeid( int32 ) )
									{
										Console::WriteLine( "-------> Type: Int32" );
										Console::WriteLine( "-------> Value: ", std::any_cast<int32>( value ) );
									}
									else if( t == typeid( uint32 ) )
									{
										Console::WriteLine( "-------> Type: UInt32" );
										Console::WriteLine( "-------> Value: ", std::any_cast<uint32>( value ) );
									}
									else if( t == typeid( float ) )
									{
										Console::WriteLine( "-------> Type: Float" );
										Console::WriteLine( "------> Value: ", std::any_cast<float>( value ) );
									}
									else if( t == typeid( String ) )
									{
										Console::WriteLine( "-------> Type: String" );
										Console::WriteLine( "-------> Value: ", std::any_cast<String>( value ) );
									}
									else if( t == typeid( TextureReference ) )
									{
										Console::WriteLine( "------> Type: Texture" );
										auto tex = AssetManager::Get< TextureAsset >( std::any_cast<TextureReference>( value ).Identifier );
										Console::WriteLine( "------> Value: ", tex ? tex->GetPath().ToString() : "NULL" );
									}
									else
									{
										Console::WriteLine( "-------> Unknown Type!" );
									}
								}
							}

							Console::WriteLine( "---> Finished reading .hmat file!" );
						}
					}
				}
			}

			Console::WriteLine( "--> HMAT test complete!" );
			Console::WriteLine( "================================================================================================\n" );

		}
	}
}