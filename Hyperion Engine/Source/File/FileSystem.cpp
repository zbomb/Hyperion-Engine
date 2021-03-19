/*==================================================================================================
	Hyperion Engine
	Source/File/FileSystem.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/File/FileSystem.h"
#include "Hyperion/Core/AssetManager.h"
#include "Hyperion/Tools/HHTReader.h"
#include "Hyperion/Core/Platform.h"


namespace Hyperion
{

	/*===============================================================================================================
		class PhysicalFile
	===============================================================================================================*/
	File::File( const FilePath& inPath, FileMode inMode, WriteMode inWriteMode )
		: IFile( inPath ), m_Mode( inMode ), m_WriteMode( inWriteMode ), m_Buffer( std::make_unique< std::basic_filebuf< byte > >() )
	{
		// Build our flags for the stl open operation
		int openFlags = 0;
		openFlags |= std::ios::binary;

		if( inMode == FileMode::Read || inMode == FileMode::ReadWrite )
		{
			openFlags |= std::ios::in;
		}

		if( inMode == FileMode::Write || inMode == FileMode::ReadWrite )
		{
			openFlags |= std::ios::out;

			if( inWriteMode == WriteMode::Overwrite )		openFlags |= std::ios::trunc;
			else if( inWriteMode == WriteMode::Append )		openFlags |= std::ios::app;
		}

		// Attempt to open the filebuf.. the function that calls this should have already checked if the file already exists
		// so were just going to open/create the file, if inWriteMode == WriteMode::Overwrite, were going to delete existing contents
		std::basic_filebuf< byte >* ret = m_Buffer->open( m_Path.ToCPath( true ), openFlags );
		if( !ret || !ret->is_open() )
		{
			// Failed to open the file.. so were going to clear the buffer pointer
			m_Buffer.reset();
		}
	}


	std::basic_streambuf< byte >* File::GetStreamBuffer()
	{
		return m_Buffer ? m_Buffer.get() : nullptr;
	}


	bool File::CanReadStream() const
	{
		return m_Mode != FileMode::Write;
	}


	bool File::CanWriteStream() const
	{
		return m_Mode != FileMode::Read;
	}


	bool File::IsValid() const
	{
		return m_Buffer ? true : false;
	}


	bool File::IsClamped() const
	{
		return false;
	}


	size_t File::GetSize() const
	{
		if( !m_Buffer )
			return 0;

		std::basic_istream< byte > f( m_Buffer.get() );
		f.seekg( 0, std::ios::end );
		return f.tellg();
	}


	std::unique_ptr< FileMetaData > File::GetMetaData() const
	{
		return FileSystem::GetMetaData( m_Path, false );
	}


	/*===============================================================================================================
		class PhysicalDirectory
	===============================================================================================================*/

	Directory::Directory( const FilePath& inPath )
		: IDirectory( inPath, 
					  std::filesystem::is_directory( inPath.ToCPath( true ) ) 
		)
	{
	}


	void Directory::CacheContents()
	{
		// Clear out the currently cached files and directories
		{
			std::vector< FilePath >().swap( m_Files );
			std::vector< FilePath >().swap( m_Directories );
		}

		// Get list of all contents
		std::error_code err;
		std::filesystem::directory_iterator iter( m_Path.ToCPath( true ), err );
		for( auto& entry : iter )
		{
			// Ensure this entry is valid
			std::error_code err_exists;
			if( entry.exists( err_exists ) )
			{
				std::error_code type_err;
				if( entry.is_regular_file( type_err ) )
				{
					// Get filename and extension into a hyperion string
					auto filePath = entry.path();
					if( filePath.has_filename() )
					{
						m_Files.push_back(
							FilePath(
								std::filesystem::path( filePath.generic_wstring(), std::filesystem::path::format::generic_format ),
								PathRoot::SystemRoot
							)
						);
					}
				}
				else if( entry.is_directory( type_err ) )
				{
					// Get directory name
					auto dirPath = entry.path();
					if( dirPath.has_stem() )
					{
						m_Directories.push_back(
							FilePath(
								std::filesystem::path( dirPath.generic_wstring(), std::filesystem::path::format::generic_format ),
								PathRoot::SystemRoot
							)
						);
					}
				}
			}
		}
	}


	std::unique_ptr< FileMetaData > Directory::GetMetaData() const
	{
		return FileSystem::GetMetaData( m_Path, true );
	}


	/*===============================================================================================================
		class PhysicalMetaData
	===============================================================================================================*/
	
	FileMetaData::FileMetaData( const FilePath& inPath )
		: m_Path( inPath )
	{}


	/*===============================================================================================================
		class FileSystem
	===============================================================================================================*/

	bool FileSystem::Initialize( bool bDiscoverAssets, uint32 inFlags )
	{
		// Check if we need to discover assets through this file system
		if( bDiscoverAssets )
		{

			FilePath contentPath( "content/", PathRoot::Game );
			std::vector< FilePath > foundFiles;

			if( !DirectoryExists( contentPath ) )
			{
				Console::WriteLine( "[Warning] FileSystem: Failed to discover assets.. no content folder found??" );
				return true;
			}

			FindFiles( contentPath, foundFiles, true );
			auto rootPath = Platform::GetExecutablePath();

			for( const auto& f : foundFiles )
			{
				auto strPath = f.ToString();
				auto npath = strPath.SubStr( rootPath.Length() + 1, strPath.Length() - rootPath.Length() );

				if( npath.StartsWith( "content/" ) )
				{
					npath = npath.SubStr( 8, npath.Length() - 8 );
				}

				auto utfStr = npath.GetU8Str();
				std::vector< byte > utfData( utfStr.begin(), utfStr.end() );

				uint32 hashCode = Crypto::ELFHash( utfData );

				AssetInstanceInfo newAsset;
				newAsset.AlwaysCached	= false;
				newAsset.AssetType		= AssetManager::FindTypeFromExtension( f.Extension() ); // TODO: This is extreemly inefficient.
				newAsset.Identifier		= hashCode;
				newAsset.Offset			= 0;
				newAsset.Length			= 0;
				newAsset.Path			= npath;

				AssetManager::RegisterAsset( newAsset );
			}

			Console::WriteLine( "[FileSystem] Discovered ", foundFiles.size(), " content files on disk" );
		}

		return true;
	}


	void FileSystem::Shutdown()
	{

	}


	bool FileSystem::CreateNeededDirectories( const FilePath& inPath )
	{
		// Get underlying path
		auto p = inPath.ToCPath( true );
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


	std::unique_ptr< FileMetaData > FileSystem::GetMetaData( const FilePath& inPath, bool bIsDirectory )
	{
		if( bIsDirectory )
		{
			if( DirectoryExists( inPath ) )
			{
				return std::make_unique< FileMetaData >( inPath );
			}
		}
		else
		{
			if( FileExists( inPath ) )
			{
				return std::make_unique< FileMetaData >( inPath );
			}
		}

		return nullptr;
	}
	

	std::unique_ptr< File > FileSystem::OpenFile( const FilePath& inPath, FileMode inMode, WriteMode inWriteMode /* = WriteMode::Append */, bool bCreateIfNotExists /* = false */ )
	{
		// Override the 'system' part of the path, ensure its physical
		auto newPath( inPath );

		// First, check for empty path.. or empty filename and extension
		if( newPath.IsEmpty() || ( !newPath.HasExtension() && !newPath.HasFilename() ) )
			return nullptr;

		// Now, lets check if this file doesnt exist, and if not, then we will create it
		std::error_code err;
		auto cPath = newPath.ToCPath( true );

		if( !std::filesystem::exists( cPath, err ) )
		{
			if( bCreateIfNotExists )
			{
				return CreateFile( newPath, inMode, inWriteMode, true );
			}
			else
			{
				return nullptr;
			}
		}

		// File exists, so we need to open a filebuf and construct our output object
		if( std::filesystem::is_regular_file( cPath, err ) )
		{
			return std::unique_ptr< File >( new File( newPath, inMode, inWriteMode ) );
		}
		else
		{
			return nullptr;
		}
	}


	std::unique_ptr< File > FileSystem::CreateFile( const FilePath& inPath, FileMode inMode, WriteMode inWriteMode /* = WriteMode::Append */, bool bFailIfExists /* = true */ )
	{
		// Copy path, ensure the system is set to physical
		auto newPath( inPath );

		if( newPath.IsEmpty() || !newPath.HasExtension() || !newPath.HasFilename() )
		{
			return nullptr;
		}

		// If the file already exists, then either fail or open the file depending on the args
		std::error_code err;
		if( std::filesystem::exists( newPath.ToCPath( true ), err ) )
		{
			if( bFailIfExists )
			{
				return nullptr;
			}
			else
			{
				return OpenFile( newPath, inMode, inWriteMode, false );
			}
		}

		// File doesnt exist, so lets open a filebuf and create this new file
		// Also, create any needed directories for this file path
		if( !CreateNeededDirectories( newPath ) )
		{
			return nullptr;
		}

		return std::unique_ptr< File >( new File( newPath, inMode, inWriteMode ) );
	}


	bool FileSystem::FileExists( const FilePath& inPath )
	{
		std::error_code err;
		auto cPath = inPath.ToCPath( true );
		return std::filesystem::exists( cPath, err ) && std::filesystem::is_regular_file( cPath, err );
	}


	DeleteResult FileSystem::DeleteFile( const FilePath& inPath )
	{
		// Check if this file exists and is a normal file
		if( FileExists( inPath ) )
		{
			std::error_code err;
			return std::filesystem::remove( inPath.ToCPath( true ), err ) ? DeleteResult::Success : DeleteResult::Error;
		}
		else
		{
			return DeleteResult::DoesNotExist;
		}
	}


	std::unique_ptr< Directory > FileSystem::OpenDirectory( const FilePath& inPath, bool bCreateIfNotExists /* = false */ )
	{
		// Copy path, ensure 'system' is set to disk
		auto newPath( inPath );

		if( newPath.IsEmpty() || !newPath.HasStem() || newPath.HasExtension() )
			return nullptr;

		std::error_code err;
		auto cPath = newPath.ToCPath( true );
		if( !std::filesystem::exists( cPath, err ) )
		{
			// The directory doesnt exist, so create is bCreateIfNotExists is true
			if( bCreateIfNotExists )
			{
				return CreateDirectory( newPath, true );
			}
			else
			{
				return nullptr;
			}
		}

		// Directory doesnt exist, so lets open it, as long as its a directory
		if( std::filesystem::is_directory( cPath, err ) )
		{
			return std::unique_ptr< Directory >( new Directory( newPath ) );
		}
		else
		{
			return nullptr;
		}
	}


	std::unique_ptr< Directory > FileSystem::CreateDirectory( const FilePath& inPath, bool bFailIfExists /* = true */ )
	{
		auto newPath( inPath );

		if( newPath.IsEmpty() || !newPath.HasStem() || newPath.HasExtension() )
			return nullptr;

		std::error_code err;
		auto cPath = newPath.ToCPath( true );
		if( std::filesystem::exists( cPath, err ) )
		{
			// If this already exists, check bFailIfNotExists
			if( bFailIfExists )
			{
				return nullptr;
			}
			else
			{
				return OpenDirectory( newPath, false );
			}
		}

		// Directory doesnt exist, so lets create it
		std::error_code create_err;
		if( std::filesystem::create_directories( cPath, create_err ) )
		{
			return std::unique_ptr< Directory >( new Directory( newPath ) );
		}
		else
		{
			return nullptr;
		}
	}


	bool FileSystem::DirectoryExists( const FilePath& inPath )
	{
		std::error_code err;
		auto cPath = inPath.ToCPath( true );
		return std::filesystem::exists( cPath, err ) && std::filesystem::is_directory( cPath, err );
	}


	DeleteResult FileSystem::DeleteDirectory( const FilePath& inPath, bool bFailIfHasContents /* = true */ )
	{
		// Check if the directory exists
		if( !DirectoryExists( inPath ) )
		{
			return DeleteResult::DoesNotExist;
		}

		// Check if the directory has contents
		std::error_code err;
		auto cPath = inPath.ToCPath( true );
		std::filesystem::directory_iterator iter( cPath, err );

		auto entryCount = std::distance( std::filesystem::begin( iter ), std::filesystem::end( iter ) );

		if( entryCount > 0 )
		{
			if( bFailIfHasContents )
			{
				return DeleteResult::Error;
			}
			else
			{
				std::error_code delete_err;
				return std::filesystem::remove_all( cPath, delete_err ) > 0 ? DeleteResult::Success : DeleteResult::Error;
			}
		}
		else
		{
			std::error_code delete_err;
			return std::filesystem::remove( cPath, delete_err ) ? DeleteResult::Success : DeleteResult::Error;
		}
	}


	void FileSystem::FindFiles( const FilePath& inPath, std::vector< FilePath >& Out, std::function< bool( const FilePath& ) > inPredicate, bool bRecursive /* = false */ )
	{
		// Basically, we want to get a list of all files and directories in this path
		auto stdPath = inPath.ToCPath( true );
		if( !std::filesystem::is_directory( stdPath ) ) { return; }

		for( auto& entry : std::filesystem::directory_iterator( stdPath ) )
		{
			if( entry.is_regular_file() )
			{
				FilePath fPath( entry.path(), PathRoot::SystemRoot );
				if( inPredicate( fPath ) )
				{
					Out.push_back( fPath );
				}
			}
			else if( entry.is_directory() && bRecursive )
			{
				FindFiles( FilePath( entry.path(), PathRoot::SystemRoot ), Out, inPredicate, bRecursive );
			}
		}
	}

	
	void FileSystem::FindDirectories( const FilePath& inPath, std::vector< FilePath >& Out, std::function< bool( const FilePath& ) > inPredicate, bool bRecursive /* = false */ )
	{
		// Basically, we want to get a list of all files and directories in this path
		auto stdPath = inPath.ToCPath( true );
		if( !std::filesystem::is_directory( stdPath ) ) { return; }

		for( auto& entry : std::filesystem::directory_iterator( stdPath ) )
		{
			if( entry.is_directory() )
			{
				FilePath fPath( entry.path(), PathRoot::SystemRoot );
				if( inPredicate( fPath ) )
				{
					Out.push_back( fPath );
				}

				if( bRecursive )
				{
					FindFiles( FilePath( entry.path(), PathRoot::SystemRoot ), Out, inPredicate, bRecursive );
				}
			}
		}
	}


	void FileSystem::FindFiles( const FilePath& inPath, std::vector< FilePath >& Out, bool bRecursive /* = false */ )
	{
		auto stdPath = inPath.ToCPath( true );
		if( !std::filesystem::is_directory( stdPath ) ) { return; }
		
		for( auto& entry : std::filesystem::directory_iterator( stdPath ) )
		{
			if( entry.is_regular_file() )
			{
				FilePath fPath( String( entry.path().generic_u8string(), StringEncoding::UTF8 ), PathRoot::SystemRoot );
				Out.push_back( fPath );
			}
			else if( entry.is_directory() && bRecursive )
			{
				FindFiles( FilePath( entry.path(), PathRoot::SystemRoot ), Out, bRecursive );
			}
		}
	}


	void FileSystem::FindDirectories( const FilePath& inPath, std::vector< FilePath >& Out, bool bRecursive /* = false */ )
	{
		auto stdPath = inPath.ToCPath( true );
		if( !std::filesystem::is_directory( stdPath ) ) { return; }

		for( auto& entry : std::filesystem::directory_iterator( stdPath ) )
		{
			if( entry.is_directory() )
			{
				FilePath fPath( String( entry.path().generic_u8string(), StringEncoding::UTF8 ), PathRoot::SystemRoot );
				Out.push_back( fPath );

				if( bRecursive )
				{
					FindFiles( FilePath( entry.path(), PathRoot::SystemRoot ), Out, bRecursive );

				}
			}
		}
	}
}