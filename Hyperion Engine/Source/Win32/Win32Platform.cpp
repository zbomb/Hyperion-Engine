/*==================================================================================================
	Hyperion Engine
	Source/Win32/Win32Platform.cpp
	� 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Hyperion.h"

#if HYPERION_OS_WIN32

#include "Hyperion/Win32/Win32Platform.h"
#include <Windows.h>
#include <KnownFolders.h>
#include <ShlObj.h>
#include <filesystem>

#undef CreateFile
#undef DeleteFile
#undef CreateDirectory

namespace Hyperion
{

	Win32PlatformServices::Win32PlatformServices()
	{
		// Were going to cache the name of the executable at startup, so we dont have to keep calling
		// into the kernel to get the executable name and path every time
		std::vector< wchar_t > pathBuffer;
		DWORD bytesCopied = 0;

		// Since we dont really know the path size ahead of time, were just going to grow the buffer by MAX_PATH each call
		do
		{
			pathBuffer.resize( MAX_PATH + bytesCopied );
			bytesCopied = GetModuleFileName( NULL, pathBuffer.data(), (DWORD)pathBuffer.size() );
		}
		while( bytesCopied >= pathBuffer.size() );

		// Resize buffer to exactly fit the data
		pathBuffer.resize( bytesCopied );

		// Now were going to construct a wstring, and clear the buffer, this should remove any terminators in the source data
		std::wstring widePath( pathBuffer.data() );
		pathBuffer.clear();

		// Use std::filesystem to split our path into the exe filename, and the directory its in
		std::filesystem::path fullPath( widePath );
		std::filesystem::path execName = fullPath.filename();
		std::filesystem::path execPath = fullPath.parent_path();

		// Now, we need to get the 'Documents' folder for the active user
		PWSTR docPath = NULL;
		HRESULT res = SHGetKnownFolderPath( FOLDERID_Documents, 0, NULL, &docPath );

		if( SUCCEEDED( res ) )
		{
			std::filesystem::path dpath( docPath );
			m_UserPath = String( dpath.generic_u8string(), StringEncoding::UTF8 );
		}
		else
		{
			// We need a path so we will use the ExecPath as a default
			m_UserPath = String( execPath.generic_u8string(), StringEncoding::UTF8 );
			Console::WriteLine( "[ERROR] Win32PlatformServices: Failed to get the documents directory for current user! Defaulting to game directory..." );
		}

		// Make sure we free the memory allocated by the winapi call
		CoTaskMemFree( docPath );

		// Now, we have to turn these path objects into hyperion strings
		m_ExecName = String( execName.generic_u8string(), StringEncoding::UTF8 );
		m_ExecPath = String( execPath.generic_u8string(), StringEncoding::UTF8 );
	}


	String Win32PlatformServices::GetExecutableName()
	{
		return m_ExecName;
	}

	String Win32PlatformServices::GetExecutablePath()
	{
		return m_ExecPath;
	}

	String Win32PlatformServices::GetUserDataPath()
	{
		return m_UserPath;
	}

	void Win32PlatformServices::Init()
	{
		// We really just want the constructor to run, and a call to this when the engine is starting up will ensure that happens
	}

}



#endif