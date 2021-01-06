/*==================================================================================================
	Hyperion Engine
	Tests/RenderTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <iostream>

#include "Hyperion/Framework/World.h"
#include "Hyperion/Framework/TestEntity.h"
#include "Hyperion/Core/GameInstance.h"


namespace Hyperion
{



namespace Tests
{

	void RunRendererTests()
	{
		Console::WriteLine( "\n---------------------------------------------------------------------------------------------\n[TEST] Running rederer test..." );

		// Lets create some type of entity, thats renderable
		auto newEnt = CreateObject< TestEntity >();
		auto world = Engine::GetGame()->GetWorld();

		world->AddEntity( newEnt );

		Console::WriteLine( "\n----> Rederer Test Complete!" );
		Console::WriteLine( "---------------------------------------------------------------------------------------------" );
	}
}
}