/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/File/PhysicalFileSystem.h
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


	class PhysicalMetaData
	{

		const FilePath m_Path;

	public:

		PhysicalMetaData( const FilePath& inPath );

		PhysicalMetaData() = delete;
	};


	class PhysicalFile : public IFile
	{

	protected:

		PhysicalFile( const FilePath& inPath, FileMode inMode, WriteMode inWriteMode );

		const FileMode m_Mode;
		const WriteMode m_WriteMode;

		std::unique_ptr< std::basic_filebuf< byte > > m_Buffer;

	public:

		PhysicalFile() = delete;
		PhysicalFile( const PhysicalFile& ) = delete;

		PhysicalFile& operator=( const PhysicalFile& ) = delete;
		PhysicalFile& operator=( PhysicalFile&& ) = delete;

		std::basic_streambuf< byte >* GetStreamBuffer() final;

		bool CanReadStream() const final;
		bool CanWriteStream() const final;

		bool IsValid() const final;
		size_t GetSize() const final;

		bool IsClamped() const final;

		inline FileMode GetFileMode() const { return m_Mode; }
		inline WriteMode GetWriteMode() const { return m_WriteMode; }

		std::unique_ptr< PhysicalMetaData > GetMetaData() const;


		friend class PhysicalFileSystem;

	};


	class PhysicalDirectory : public IDirectory
	{

	protected:

		PhysicalDirectory( const FilePath& inPath );

		void CacheContents() final;

	public:

		PhysicalDirectory() = delete;
		PhysicalDirectory( const PhysicalDirectory& ) = delete;
		
		PhysicalDirectory& operator=( const PhysicalDirectory& ) = delete;
		PhysicalDirectory& operator=( PhysicalDirectory&& ) = delete;

		std::unique_ptr< PhysicalMetaData > GetMetaData() const;

		friend class PhysicalFileSystem;

	};


	class PhysicalFileSystem
	{

	private:

		static bool CreateNeededDirectories( const FilePath& inPath );
		static std::unique_ptr< PhysicalMetaData > GetMetaData( const FilePath& inPath, bool bIsDirectory );

	public:

		static std::unique_ptr< PhysicalFile > OpenFile( const FilePath& inPath, FileMode inFileMode, WriteMode inWriteMode = WriteMode::Append, bool bCreateIfNotExists = false );
		static std::unique_ptr< PhysicalFile > CreateFile( const FilePath& inPath, FileMode inFileMode, WriteMode inWriteMode = WriteMode::Append, bool bFailIfExists = true );
		static bool FileExists( const FilePath& inPath );
		static DeleteResult DeleteFile( const FilePath& inPath );

		static std::unique_ptr< PhysicalDirectory > OpenDirectory( const FilePath& inPath, bool bCreateIfNotExists = false );
		static std::unique_ptr< PhysicalDirectory > CreateDirectory( const FilePath& inPath, bool bFailIfExists = true );
		static bool DirectoryExists( const FilePath& inPath );
		static DeleteResult DeleteDirectory( const FilePath& inPath, bool bFailIfHasContents = true );

		static void FindFiles( const FilePath& inPath, std::vector< FilePath >& Out, std::function< bool( const FilePath& ) > inPredicate, bool bIncludeSubFolders = false );

		static void FindFiles( const FilePath& inPath, std::vector< FilePath >& Out, bool bIncludeSubFolders = false );

		static bool Initialize( bool bDiscoverAssets );
		static void Shutdown();


		friend class PhysicalFile;
		friend class PhysicalDirectory;

	};


}