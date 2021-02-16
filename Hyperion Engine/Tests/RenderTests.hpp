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
#include "Hyperion/Framework/TransparencyTestEntity.h"


namespace Hyperion
{



namespace Tests
{

	void RunRendererTests()
	{
		Console::WriteLine( "\n---------------------------------------------------------------------------------------------\n[TEST] Running rederer test..." );

		auto world = Engine::GetGame()->GetWorld();

		// Time for a intense render test...
		uint32 objRowCount = 100;
		uint32 objColCount = 10;

		uint32 lightRowCount = 10;
		uint32 lightColCount = 10;

		float rowSize = 1000.f;
		float colSize = 1000.f;

		float objRowSpacing = rowSize / (float) objRowCount;
		float objColSpacing = colSize / (float) objColCount;
		float lightRowSpacing = rowSize / (float) lightRowCount;
		float lightColSpacing = colSize / (float) lightColCount;

		float yaw = 0.f;

		for( uint32 c = 0; c < objColCount; c++ )
		{
			for( uint32 r = 0; r < objRowCount; r++ )
			{
				yaw += 10.f;

				float x = -( colSize / 2.f ) + (float) c * objColSpacing;
				float z = 25.f + (float) r * objRowSpacing;

				auto ent = CreateObject< TestEntity >();
				world->AddEntity( ent );

				ent->SetPosition( Vector3D( x, 0.f, z ) );
				ent->SetRotation( Angle3D( 0.f, yaw, 0.f ) );
			}
		}

		int k = 0;

		for( uint32 c = 0; c < lightColCount; c++ )
		{
			for( uint32 r = 0; r < lightRowCount; r++ )
			{
				float x = -( colSize / 2.f ) + (float) c * lightColSpacing;
				float z = 25.f + (float) r * lightRowSpacing;

				auto light = CreateObject< PointLight >();
				light->SetBrightness( 0.8f );
				light->SetRadius( 200.f );
				
				/*
				switch( k )
				{
				case 0:
					light->SetColor( Color3F( 1.f, 0.f, 0.f ) );
					break;
				case 1:
					light->SetColor( Color3F( 0.f, 1.f, 0.f ) );
					break;
				case 2:
					light->SetColor( Color3F( 0.f, 0.f, 1.f ) );
					break;
				case 3:
					light->SetColor( Color3F( 1.f, 1.f, 1.f ) );
					break;
				}

				k++;
				if( k > 3 ) { k = 0; }
				*/

				light->SetColor( Color3F( 1.f, 1.f, 1.f ) );

				world->AddEntity( light );
				light->SetPosition( Vector3D( x, 20.f, z ) );
			}
		}

		Console::WriteLine( "\n----> Rederer Test Complete!" );
		Console::WriteLine( "---------------------------------------------------------------------------------------------" );
	}
}
}