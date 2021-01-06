/*==================================================================================================
	Hyperion Engine
	Source/Win32/Win32FileSystem.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Hyperion.h"

#if HYPERION_OS_WIN32

#include "Hyperion/Win32/Win32FileSystem.h"
#include "Hyperion/Core/Platform.h"

#include <sys/stat.h>

#undef CreateFile
#undef DeleteFile
#undef CreateDirectory

namespace Hyperion
{

	String Win32FileServices::GetRootPathLocation( PathRoot inPath )
	{
		switch( inPath )
		{
		case PathRoot::SystemRoot:
			return String();

		case PathRoot::Game:
			return Platform::GetExecutablePath().Append( "/" );

		case PathRoot::Content:
			return Platform::GetExecutablePath().Append( "/content/" );

		case PathRoot::Documents:
		default:
			return Platform::GetUserDataPath().Append( "/" ).Append( HYPERION_DOC_FOLDER );
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