/*==================================================================================================
	Hyperion Engine
	Source/File/PhysicalFileSystem.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/File/PhysicalFileSystem.h"
#include "Hyperion/Core/AssetManager.h"
#include "Hyperion/Tools/HHTReader.h"


namespace Hyperion
{

	/*===============================================================================================================
		class PhysicalFile
	===============================================================================================================*/
	PhysicalFile::PhysicalFile( const FilePath& inPath, FileMode inMode, WriteMode inWriteMode )
		: IFile( inPath.IsDisk() ? inPath : FilePath() ), m_Mode( inMode ), m_WriteMode( inWriteMode ), m_Buffer( std::make_unique< std::basic_filebuf< byte > >() )
	{
		if( !inPath.IsDisk() )
		{
			Console::WriteLine( "[ERROR] PhysicalFileSystem: Attempt to open a file with a non-disk path!" );
		}
		else
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
	}


	std::basic_streambuf< byte >* PhysicalFile::GetStreamBuffer()
	{
		return m_Buffer ? m_Buffer.get() : nullptr;
	}


	bool PhysicalFile::CanReadStream() const
	{
		return m_Mode != FileMode::Write;
	}


	bool PhysicalFile::CanWriteStream() const
	{
		return m_Mode != FileMode::Read;
	}


	bool PhysicalFile::IsValid() const
	{
		return m_Buffer ? true : false;
	}


	bool PhysicalFile::IsClamped() const
	{
		return false;
	}


	size_t PhysicalFile::GetSize() const
	{
		if( !m_Buffer )
			return 0;

		std::basic_istream< byte > f( m_Buffer.get() );
		f.seekg( 0, std::ios::end );
		return f.tellg();
	}


	std::unique_ptr< PhysicalMetaData > PhysicalFile::GetMetaData() const
	{
		return PhysicalFileSystem::GetMetaData( m_Path, false );
	}


	/*===============================================================================================================
		class PhysicalDirectory
	===============================================================================================================*/

	PhysicalDirectory::PhysicalDirectory( const FilePath& inPath )
		: IDirectory( inPath, 
					  inPath.IsDisk() &&
					  std::filesystem::is_directory( inPath.ToCPath( true ) ) 
		)
	{
		if( !inPath.IsDisk() )
		{
			Console::WriteLine( "[ERROR] PhysicalFileSystem: Attempt to open a directory with a non-disk path!" );
		}
	}


	void PhysicalDirectory::CacheContents()
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
								LocalPath::Root,
								FileSystem::Disk
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
								LocalPath::Root,
								FileSystem::Disk
							)
						);
					}
				}
			}
		}
	}


	std::unique_ptr< PhysicalMetaData > PhysicalDirectory::GetMetaData() const
	{
		return PhysicalFileSystem::GetMetaData( m_Path, true );
	}


	/*===============================================================================================================
		class PhysicalMetaData
	===============================================================================================================*/
	
	PhysicalMetaData::PhysicalMetaData( const FilePath& inPath )
		: m_Path( inPath )
	{}


	/*===============================================================================================================
		class PhysicalFileSystem
	===============================================================================================================*/

	bool PhysicalFileSystem::Initialize( bool bDiscoverAssets )
	{
		// Check if we need to discover assets through this file system
		if( bDiscoverAssets )
		{
			auto f = OpenFile( FilePath( String( "manifest.hht" ), LocalPath::Content, FileSystem::Disk ), FileMode::Read );
			if( !f || !f->IsValid() )
			{
				Console::WriteLine( "[ERROR] FileSystem: The currently selected content system is disk, but there was no content manifest.. no assets will be able to load!" );
				// Should we return false here? What if there is not supposed to be any content for some reason?
			}
			else
			{
				// Once we load the file in, we need to seek through it, using some type of reader, and insert all entries into the asset manager
				HHTReader reader( *f );
				if( !reader.Validate() )
				{
					Console::WriteLine( "[ERROR] FileSystem: The currently selected content system is disk, but the content manifest was invalid!" );
					// Should we return false here?
				}
				else
				{
					reader.Begin();

					// Scan through file, and read all table entries
					uint32 assetCounter = 0;

					while( reader.NextEntry() )
					{
						uint32 hashCode;
						std::vector< byte > strData;

						if( reader.ReadEntry( hashCode, strData ) == HHTReader::Result::Success )
						{
							// Paths are stored as a UTF-8 string, and the hash code is a uint32
							AssetManager::RegisterAsset( hashCode, String( strData, StringEncoding::UTF8 ) );
							assetCounter++;
						}
					}

					Console::WriteLine( "[Status] FileSystem: Discovered ", assetCounter, " assets on disk" );
				}
			}
		}

		return true;
	}


	void PhysicalFileSystem::Shutdown()
	{

	}


	bool PhysicalFileSystem::CreateNeededDirectories( const FilePath& inPath )
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


	std::unique_ptr< PhysicalMetaData > PhysicalFileSystem::GetMetaData( const FilePath& inPath, bool bIsDirectory )
	{
		if( bIsDirectory )
		{
			if( DirectoryExists( inPath ) )
			{
				return std::make_unique< PhysicalMetaData >( inPath );
			}
		}
		else
		{
			if( FileExists( inPath ) )
			{
				return std::make_unique< PhysicalMetaData >( inPath );
			}
		}

		return nullptr;
	}
	

	std::unique_ptr< PhysicalFile > PhysicalFileSystem::OpenFile( const FilePath& inPath, FileMode inMode, WriteMode inWriteMode /* = WriteMode::Append */, bool bCreateIfNotExists /* = false */ )
	{
		// Override the 'system' part of the path, ensure its physical
		auto newPath( inPath );
		newPath.SetSystem( FileSystem::Disk );

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
			return std::unique_ptr< PhysicalFile >( new PhysicalFile( newPath, inMode, inWriteMode ) );
		}
		else
		{
			return nullptr;
		}
	}


	std::unique_ptr< PhysicalFile > PhysicalFileSystem::CreateFile( const FilePath& inPath, FileMode inMode, WriteMode inWriteMode /* = WriteMode::Append */, bool bFailIfExists /* = true */ )
	{
		// Copy path, ensure the system is set to physical
		auto newPath( inPath );
		newPath.SetSystem( FileSystem::Disk );

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

		return std::unique_ptr< PhysicalFile >( new PhysicalFile( newPath, inMode, inWriteMode ) );
	}


	bool PhysicalFileSystem::FileExists( const FilePath& inPath )
	{
		std::error_code err;
		auto cPath = inPath.ToCPath( true );
		return std::filesystem::exists( cPath, err ) && std::filesystem::is_regular_file( cPath, err );
	}


	DeleteResult PhysicalFileSystem::DeleteFile( const FilePath& inPath )
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


	std::unique_ptr< PhysicalDirectory > PhysicalFileSystem::OpenDirectory( const FilePath& inPath, bool bCreateIfNotExists /* = false */ )
	{
		// Copy path, ensure 'system' is set to disk
		auto newPath( inPath );
		newPath.SetSystem( FileSystem::Disk );

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
			return std::unique_ptr< PhysicalDirectory >( new PhysicalDirectory( newPath ) );
		}
		else
		{
			return nullptr;
		}
	}


	std::unique_ptr< PhysicalDirectory > PhysicalFileSystem::CreateDirectory( const FilePath& inPath, bool bFailIfExists /* = true */ )
	{
		auto newPath( inPath );
		newPath.SetSystem( FileSystem::Disk );

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
			return std::unique_ptr< PhysicalDirectory >( new PhysicalDirectory( newPath ) );
		}
		else
		{
			return nullptr;
		}
	}


	bool PhysicalFileSystem::DirectoryExists( const FilePath& inPath )
	{
		std::error_code err;
		auto cPath = inPath.ToCPath( true );
		return std::filesystem::exists( cPath, err ) && std::filesystem::is_directory( cPath, err );
	}


	DeleteResult PhysicalFileSystem::DeleteDirectory( const FilePath& inPath, bool bFailIfHasContents /* = true */ )
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


	void PhysicalFileSystem::FindFiles( const FilePath& inPath, std::vector< FilePath >& Out, std::function< bool( const FilePath& ) > inPredicate, bool bIncludeSubFolders /* = false */ )
	{
		// Basically, we want to get a list of all files and directories in this path
		auto stdPath = inPath.ToCPath( true );
		if( !std::filesystem::is_directory( stdPath ) ) { return; }

		for( auto& entry : std::filesystem::directory_iterator( stdPath ) )
		{
			if( entry.is_regular_file() )
			{
				FilePath fPath( entry.path(), LocalPath::Root, FileSystem::Disk );
				if( inPredicate( fPath ) )
				{
					Out.push_back( fPath );
				}
			}
			else if( entry.is_directory() && bIncludeSubFolders )
			{
				FindFiles( FilePath( entry.path(), LocalPath::Root, FileSystem::Disk ), Out, inPredicate, bIncludeSubFolders );
			}
		}
	}


	void PhysicalFileSystem::FindFiles( const FilePath& inPath, std::vector< FilePath >& Out, bool bIncludeSubFolders /* = false */ )
	{
		auto stdPath = inPath.ToCPath( true );
		if( !std::filesystem::is_directory( stdPath ) ) { return; }

		for( auto& entry : std::filesystem::directory_iterator( stdPath ) )
		{
			if( entry.is_regular_file() )
			{
				FilePath fPath( String( entry.path().generic_u8string(), StringEncoding::UTF8 ), LocalPath::Root, FileSystem::Disk );
				Out.push_back( fPath );
			}
			else if( entry.is_directory() && bIncludeSubFolders )
			{
				FilePath fPath( String( entry.path().generic_u8string(), StringEncoding::UTF8 ), LocalPath::Root, FileSystem::Disk );
				Out.push_back( fPath );
			}
		}
	}



}