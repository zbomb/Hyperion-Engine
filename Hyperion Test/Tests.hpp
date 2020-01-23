
#pragma once

#include "Hyperion/Framework/TestEntity.h"
#include "Hyperion/Framework/TestComponent.h"
#include "Hyperion/Framework/World.h"
#include "Hyperion/Core/GameManager.h"
#include "ZLibTests.hpp"
#include "PNGTests.hpp"


void RunEngineTests()
{
	// Create the entity, attach the component, spawn them into the world
	auto newEnt = Hyperion::CreateObject< Hyperion::TestEntity >();
	auto newComp = Hyperion::CreateObject< Hyperion::TestComponent >();

	newEnt->AddComponent( newComp, "test" );
	if( !Hyperion::GameManager::GetInstance()->AddEntityToActiveWorld( newEnt ) )
	{
		Hyperion::Console::WriteLine( "[ERROR] Win32 Test: Failed to add test entity to the world!" );
	}
	else
	{
		Hyperion::Console::WriteLine( "[DEBUG] Win32 Test: Added entity to the active world" );
	}

}

void RunMiscTests()
{
	std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
	Hyperion::Tests::RunPNGTest();

}