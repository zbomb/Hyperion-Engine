/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Misc.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{

	struct ScreenResolution
	{
		uint32 Width;
		uint32 Height;
		bool FullScreen;

		bool LoadFromString( const String& inStr );
	};

}