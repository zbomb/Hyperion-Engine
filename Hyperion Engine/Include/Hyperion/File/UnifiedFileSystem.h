/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/File/UnifiedFileSystem.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"
#include "Hyperion/Console/ConsoleVar.h"
#include "Hyperion/File/FilePath.h"
#include "Hyperion/File/IFile.h"
#include "Hyperion/File/IDirectory.h"



namespace Hyperion
{
	/*
		Console Variables
	*/
	extern ConsoleVar< String > g_CVar_ContentSystem;


	class UnifiedFileSystem
	{

	private:

		static bool m_bInit;
		static FileSystem m_ContentSystem;

		static FileSystem DetermineFileSystem( const FilePath& inPath );

	public:

		static void Initialize();
		static void Shutdown();

		static std::unique_ptr< IFile > OpenFile( const FilePath& inPath );
		static bool FileExists( const FilePath& inPath );

		static std::unique_ptr< IDirectory > OpenDirectory( const FilePath& inPath );
		static bool DirectoryExists( const FilePath& inPath );

	};


}