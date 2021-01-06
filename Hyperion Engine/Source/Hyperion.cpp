/*==================================================================================================
	Hyperion Engine
	Source/Lib/Hyperion.cpp
	© 2019, Zachary Berry
==================================================================================================*/


#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/ThreadManager.h"
#include "Hyperion/Core/Engine.h"


namespace Hyperion
{
	bool IsGameThread()
	{
		auto th = Engine::Get()->GetGameThread();
		return( th.IsValid() ? std::this_thread::get_id() == th->GetSystemIdentifier() : false );
	}

	bool IsRenderThread()
	{
		auto th = Engine::Get()->GetRenderThread();
		return( th.IsValid() ? std::this_thread::get_id() == th->GetSystemIdentifier() : false );
	}

	bool IsWorkerThread()
	{
		return ThreadManager::IsWorkerThread( std::this_thread::get_id() );
	}
}