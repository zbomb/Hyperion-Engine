/*==================================================================================================
	Hyperion Engine
	Source/Lib/Hyperion.cpp
	© 2019, Zachary Berry
==================================================================================================*/


#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/GameManager.h"
#include "Hyperion/Core/RenderManager.h"
#include "Hyperion/Core/ThreadManager.h"


namespace Hyperion
{
	bool IsGameThread()
	{
		return( std::this_thread::get_id() == GameManager::GetThreadId() );
	}

	bool IsRenderThread()
	{
		return( std::this_thread::get_id() == RenderManager::GetThreadId() );
	}

	bool IsWorkerThread()
	{
		return ThreadManager::IsWorkerThread( std::this_thread::get_id() );
	}
}