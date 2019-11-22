/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/FileImpl.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"
#include "Hyperion/Core/Stream.h"

#include <fstream>
#include <filesystem>
#include <sstream>

// Ensure we dont have preprocessor defs that may alter function names
#undef CreateFile
#undef DeleteFile
#undef CreateDirectory

#define HYPERION_CHECK_PERM( _TARGET_, _PERM_ ) ( ( _TARGET_ & std::filesystem::perms::_PERM_ ) != std::filesystem::perms::none )

namespace Hyperion
{

	enum class PathRoot
	{
		Root,
		Game,
		Assets,
		Data,
		Documents
	};


	class FilePath
	{

	protected:

		std::filesystem::path m_Path;

		explicit FilePath( const std::filesystem::path& );

	public:

		FilePath();
		explicit FilePath( const String&, PathRoot r = PathRoot::Game );
		explicit FilePath( PathRoot r );
		FilePath( const FilePath& );
		FilePath& operator=( const FilePath& );

		String ToString_Native() const;
		String ToString() const;

		void Clear();
		bool IsEmpty() const;

		FilePath& operator+=( const String& );
		FilePath& operator/=( const String& );

		int Compare( const FilePath& ) const;

		FilePath RootName() const;
		FilePath RootDirectory() const;
		FilePath RootPath() const;
		FilePath RelativePath() const;
		FilePath ParentPath() const;
		FilePath Filename() const;
		FilePath Stem() const;
		FilePath Extension() const;

		bool HasRootPath() const;
		bool HasRootName() const;
		bool HasRootDirectory() const;
		bool HasRelativePath() const;
		bool HasParentPath() const;
		bool HasFilename() const;
		bool HasExtension() const;
		bool HasStem() const;

		bool IsAbsolute() const;
		bool IsRelative() const;

		bool operator==( const FilePath& ) const;
		bool operator!=( const FilePath& ) const;
		bool operator<( const FilePath& ) const;
		bool operator<=( const FilePath& ) const;
		bool operator>( const FilePath& ) const;
		bool operator>=( const FilePath& ) const;

		FilePath operator/( const FilePath& );

		const std::filesystem::path& GetSTLPath() const;

		friend std::ostream& operator<<( std::ostream&, const FilePath& );
		friend class Directory;
	};

	std::ostream& operator<<( std::ostream&, const FilePath& );

	using FileTimePoint		= std::filesystem::file_time_type;
	using FileTimeClock		= std::filesystem::_File_time_clock;
	using Permissions		= std::filesystem::perms;

	class MetaData
	{
	public:

		MetaData() = delete;
		explicit MetaData( const FilePath& );

	protected:

		const FilePath m_Path;

		std::time_t m_LastWrite;
		Permissions m_Permissions;
		std::streamsize m_Size;

		bool bIsValid;
		bool bIsDirectory;

	public:

		std::time_t GetLastWrite() const { return m_LastWrite; }
		Permissions GetPermissions() const { return m_Permissions; }
		std::streamsize GetSize() const { return m_Size; }
		bool IsValid() const { return bIsValid; }
		bool IsDirectory() const { return bIsDirectory; }
		bool IsFile() const { return !bIsDirectory; }

		String GetPermissionsAsString()
		{
			std::stringstream Output;
			auto p = GetPermissions();

			Output << "Owner: ";
			Output << ( HYPERION_CHECK_PERM( p, owner_read )	? "r" : "-" );
			Output << ( HYPERION_CHECK_PERM( p, owner_write )	? "w" : "-" );
			Output << ( HYPERION_CHECK_PERM( p, owner_exec )	? "x" : "-" );

			Output << " Group: ";
			Output << ( HYPERION_CHECK_PERM( p, group_read )	? "r" : "-" );
			Output << ( HYPERION_CHECK_PERM( p, group_write )	? "w" : "-" );
			Output << ( HYPERION_CHECK_PERM( p, group_exec )	? "x" : "-" );

			Output << " Others: ";
			Output << ( HYPERION_CHECK_PERM( p, others_read )	? "r" : "-" );
			Output << ( HYPERION_CHECK_PERM( p, others_write )	? "w" : "-" );
			Output << ( HYPERION_CHECK_PERM( p, others_exec )	? "x" : "-" );

			return String( Output.str() );
		}
	};

	enum class FileMode
	{
		Read,
		Write,
		ReadWrite
	};

	enum class WriteMode
	{
		Append,
		Overwrite
	};

	class File : public IDataSource
	{
	protected:

		std::unique_ptr< std::basic_filebuf< byte > > m_Buffer;
		const FileMode m_Mode;
		const FilePath m_Path;
		const WriteMode m_WriteMode;

	public:

		File() = delete;
		File( const File& ) = delete;
		File( const FilePath&, FileMode, WriteMode );

		File& operator=( const File& ) = delete;

		virtual std::basic_streambuf< byte >* GetStreamBuffer();
		virtual bool CanReadStream() const;
		virtual bool CanWriteStream() const;
		bool IsValid() const;
		std::streampos Size() const;
		std::unique_ptr< MetaData > GetMetaData();

		inline FileMode GetMode() const			{ return m_Mode; }
		inline FilePath GetPath() const			{ return m_Path; }
		inline WriteMode GetWriteMode() const	{ return m_WriteMode; }


	};

	class Directory
	{
	protected:

		const FilePath m_Path;
		const bool m_Valid;

		std::vector< FilePath > m_Files;
		std::vector< FilePath > m_Folders;

		bool m_Cached;

		void CacheContents();
		inline bool IsCached() const { return m_Cached; }

	public:

		Directory() = delete;
		explicit Directory( const FilePath& );

		inline bool IsValid() const			{ return m_Valid; }
		inline FilePath GetPath() const		{ return m_Path; }

		const std::vector< FilePath >& GetSubFiles();
		const std::vector< FilePath >& GetSubDirectories();

		std::unique_ptr< MetaData > GetMetaData() const;

	};

	enum class DeleteResult
	{
		Error,
		DoesntExist,
		Success
	};

	class IFileServices
	{
	public:

		// Ugly, used internally by FilePath constructor
		virtual std::filesystem::path BuildSystemPath( const String&, PathRoot ) = 0;
		virtual std::time_t GetLastWrite( const FilePath& ) = 0;

		virtual std::unique_ptr< File > OpenFile( const FilePath& inPath, FileMode inMode, WriteMode inWrite = WriteMode::Append, bool bCreateIfNotExists = false ) = 0;
		virtual std::unique_ptr< File > CreateFile( const FilePath& inPath, FileMode inMode, WriteMode inWrite = WriteMode::Append, bool bFailIfExists = true ) = 0;
		virtual bool FileExists( const FilePath& inPath ) = 0;
		virtual DeleteResult DeleteFile( const FilePath& inPath ) = 0;
		virtual std::unique_ptr< MetaData > GetFileMetaData( const FilePath& inPath ) = 0;

		virtual std::unique_ptr< Directory > OpenDirectory( const FilePath& inPath, bool bCreateIfNotExists = false ) = 0;
		virtual std::unique_ptr< Directory > CreateDirectory( const FilePath& inPath, bool bFailIfExists = true ) = 0;
		virtual bool DirectoryExists( const FilePath& inPath ) = 0;
		virtual DeleteResult DeleteDirectory( const FilePath& inPath, bool bFailIfHasContents = true ) = 0;
		virtual std::unique_ptr< MetaData > GetDirectoryMetaData( const FilePath& inPath ) = 0;

	};

	template< typename FType >
	class FileSystem_Impl
	{

	private:

		static std::unique_ptr< IFileServices > m_Impl;
		static IFileServices& Get()
		{
			if( !m_Impl ) 
			{
				m_Impl = std::make_unique< FType >();
			}

			return *m_Impl;
		}

	protected:

		static std::filesystem::path BuildSystemPath( const String& inPath, PathRoot inRoot ) { return Get().BuildSystemPath( inPath, inRoot ); }
		static std::time_t GetLastWrite( const FilePath& Target ) { return Get().GetLastWrite( Target ); }

	public:

		static std::unique_ptr< File > OpenFile( const FilePath& inPath, FileMode inMode, WriteMode inWrite = WriteMode::Append, bool bCreateIfNotExists = false ) { return Get().OpenFile( inPath, inMode, inWrite, bCreateIfNotExists ); }
		static std::unique_ptr< File > CreateFile( const FilePath& inPath, FileMode inMode, WriteMode inWrite = WriteMode::Append, bool bFailIfExists = true ) { return Get().CreateFile( inPath, inMode, inWrite, bFailIfExists ); }
		static bool FileExists( const FilePath& inPath ) { return Get().FileExists( inPath ); }
		static DeleteResult DeleteFile( const FilePath& inPath ) { return Get().DeleteFile( inPath ); }
		static std::unique_ptr< MetaData > GetFileMetaData( const FilePath& inPath ) { return Get().GetFileMetaData( inPath ); }
		static std::unique_ptr< MetaData > GetFileMetaData( const std::unique_ptr< File >& In )
		{
			if( In && In->IsValid() )
			{
				return In->GetMetaData();
			}

			return nullptr;
		}

		static std::unique_ptr< Directory > OpenDirectory( const FilePath& inPath, bool bCreateIfNotExists = false ) { return Get().OpenDirectory( inPath, bCreateIfNotExists ); }
		static std::unique_ptr< Directory > CreateDirectory( const FilePath& inPath, bool bFailIfExists = true ) { return Get().CreateDirectory( inPath, bFailIfExists ); }
		static bool DirectoryExists( const FilePath& inPath ) { return Get().DirectoryExists( inPath ); }
		static DeleteResult DeleteDirectory( const FilePath& inPath, bool bFailIfHasContents = true ) { return Get().DeleteDirectory( inPath, bFailIfHasContents ); }
		static std::unique_ptr< MetaData > GetDirectoryMetaData( const FilePath& inPath ) { return Get().GetDirectoryMetaData( inPath ); }
		static std::unique_ptr< MetaData > GetDirectoryMetaData( const std::unique_ptr< Directory >& In )
		{
			if( In && In->IsValid() )
			{
				return In->GetMetaData();
			}

			return nullptr;
		}

		friend class FilePath;
		friend class MetaData;
	};

	template< typename T >
	std::unique_ptr< IFileServices > FileSystem_Impl< T >::m_Impl; 
}