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


	uint32 Pow( uint32 base, uint32 exp )
	{
		uint32 output = 1;

		for( uint32 i = 0; i < exp; i++ )
		{
			if( i == 0 )
				output = base;
			else
				output *= base;
		}

		return output;
	}
}