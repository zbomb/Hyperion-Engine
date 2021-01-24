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
#include "Hyperion/Framework/PointLight.h"


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
		newEnt->SetPosition( Vector3D( 0.f, 0.f, 30.f ) );

		auto ent2 = CreateObject< TestEntity >();
		world->AddEntity( ent2 );
		ent2->SetPosition( Vector3D( 15.f, 0.f, 30.f ) );
		ent2->SetRotation( Angle3D( 0.f, 45.f, 0.f ) );

		auto ent3 = CreateObject< TestEntity >();
		world->AddEntity( ent3 );
		ent3->SetPosition( Vector3D( -5.f, 0.f, 40.f ) );
		ent3->SetRotation( Angle3D( 0.f, 60.f, 0.f ) );

		auto light = CreateObject< PointLight >();
		light->SetColor( Color3F( 1.f, 0.f, 0.f ) );
		light->SetBrightness( 0.8f );;
		light->SetRadius( 200.f );

		world->AddEntity( light );
		light->SetPosition( Vector3D( 0.f, 20.f, 0.f ) );
		
		auto olight = CreateObject< PointLight >();
		olight->SetColor( Color3F( 0.f, 0.f, 1.f ) );
		olight->SetBrightness( 0.8f );
		olight->SetRadius( 200.f );

		world->AddEntity( olight );
		olight->SetPosition( Vector3D( 0.f, 10.f, 50.f ) );

		Console::WriteLine( "\n----> Quaternion tests" );
		Console::WriteLine( "---------> Creating Quaternion with a roll of 45, yaw of 45" );

		Quaternion q( Angle3D( 0.f, 45.f, 45.f ) );
		Vector3D p( 1.f, 0.f, 0.f );

		Console::WriteLine( "--------> Transforming point at {1,0,0}" );

		auto p_prime = q.RotateVector( p );
		Console::WriteLine( "-------------> Result: ", p_prime.ToString() );

		auto eu = q.GetEulerAngles();
		Console::WriteLine( "------------> Euler Angles: ", eu.ToString() );

		Console::WriteLine( "\n----> Rederer Test Complete!" );
		Console::WriteLine( "---------------------------------------------------------------------------------------------" );
	}
}
}