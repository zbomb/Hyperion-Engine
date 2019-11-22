/*==================================================================================================
	Hyperion Engine
	Source/Win32/Win32FileSystem.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Hyperion.h"

#if HYPERION_OS_WIN32

#include "Hyperion/Win32/Win32FileSystem.h"
#include "Hyperion/Core/Platform.h"
#include "Hyperion/Core/Engine.h"

#include <sys/stat.h>

namespace Hyperion
{


	std::unique_ptr< File > Win32FileSystem::OpenFile( const FilePath& inPath, FileMode inMode, WriteMode inWrite /* = WriteMode::Append */, bool bCreateIfNotExists /* = false */ )
	{
		// First, check for empty path.. or empty filename and extension
		if( inPath.IsEmpty() || ( !inPath.HasExtension() && !inPath.HasFilename() ) )
			return nullptr;

		// Now, lets check if this file doesnt exist, and if not, then we will create it
		std::error_code err;
		if( !std::filesystem::exists( inPath.GetSTLPath(), err ) )
		{
			if( bCreateIfNotExists )
			{
				return CreateFile( inPath, inMode, inWrite, true );
			}
			else
			{
				return nullptr;
			}
		}

		// File exists, so we need to open a filebuf and construct our output object
		if( std::filesystem::is_regular_file( inPath.GetSTLPath(), err ) )
		{
			return std::unique_ptr< File >( new File( inPath, inMode, inWrite ) );
		}
		else
		{
			return nullptr;
		}
	}

	bool Win32FileSystem::CreateNeededDirectories( const FilePath& In )
	{
		// Get underlying path
		auto& p = In.GetSTLPath();
		auto root = p.root_path();

		// Ensure root path is valid
		if( root.empty() || !std::filesystem::exists( root ) )
		{
			return false;
		}

		// Copy path and remove filename
		std::filesystem::path copy( p );

		if( copy.has_filename() )
		{
			copy.remove_filename();
		}

		// Remove trailing backslash
		std::filesystem::path clean_path;
		for( auto& p : copy )
		{
			if( !p.empty() )
				clean_path /= p;
		}

		clean_path.make_preferred();

		// Create any missing directories
		std::error_code err, err2;
		return std::filesystem::exists( clean_path, err ) || std::filesystem::create_directories( clean_path, err2 );
	}

	std::unique_ptr< File > Win32FileSystem::CreateFile( const FilePath& inPath, FileMode inMode, WriteMode inWrite /* = WriteMode::Append */, bool bFailIfExists /* = true */ )
	{
		if( inPath.IsEmpty() || !inPath.HasExtension() || !inPath.HasFilename() )
		{
			return nullptr;
		}

		// If the file already exists, then either fail or open the file depending on the args
		std::error_code err;
		if( std::filesystem::exists( inPath.GetSTLPath(), err ) )
		{
			if( bFailIfExists )
			{
				return nullptr;
			}
			else
			{
				return OpenFile( inPath, inMode, inWrite, false );
			}
		}
		
		// File doesnt exist, so lets open a filebuf and create this new file
		// Also, create any needed directories for this file path
		if( !CreateNeededDirectories( inPath ) )
		{
			return nullptr;
		}

		return std::unique_ptr< File >( new File( inPath, inMode, inWrite ) );
	}

	bool Win32FileSystem::FileExists( const FilePath& inPath )
	{
		std::error_code err;
		return std::filesystem::exists( inPath.GetSTLPath(), err ) && std::filesystem::is_regular_file( inPath.GetSTLPath(), err );
	}

	DeleteResult Win32FileSystem::DeleteFile( const FilePath& inPath )
	{
		// Check if this file exists and is a normal file
		if( FileExists( inPath ) )
		{
			std::error_code err;
			return std::filesystem::remove( inPath.GetSTLPath(), err ) ? DeleteResult::Success : DeleteResult::Error;
		}
		else
		{
			return DeleteResult::DoesntExist;
		}
	}

	std::unique_ptr< MetaData > Win32FileSystem::GetFileMetaData( const FilePath& inPath )
	{
		if( FileExists( inPath ) )
		{
			return std::unique_ptr< MetaData >( new MetaData( inPath ) );
		}
		else
		{
			return nullptr;
		}
	}


	std::unique_ptr< Directory > Win32FileSystem::OpenDirectory( const FilePath& inPath, bool bCreateIfNotExists /* = false */ )
	{
		if( inPath.IsEmpty() || !inPath.HasStem() || inPath.HasExtension() )
			return nullptr;

		std::error_code err;
		if( !std::filesystem::exists( inPath.GetSTLPath(), err ) )
		{
			// The directory doesnt exist, so create is bCreateIfNotExists is true
			if( bCreateIfNotExists )
			{
				return CreateDirectory( inPath, true );
			}
			else
			{
				return nullptr;
			}
		}

		// Directory doesnt exist, so lets open it, as long as its a directory
		if( std::filesystem::is_directory( inPath.GetSTLPath(), err ) )
		{
			return std::unique_ptr< Directory >( new Directory( inPath ) );
		}
		else
		{
			return nullptr;
		}
	}

	std::unique_ptr< Directory > Win32FileSystem::CreateDirectory( const FilePath& inPath, bool bFailIfExists /* = true */ )
	{
		if( inPath.IsEmpty() || !inPath.HasStem() || inPath.HasExtension() )
			return nullptr;

		std::error_code err;
		if( std::filesystem::exists( inPath.GetSTLPath(), err ) )
		{
			// If this already exists, check bFailIfNotExists
			if( bFailIfExists )
			{
				return nullptr;
			}
			else
			{
				return OpenDirectory( inPath, false );
			}
		}

		// Directory doesnt exist, so lets create it
		std::error_code create_err;
		if( std::filesystem::create_directories( inPath.GetSTLPath(), create_err ) )
		{
			return std::unique_ptr< Directory >( new Directory( inPath ) );
		}
		else
		{
			return nullptr;
		}
	}

	bool Win32FileSystem::DirectoryExists( const FilePath& inPath )
	{
		std::error_code err;
		return std::filesystem::exists( inPath.GetSTLPath(), err ) && std::filesystem::is_directory( inPath.GetSTLPath(), err );
	}

	DeleteResult Win32FileSystem::DeleteDirectory( const FilePath& inPath, bool bFailIfHasContents /* = true */ )
	{
		// Check if the directory exists
		if( !DirectoryExists( inPath ) )
		{
			return DeleteResult::DoesntExist;
		}

		// Check if the directory has contents
		std::error_code err;
		std::filesystem::directory_iterator iter( inPath.GetSTLPath(), err );
		
		auto entryCount = std::distance( std::filesystem::begin( iter), std::filesystem::end( iter ) );

		if( entryCount > 0 )
		{
			if( bFailIfHasContents )
			{
				return DeleteResult::Error;
			}
			else
			{
				std::error_code delete_err;
				return std::filesystem::remove_all( inPath.GetSTLPath(), delete_err ) > 0 ? DeleteResult::Success : DeleteResult::Error;
			}
		}
		else
		{
			std::error_code delete_err;
			return std::filesystem::remove( inPath.GetSTLPath(), delete_err ) ? DeleteResult::Success : DeleteResult::Error;
		}


	}

	std::unique_ptr< MetaData > Win32FileSystem::GetDirectoryMetaData( const FilePath& inPath )
	{
		if( DirectoryExists( inPath ) )
		{
			return std::unique_ptr< MetaData >( new MetaData( inPath ) );
		}
		else
		{
			return nullptr;
		}
	}


	std::filesystem::path Win32FileSystem::BuildSystemPath( const String& inStr, PathRoot Root )
	{
		if( Root == PathRoot::Root )
		{
			return std::filesystem::path( inStr.GetU16Str(), std::filesystem::path::format::generic_format );
		}
		else if( Root == PathRoot::Documents )
		{
			auto docPath = Platform::GetUserDataPath();
			auto& eng = Engine::GetInstance();

			docPath = docPath.Append( "/" ).Append( eng.GetDocumentsFolderName() );
			if( !inStr.IsEmpty() )
			{
				docPath = docPath.Append( "/" ).Append( inStr );
			}

			return std::filesystem::path( docPath.GetU16Str(), std::filesystem::path::generic_format );
		}
		else if( Root == PathRoot::Data )
		{
			auto gameRoot = Platform::GetExecutablePath().Append( "/data" );

			if( !inStr.IsEmpty() )
			{
				gameRoot = gameRoot.Append( "/" ).Append( inStr );
			}

			return std::filesystem::path( gameRoot.GetU16Str(), std::filesystem::path::format::generic_format );
		}
		else if( Root == PathRoot::Assets )
		{
			auto gameRoot = Platform::GetExecutablePath().Append( "/assets" );

			if( !inStr.IsEmpty() )
			{
				gameRoot = gameRoot.Append( "/" ).Append( inStr );
			}

			return std::filesystem::path( gameRoot.GetU16Str(), std::filesystem::path::format::generic_format );
		}
		else /* PathRoot::Game & default */
		{
			auto gameRoot = Platform::GetExecutablePath();

			if( !inStr.IsEmpty() )
			{
				gameRoot = gameRoot.Append( "/" ).Append( inStr );
			}

			return std::filesystem::path( gameRoot.GetU16Str(), std::filesystem::path::format::generic_format );
		}
	}

	std::time_t Win32FileSystem::GetLastWrite( const FilePath& Target )
	{
		// Try to get the last write to this file/folder
		struct _stat64 fileInfo;
		if( _wstati64( Target.GetSTLPath().wstring().c_str(), &fileInfo ) != 0 )
		{
			// Error!
			return std::time_t( 0 );
		}

		return fileInfo.st_mtime;
	}

}




#endif