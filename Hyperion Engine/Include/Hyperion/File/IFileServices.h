/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/File/IFileSystem.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"
#include "Hyperion/File/FilePath.h"

#include <filesystem>


namespace Hyperion
{

	class IFileServices
	{

	private:

		static std::unique_ptr< IFileServices > m_Singleton;

	public:

		static IFileServices& Get();
		virtual String GetLocalPathLocation( LocalPath inLocal ) = 0;

	};

}