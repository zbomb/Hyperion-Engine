/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/File.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"
#include "Hyperion/Core/Stream.h"

#include <fstream>
#include <filesystem>


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

		FilePath( const std::filesystem::path& );

	public:

		FilePath();
		FilePath( const String&, PathRoot r = PathRoot::Game );
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

	};

	using FileTimePoint		= std::filesystem::file_time_type;
	using FileTimeClock		= std::filesystem::_File_time_clock;
	using Permissions		= std::filesystem::perms;

	class MetaData
	{
	public:

		MetaData() = delete;
		MetaData( const FilePath& );

	protected:

		const FilePath m_Path;

		FileTimePoint m_LastWrite;
		Permissions m_Permissions;
		std::streamsize m_Size;

		bool bIsValid;
		bool bIsDirectory;

	public:

		FileTimePoint GetLastWrite() const { return m_LastWrite; }
		Permissions GetPremissions() const { return m_Permissions; }
		std::streamsize GetSize() const { return m_Size; }
		bool IsValid() const { return bIsValid; }
		bool IsDirectory() const { return bIsDirectory; }
		bool IsFile() const { return !bIsDirectory; }
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

		std::vector< String > m_Files;
		std::vector< String > m_Folders;

		bool m_Cached;

		void CacheContents();
		inline bool IsCached() const { return m_Cached; }

	public:

		Directory() = delete;
		Directory( const FilePath& );

		inline bool IsValid() const			{ return m_Valid; }
		inline FilePath GetPath() const		{ return m_Path; }

		std::vector< String > GetSubFiles();
		std::vector< String > GetSubDirectories();

		std::unique_ptr< MetaData > GetMetaData() const;

	};

	enum class DeleteResult
	{
		Error,
		DoesntExist,
		Success
	};

	class IFileSystem
	{

	private:

		static std::unique_ptr< IFileSystem > m_Singleton;

	protected:

		virtual std::filesystem::path BuildSystemPath( const String&, PathRoot ) = 0;
		
	public:

		static IFileSystem& Get();

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

		friend class FilePath;

	};


}