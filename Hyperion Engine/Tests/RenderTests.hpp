/*==================================================================================================
	Hyperion Engine
	Tests/RenderTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <iostream>

#include "Hyperion/Framework/World.h"
#include "Hyperion/Framework/Entity.h"
#include "Hyperion/Framework/Component.h"


namespace Hyperion
{



namespace Tests
{

	void RunRendererTests()
	{
		Console::WriteLine( "\n---------------------------------------------------------------------------------------------\n[TEST] Running rederer test..." );


		Console::WriteLine( "\n----> Rederer Test Complete!" );
		Console::WriteLine( "---------------------------------------------------------------------------------------------" );
	}
}
}