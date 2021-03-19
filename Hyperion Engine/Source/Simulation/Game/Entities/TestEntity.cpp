/*==================================================================================================
	Hyperion Engine
	Source/Simulation/Game/Entities/TestEntity.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Simulation/Game/Entities/TestEntity.h"
#include "Hyperion/Core/AssetManager.h"
#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Assets/StaticModelAsset.h"



namespace Hyperion
{

	void TestEntity::OnCreate()
	{
		// Create our static model component
		m_Comp = CreateObject< StaticModelComponent >();

		// Set the material'
		auto mat = AssetManager::Get< MaterialAsset >( "materials/mip_test.hmat" );
		auto mdl = AssetManager::Get< StaticModelAsset >( "models/test_model.hsm" );

		m_Comp->SetMaterial( mat, 0 );
		m_Comp->SetModel( mdl );

		AddComponent( m_Comp, "model" );

		bRequiresTick = true;
	}


	void TestEntity::OnDestroy()
	{

	}


	void TestEntity::OnSpawn( const HypPtr< World >& inWorld )
	{

	}


	void TestEntity::OnDespawn( const HypPtr< World >& inWorld )
	{

	}


	void TestEntity::Tick( double inDelta )
	{
		m_Comp->Rotate( Angle3D( 0.f, inDelta * 60.0, 0.f ) );
	}

}


/*
*	Register Type
*/
HYPERION_REGISTER_OBJECT_TYPE( TestEntity, Entity );