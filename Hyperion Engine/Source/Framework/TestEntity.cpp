/*==================================================================================================
	Hyperion Engine
	Source/Framework/TestEntity.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/TestEntity.h"
#include "Hyperion/Core/AssetManager.h"
#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Assets/StaticModelAsset.h"



namespace Hyperion
{

	void TestEntity::OnCreate()
	{
		// Create our static model component
		m_Comp = CreateObject< StaticModelComponent >();

		// Set the material
		m_Comp->SetMaterial( AssetManager::Get< MaterialAsset >( "materials/mat_test.hmat" ), 0 );
		m_Comp->SetModel( AssetManager::Get< StaticModelAsset >( "models/mdl_test.hsm" ) );

		AddComponent( m_Comp, "model" );
	}


	void TestEntity::OnDestroy()
	{

	}


	void TestEntity::OnSpawn( const HypPtr< World >& inWorld )
	{
		Console::WriteLine( "[DEBUG] TestEntity: On spawn!" );
	}


	void TestEntity::OnDespawn( const HypPtr< World >& inWorld )
	{
		Console::WriteLine( "[DEBUG] TestEntity: On despawn!" );
	}

}


/*
*	Register Type
*/
HYPERION_REGISTER_OBJECT_TYPE( TestEntity, Entity );