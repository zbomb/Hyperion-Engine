/*==================================================================================================
	Hyperion Engine
	Source/File/IFileSystem.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/File/IFileServices.h"
#include "Hyperion/Win32/Win32FileSystem.h"


namespace Hyperion
{
	// Static Definitions
	std::unique_ptr< IFileServices > IFileServices::m_Singleton;


	IFileServices& IFileServices::Get()
	{
		if( !m_Singleton )
		{
			#if HYPERION_OS_WIN32
			m_Singleton = std::make_unique< Win32FileServices >();
			#else
			HYPERION_NOT_IMPLEMENTED( "IFileService implementations for OS's other than Win32" );
			#endif
		}

		return *( m_Singleton.get() );
	}

}