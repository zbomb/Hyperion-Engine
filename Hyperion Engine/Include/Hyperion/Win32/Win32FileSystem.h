/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Win32/Win32FileSystem.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/File/IFileServices.h"



namespace Hyperion
{

	class Win32FileServices : public IFileServices
	{

	public:

		String GetLocalPathLocation( LocalPath inLocal ) final;

	};


	/*
	class Win32FileSystem : public IFileServices
	{
	private:

		bool CreateNeededDirectories( const FilePath& );

	protected:

		virtual std::filesystem::path BuildSystemPath( const String& inPath, PathRoot Root ) override;
		virtual std::time_t GetLastWrite( const FilePath& Target ) override;

	public:

		virtual std::unique_ptr< File > OpenFile( const FilePath& inPath, FileMode inMode, WriteMode inWrite = WriteMode::Append, bool bCreateIfNotExists = false ) override;
		virtual std::unique_ptr< File > CreateFile( const FilePath& inPath, FileMode inMode, WriteMode inWrite = WriteMode::Append, bool bFailIfExists = true ) override;
		virtual bool FileExists( const FilePath& inPath ) override;
		virtual DeleteResult DeleteFile( const FilePath& inPath ) override;
		virtual std::unique_ptr< MetaData > GetFileMetaData( const FilePath& inPath ) override;

		virtual std::unique_ptr< Directory > OpenDirectory( const FilePath& inPath, bool bCreateIfNotExists = false ) override;
		virtual std::unique_ptr< Directory > CreateDirectory( const FilePath& inPath, bool bFailIfExists = true ) override;
		virtual bool DirectoryExists( const FilePath& inPath ) override;
		virtual DeleteResult DeleteDirectory( const FilePath& inPath, bool bFailIfHasContents = true ) override;
		virtual std::unique_ptr< MetaData > GetDirectoryMetaData( const FilePath& inPath ) override;

	};
	*/

}