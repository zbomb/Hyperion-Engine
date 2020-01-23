/*==================================================================================================
	Hyperion Engine
	Source/Framework/TestEntity.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/TestEntity.h"


namespace Hyperion
{

	void TestEntity::OnSpawn( const HypPtr< World >& inWorld )
	{
		Console::WriteLine( "[DEBUG] TestEntity: On spawn!" );
	}


	void TestEntity::OnDespawn( const HypPtr< World >& inWorld )
	{
		Console::WriteLine( "[DEBUG] TestEntity: On despawn!" );
	}

}