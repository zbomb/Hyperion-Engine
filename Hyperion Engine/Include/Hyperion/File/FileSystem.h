/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/File/FileSystem.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"
#include "Hyperion/File/IFile.h"
#include "Hyperion/File/IDirectory.h"
#include "Hyperion/File/FilePath.h"

#include <fstream>

#undef CreateFile
#undef OpenFile
#undef CreateDirectory
#undef OpenDirectory
#undef DeleteFile
#undef DeleteDirectory
#undef DirectoryExists
#undef FileExists


namespace Hyperion
{

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


	enum class DeleteResult
	{
		Error,
		DoesNotExist,
		Success
	};


	class FileMetaData
	{

		const FilePath m_Path;

	public:

		FileMetaData( const FilePath& inPath );

		FileMetaData() = delete;
	};


	class File : public IFile
	{

	protected:

		File( const FilePath& inPath, FileMode inMode, WriteMode inWriteMode );

		const FileMode m_Mode;
		const WriteMode m_WriteMode;

		std::unique_ptr< std::basic_filebuf< byte > > m_Buffer;

	public:

		File() = delete;
		File( const File& ) = delete;

		File& operator=( const File& ) = delete;
		File& operator=( File&& ) = delete;

		std::basic_streambuf< byte >* GetStreamBuffer() final;

		bool CanReadStream() const final;
		bool CanWriteStream() const final;

		bool IsValid() const final;
		size_t GetSize() const final;

		bool IsClamped() const final;

		inline FileMode GetFileMode() const { return m_Mode; }
		inline WriteMode GetWriteMode() const { return m_WriteMode; }

		std::unique_ptr< FileMetaData > GetMetaData() const;


		friend class FileSystem;

	};


	class Directory : public IDirectory
	{

	protected:

		Directory( const FilePath& inPath );

		void CacheContents() final;

	public:

		Directory() = delete;
		Directory( const Directory& ) = delete;
		
		Directory& operator=( const Directory& ) = delete;
		Directory& operator=( Directory&& ) = delete;

		std::unique_ptr< FileMetaData > GetMetaData() const;

		friend class FileSystem;

	};


	class FileSystem
	{

	private:

		static bool CreateNeededDirectories( const FilePath& inPath );
		static std::unique_ptr< FileMetaData > GetMetaData( const FilePath& inPath, bool bIsDirectory );

	public:

		static std::unique_ptr< File > OpenFile( const FilePath& inPath, FileMode inFileMode, WriteMode inWriteMode = WriteMode::Append, bool bCreateIfNotExists = false );
		static std::unique_ptr< File > CreateFile( const FilePath& inPath, FileMode inFileMode, WriteMode inWriteMode = WriteMode::Append, bool bFailIfExists = true );
		static bool FileExists( const FilePath& inPath );
		static DeleteResult DeleteFile( const FilePath& inPath );

		static std::unique_ptr< Directory > OpenDirectory( const FilePath& inPath, bool bCreateIfNotExists = false );
		static std::unique_ptr< Directory > CreateDirectory( const FilePath& inPath, bool bFailIfExists = true );
		static bool DirectoryExists( const FilePath& inPath );
		static DeleteResult DeleteDirectory( const FilePath& inPath, bool bFailIfHasContents = true );

		static void FindFiles( const FilePath& inPath, std::vector< FilePath >& Out, std::function< bool( const FilePath& ) > inPredicate, bool bIncludeSubFolders = false );

		static void FindFiles( const FilePath& inPath, std::vector< FilePath >& Out, bool bIncludeSubFolders = false );

		static bool Initialize( bool bDiscoverAssets );
		static void Shutdown();


		friend class File;
		friend class Directory;

	};


}