/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/PlatformImpl.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"


namespace Hyperion
{

	class IPlatformServices
	{

	public:

		virtual String GetExecutableName() = 0;
		virtual String GetExecutablePath() = 0;
		virtual String GetUserDataPath() = 0;

		virtual void Init() = 0;

	};

	template< typename _PlatformClass >
	class PlatformImpl
	{

	private:

		static std::unique_ptr< IPlatformServices > m_Impl;
		static IPlatformServices& Get()
		{
			if( !m_Impl )
			{
				m_Impl = std::make_unique< _PlatformClass >();
			}

			return *m_Impl;
		}

	public:

		static String GetExecutableName() { return Get().GetExecutableName(); }
		static String GetExecutablePath() { return Get().GetExecutablePath(); }
		static String GetUserDataPath() { return Get().GetUserDataPath(); }
		static void Init() { return Get().Init(); }

	};

	template< typename _PlatformClass >
	std::unique_ptr< IPlatformServices > PlatformImpl< _PlatformClass >::m_Impl;
}