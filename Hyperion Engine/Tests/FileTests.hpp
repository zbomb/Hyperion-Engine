/*==================================================================================================
	Hyperion Engine
	Tests/FileTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/File.h"
#include "Hyperion/Core/Platform.h"

#include <iostream>


namespace Hyperion
{
namespace Tests
{
	
	void RunFileTests()
	{
		std::cout << "\n---------------------------------------------------------------------------------------------\n[TEST] Running file test...\n";

		{
			std::cout << "---> Testing 'Documents' path...\n";
			String fileData( "This is a test of the file system!\n" );
			FilePath path( "test.txt", PathRoot::Documents );

			std::cout << "----> Target Path: " << path << "\n";

			auto f = IFileSystem::CreateFile( path, FileMode::Write );
			if( f && f->IsValid() )
			{
				DataWriter w( f );

				// Copy string data into byte vector, use UTF-16 as encoding
				std::vector< byte > strData;
				fileData.CopyData( strData, StringEncoding::UTF16 );

				// Write data to the file
				if( !w.WriteBytes( strData ) )
				{
					std::cout << "------> Failed to write to file!\n";
				}
				else
				{
					std::cout << "------> Wrote to file!\n";
				}
			}
			else
			{
				std::cout << "------> Couldnt create file!\n";
			}
		}

		std::cout << "\n\n";

		{
			std::cout << "---> Testing file deletion...\n";
			
			FilePath path( "test.txt", PathRoot::Documents );
			if( IFileSystem::DeleteFile( path ) == DeleteResult::Success )
			{
				std::cout << "-----> Success!\n";
			}
			else
			{
				std::cout << "-----> Failed!\n";
			}
		}

		std::cout << "\n\n";

		{
			std::cout << "---> Testing folder creation...\n";

			FilePath path( "asdf", PathRoot::Documents );
			auto dir = IFileSystem::CreateDirectory( path );

			if( dir && dir->IsValid() )
			{
				std::cout << "-----> Success!\n";
			}
			else
			{
				std::cout << "-----> Failed!\n";
			}
		}

		std::cout << "\n\n";

		{
			std::cout << "---> Testing folder deletion...\n";

			FilePath path( "asdf", PathRoot::Documents );
			if( IFileSystem::DeleteDirectory( path ) == DeleteResult::Success )
			{
				std::cout << "------> Success!\n";
			}
			else
			{
				std::cout << "------> Failed!\n";
			}
		}

		std::cout << "\n\n";

		{
			std::cout << "---> Testing folder opening...\n";

			FilePath path( PathRoot::Game );
			auto dir = IFileSystem::OpenDirectory( path );
			if( dir && dir->IsValid() )
			{
				std::cout << "------> Opened game directory!\n";
				
				auto& files = dir->GetSubFiles();
				std::cout << "------> There are " << files.size() << " files!\n\n";

				for( auto& f : files )
				{
					std::cout << "\t" << f << "\n";
				}

				std::cout << "\n";
			}
			else
			{
				std::cout << "------> Failed to open game directory!\n";
			}
		}

		std::cout << "\n\n";

		{
			std::cout << "---> Testing file meta data...\n";

			std::cout << "------> Writing test file...\n";
			FilePath path( "test.dat", PathRoot::Documents );

			if( !IFileSystem::FileExists( path ) )
			{
				String fileData( "Data:\n" );
				for( int i = 0; i < 100; i++ )
					fileData = fileData.Append( "This is a test line of data! Really were just trying to make a fairly long file...\n" );

				auto f = IFileSystem::CreateFile( path, FileMode::Write );
				if( f && f->IsValid() )
				{
					DataWriter w( f );
					std::vector< byte > strData;
					fileData.CopyData( strData, StringEncoding::UTF16 );
					w.WriteBytes( strData );
				}
			}
			std::cout << "------> Getting file metadata...\n";
			auto m = IFileSystem::GetFileMetaData( path );
			if( m && m->IsValid() )
			{
				std::cout << "------> Success!\n\n";

				// Now lets read some info
				std::cout << "------> Is File?: " << ( m->IsFile() ? "TRUE" : "FALSE" ) << "\n";
				std::cout << "------> Is Dir?: " << ( m->IsDirectory() ? "TRUE" : "FALSE" ) << "\n";
				std::cout << "------> Size: " << m->GetSize() << " bytes\n";

				char buf[ 20 ];
				auto lastWrite = m->GetLastWrite();
				struct tm t;
				auto err = localtime_s( &t, &lastWrite );

				strftime( buf, 20, "%Y-%m-%d %H:%M:%S", &t );

				std::string strDate( buf );
				std::cout << "------> Last Write: " << strDate << "\n";

				std::cout << "------> Permissions: " << m->GetPermissionsAsString() << "\n";
			}
		}

		std::cout << "\n\n";

		{
			std::cout << "---> Testing directory metadata...\n";
			FilePath path( "test_dir", PathRoot::Documents );

			auto d = IFileSystem::CreateDirectory( path );
			if( d && d->IsValid() )
			{
				std::cout << "------> Created directory!\n";

				auto m = IFileSystem::GetDirectoryMetaData( path );
				if( m && m->IsValid() )
				{
					std::cout << "------> Success!\n\n";

					// Now lets read some info
					std::cout << "------> Is File?: " << ( m->IsFile() ? "TRUE" : "FALSE" ) << "\n";
					std::cout << "------> Is Dir?: " << ( m->IsDirectory() ? "TRUE" : "FALSE" ) << "\n";
					std::cout << "------> Size: " << m->GetSize() << " bytes\n";

					char buf[ 20 ];
					auto lastWrite = m->GetLastWrite();
					struct tm t;
					auto err = localtime_s( &t, &lastWrite );

					strftime( buf, 20, "%Y-%m-%d %H:%M:%S", &t );

					std::string strDate( buf );
					std::cout << "------> Last Write: " << strDate << "\n";

					std::cout << "------> Permissions: " << m->GetPermissionsAsString() << "\n";
				}
			}
		}

		std::cout << "\n\n";

		{
			std::cout << "---> Testing 'DirectoryExists'...\n";
			FilePath path( "test_dir", PathRoot::Documents );

			if( IFileSystem::DirectoryExists( path ) )
			{
				std::cout << "------> Success!\n";
			}
			else
			{
				std::cout << "------> Failed!\n";
			}
		}

		{
			std::cout << "---> Testing 'FileExists'...\n";
			FilePath path( "test.dat", PathRoot::Documents );

			if( IFileSystem::FileExists( path ) )
			{
				std::cout << "------> Success!\n";
			}
			else
			{
				std::cout << "------> Failed!\n";
			}
		}

		std::cout << "\n\n";
		{
			std::cout << "---> Checking if we can create extensionless files...\n";
			FilePath path( "shit_file", PathRoot::Documents );

			auto f = IFileSystem::CreateFile( path, FileMode::Write );
			if( f && f->IsValid() )
			{
				std::cout << "------> ALLOWED TO CREATE FILE!\n";
			}
			else
			{
				std::cout << "------> Success! File creation blocked!\n";
			}
		}

		{
			std::cout << "---> Checking if we can create a directory using a filename...\n";
			FilePath path( "shit_dir.fuck", PathRoot::Documents );

			auto dir = IFileSystem::CreateDirectory( path );
			if( dir && dir->IsValid() )
			{
				std::cout << "------> Failed! Was allowed to create directory!\n";
			}
			else
			{
				std::cout << "------> Success! Was not allowed to create directory!\n";
			}
		}

		std::cout << std::endl;
		std::cout << "----> File Test Complete!\n";
		std::cout << "---------------------------------------------------------------------------------------------\n";
	}

}
}