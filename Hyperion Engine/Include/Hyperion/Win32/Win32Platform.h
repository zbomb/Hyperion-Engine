/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Win32/Win32Platform.h
	� 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/PlatformImpl.h"

#if HYPERION_OS_WIN32


namespace Hyperion
{

	class Win32PlatformServices : public IPlatformServices
	{

	private:

		String m_ExecPath;
		String m_ExecName;

	public:

		Win32PlatformServices();
		Win32PlatformServices( const Win32PlatformServices& )				= delete;
		Win32PlatformServices( Win32PlatformServices&& )					= delete;
		Win32PlatformServices& operator=( const Win32PlatformServices& )	= delete;

		virtual String GetExecutableName() override;
		virtual String GetExecutablePath() override;
		virtual void Init() override;

	};
	
}

#endif