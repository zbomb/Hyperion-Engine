/*==================================================================================================
	Hyperion Engine
	Source/Lib/Hyperion.cpp
	© 2019, Zachary Berry
==================================================================================================*/


#include "Hyperion/Hyperion.h"


bool Hyperion::IsGameThread()
{
	return std::this_thread::get_id() == __gGameThreadId;
}

bool Hyperion::IsRenderThread()
{
	return std::this_thread::get_id() == __gRenderThreadId;
}

bool Hyperion::IsRenderMarshalThread()
{
	return std::this_thread::get_id() == __gRenderMarshalThreadId;
}