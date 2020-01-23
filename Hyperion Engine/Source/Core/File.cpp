/*==================================================================================================
	Hyperion Engine
	Source/Core/File.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/File.h"


#if HYPERION_OS_WIN32
#include "Hyperion/Win32/Win32FileSystem.h"
#elif HYPERION_OS_MAC
#elif HYPERION_OS_LINUX
#elif HYPERION_OS_ANDROID
#endif



namespace Hyperion
{
	FilePath::FilePath()
	{}

	FilePath::FilePath( const String& In, PathRoot Root /* = PathRoot::Game */ )
		: m_Path( IFileSystem::BuildSystemPath( In, Root ) )
	{
	}

	FilePath::FilePath( PathRoot Root )
		: m_Path( IFileSystem::BuildSystemPath( "", Root ) )
	{}

	FilePath::FilePath( const FilePath& In )
		: m_Path( In.m_Path )
	{
	}

	FilePath::FilePath( const std::filesystem::path& In )
		: m_Path( In )
	{}

	FilePath& FilePath::operator=( const FilePath& Other )
	{
		m_Path = Other.m_Path;
		return *this;
	}

	String FilePath::ToString_Native() const
	{
		return String( m_Path.u8string(), StringEncoding::UTF8 );
	}

	String FilePath::ToString() const
	{
		return String( m_Path.generic_u8string(), StringEncoding::UTF8 );
	}

	void FilePath::Clear()
	{
		m_Path.clear();
	}

	bool FilePath::IsEmpty() const
	{
		return m_Path.empty();
	}

	FilePath& FilePath::operator+=( const String& In )
	{
		m_Path += In.GetU16Str();
		return *this;
	}

	FilePath& FilePath::operator/=( const String& In )
	{
		m_Path /= In.GetU16Str();
		return *this;
	}

	int FilePath::Compare( const FilePath& Other ) const
	{
		return m_Path.compare( Other.m_Path );
	}

	FilePath FilePath::RootName() const
	{
		return FilePath( m_Path.root_name() );
	}

	FilePath FilePath::RootDirectory() const
	{
		return FilePath( m_Path.root_directory() );
	}

	FilePath FilePath::RootPath() const
	{
		return FilePath( m_Path.root_path() );
	}

	FilePath FilePath::RelativePath() const
	{
		return FilePath( m_Path.relative_path() );
	}

	FilePath FilePath::ParentPath() const
	{
		return FilePath( m_Path.parent_path() );
	}

	FilePath FilePath::Filename() const
	{
		return FilePath( m_Path.filename() );
	}

	FilePath FilePath::Stem() const
	{
		return FilePath( m_Path.stem() );
	}

	FilePath FilePath::Extension() const
	{
		return FilePath( m_Path.extension() );
	}

	bool FilePath::HasRootPath() const
	{
		return m_Path.has_root_path();
	}
	
	bool FilePath::HasRootName() const
	{
		return m_Path.has_root_name();
	}

	bool FilePath::HasRootDirectory() const
	{
		return m_Path.has_root_directory();
	}

	bool FilePath::HasRelativePath() const
	{
		return m_Path.has_relative_path();
	}

	bool FilePath::HasParentPath() const
	{
		return m_Path.has_parent_path();
	}

	bool FilePath::HasFilename() const
	{
		return m_Path.has_filename();
	}

	bool FilePath::HasExtension() const
	{
		return m_Path.has_extension();
	}

	bool FilePath::HasStem() const
	{
		return m_Path.has_stem();
	}

	bool FilePath::IsAbsolute() const
	{
		return m_Path.is_absolute();
	}

	bool FilePath::IsRelative() const
	{
		return m_Path.is_relative();
	}

	bool FilePath::operator==( const FilePath& Other ) const
	{
		return m_Path == Other.m_Path;
	}

	bool FilePath::operator!=( const FilePath& Other ) const
	{
		return m_Path != Other.m_Path;
	}

	bool FilePath::operator<( const FilePath& Other ) const
	{
		return m_Path < Other.m_Path;
	}

	bool FilePath::operator<=( const FilePath& Other ) const
	{
		return m_Path <= Other.m_Path;
	}

	bool FilePath::operator>( const FilePath& Other ) const
	{
		return m_Path > Other.m_Path;
	}

	bool FilePath::operator>=( const FilePath& Other ) const
	{
		return m_Path >= Other.m_Path;
	}

	FilePath FilePath::operator/( const FilePath& r )
	{
		return FilePath( m_Path / r.m_Path );
	}

	const std::filesystem::path& FilePath::GetSTLPath() const
	{
		return m_Path;
	}

	std::ostream& operator<<( std::ostream& inStream, const FilePath& inPath )
	{
		return inStream << inPath.GetSTLPath();
	}

	/*===================================================================================
		File
	===================================================================================*/
	File::File( const FilePath& In, FileMode inMode, WriteMode inWriteMode )
		: m_Path( In ), m_Mode( inMode ), m_WriteMode( inWriteMode ), m_Buffer( std::make_unique< std::basic_filebuf< byte > >() )
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
		auto* ret = m_Buffer->open( m_Path.GetSTLPath(), openFlags );
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
		return m_Mode == FileMode::Read || m_Mode == FileMode::ReadWrite;
	}

	bool File::CanWriteStream() const
	{
		return m_Mode == FileMode::Write || m_Mode == FileMode::ReadWrite;
	}

	bool File::IsValid() const
	{
		return m_Buffer ? true : false;
	}

	std::streampos File::Size() const
	{
		if( !m_Buffer )
			return 0;

		std::basic_istream< byte > f( m_Buffer.get() );
		f.seekg( 0, std::ios::end );
		return f.tellg();
	}

	std::unique_ptr< MetaData > File::GetMetaData()
	{
		return IFileSystem::GetFileMetaData( m_Path );
	}

	/*===================================================================================
		MetaData
	===================================================================================*/
	MetaData::MetaData( const FilePath& inPath )
		: m_Path( inPath )
	{
		// Get some meta data for the file/directory
		auto rawPath = m_Path.GetSTLPath();

		if( std::filesystem::is_directory( rawPath ) )
		{
			bIsValid = true;
			bIsDirectory = true;
		}
		else if( std::filesystem::is_regular_file( rawPath ) )
		{
			bIsValid = true;
			bIsDirectory = false;
		}
		else
		{
			bIsValid = false;
		}

		if( bIsValid )
		{
			std::error_code err;
			m_LastWrite = IFileSystem::GetLastWrite( inPath );

			auto status = std::filesystem::status( rawPath, err );
			m_Permissions = status.permissions();
		
			m_Size = bIsDirectory ? 0 : std::filesystem::file_size( rawPath );
		}
		else
		{
			m_LastWrite		= std::time_t();
			m_Size			= 0;
			m_Permissions	= Permissions();
		}
	}

	/*===================================================================================
		Directory
	===================================================================================*/
	Directory::Directory( const FilePath& inPath )
		: m_Path( inPath ), m_Valid( std::filesystem::is_directory( inPath.GetSTLPath() ) ), m_Cached( false )
	{
	}

	void Directory::CacheContents()
	{
		// Clear any cached files or folders
		{
			std::vector< FilePath >().swap( m_Files );
			std::vector< FilePath >().swap( m_Folders );
		}

		// Get list of all contents
		std::error_code err;
		std::filesystem::directory_iterator iter( m_Path.GetSTLPath(), err );
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
						m_Files.push_back( FilePath( std::filesystem::path( filePath.generic_wstring(), std::filesystem::path::format::generic_format ) ) );
					}
				}
				else if( entry.is_directory( type_err ) )
				{
					// Get directory name
					auto dirPath = entry.path();
					if( dirPath.has_stem() )
					{
						m_Folders.push_back( FilePath( std::filesystem::path( dirPath.generic_wstring(), std::filesystem::path::format::generic_format ) ) );
					}
				}
			}
		}

		m_Cached = true;
	}

	const std::vector< FilePath >& Directory::GetSubFiles()
	{
		if( !IsCached() )
		{
			CacheContents();
		}

		return m_Files;
	}

	const std::vector< FilePath >& Directory::GetSubDirectories()
	{
		if( !IsCached() )
		{
			CacheContents();
		}

		return m_Folders;
	}

	std::unique_ptr< MetaData > Directory::GetMetaData() const
	{
		return IFileSystem::GetDirectoryMetaData( m_Path );
	}

}