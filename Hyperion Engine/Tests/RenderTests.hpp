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
		newEnt->SetPosition( Vector3D( 0.f, 0.f, 30.f ) );

		Console::WriteLine( "\n----> Quaternion tests" );

		// First, lets create a point at (x,y,z) [1,0,0]
		auto point = Vector3D( 1.f, 0.f, 0.f );

		// Now, lets create a quaternion to rotate it around the Z axis, 90 degrees
		// The result should be [0,-1,0]
		auto rot = Quaternion( Vector3D::GetWorldForward(), 90.f );
		auto result = rot.RotateVector( point );
		
		Console::WriteLine( "-------> Result #1:  ", result.ToString() );
		
		// rotate by another 45 degrees on the same axis
		result = rot.RotateVector( result );
		Console::WriteLine( "-------> Result #2: ", result.ToString() );

		// Create a new one along the same axis
		auto newRot = Quaternion( Vector3D::GetWorldForward(), 45.f );

		// Rotate again
		result = newRot.RotateVector( result );
		Console::WriteLine( "------> Result #3: ", result.ToString() );

		// Rotate 3 more times
		result = newRot.RotateVector( result );
		result = newRot.RotateVector( result );
		result = newRot.RotateVector( result );

		Console::WriteLine( "------> Final Result: ", result.ToString() );

		Console::WriteLine( "\n---> Euler Conversion tests...." );

		// Lets take an angle of... 90 pitch and attempt to apply it to 0,0,1
		auto fpoint		= Vector3D( 1.f, 0.f, 0.f );
		auto frot		= Quaternion( Angle3D( 0.f, 0.f, 75.f ) );

		// Roll -> Pitch -> Yaw

		auto fres = frot.RotateVector( fpoint );

		Console::WriteLine( "---------> Result #1: ", fres.ToString() );

		// Now, lets get the axis, rotation from the quaternion
		auto axisRot = frot.GetRotationAxis();
		auto rotAmount = frot.GetRotationAmount();

		Console::WriteLine( "--------> Axis: ", axisRot.ToString() );
		Console::WriteLine( "--------> Degrees: ", rotAmount );

		auto euler = frot.GetEulerAngles();
		Console::WriteLine( "--------> Euler: ", euler.ToString() );

		Console::WriteLine( "\n\n------------------- Geometry Tests --------------------" );

		// First, lets take two 4d vectors, and do some operations between them
		Vector4D first( 10.f, 5.f, 1.f, 3.f );
		Vector4D second( 1.f, 6.f, 12.f, 10.f );

		// Lets calculate the length of each
		Console::WriteLine( "-----> First Length: ", first.Length() );
		Console::WriteLine( "-----> Second Length: ", second.Length() );

		// Now, lets calcualte the dot product
		auto dotResult = first * second;

		Console::WriteLine( "-----> Dot Product: ", dotResult );

		// Calculate distance between the two vectors
		auto distResult = first.Distance( second );
		Console::WriteLine( "-----> Distance: ", distResult );

		// Calculate normals for each vector
		auto firstNorm = first.GetNormalized();
		auto secondNorm = second.GetNormalized();

		Console::WriteLine( "-----> First Normalized: ", firstNorm.ToString() );
		Console::WriteLine( "-----> Second Normalized: ", secondNorm.ToString() );

		Console::WriteLine( "\n----> Rederer Test Complete!" );
		Console::WriteLine( "---------------------------------------------------------------------------------------------" );
	}
}
}