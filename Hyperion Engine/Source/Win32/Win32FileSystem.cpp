/*==================================================================================================
	Hyperion Engine
	Source/Win32/Win32FileSystem.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Hyperion.h"

#if HYPERION_OS_WIN32

#include "Hyperion/Win32/Win32FileSystem.h"
#include "Hyperion/Core/Platform.h"
#include "Hyperion/Core/GameManager.h"

#include <sys/stat.h>

#undef CreateFile
#undef DeleteFile
#undef CreateDirectory

namespace Hyperion
{

	String Win32FileServices::GetLocalPathLocation( LocalPath inPath )
	{
		switch( inPath )
		{
		case LocalPath::Root:
			return String();

		case LocalPath::Game:
			static String Output = Platform::GetExecutablePath();
			return Output;

		case LocalPath::Content:
			static String Output = Platform::GetExecutablePath().Append( "/content" );
			return Output;

		case LocalPath::Documents:
			static String Output = Platform::GetUserDataPath().Append( "/" ).Append( GameManager::GetInstance()->GetDocumentsFolderName() );
			return Output;

		}
	}


	/*
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
	*/

}




#endif