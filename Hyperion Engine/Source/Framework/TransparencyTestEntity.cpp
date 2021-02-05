/*==================================================================================================
	Hyperion Engine
	Source/Framework/TransparencyTetstEntity.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/TransparencyTestEntity.h"
#include "Hyperion/Core/AssetManager.h"
#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Assets/StaticModelAsset.h"



namespace Hyperion
{

	void TransparencyTestEntity::OnCreate()
	{
		// Create our static model component
		m_Comp = CreateObject< StaticModelComponent >();

		// Set the material'
		auto mat = AssetManager::Get< MaterialAsset >( "materials/glass1.hmat" );
		auto mdl = AssetManager::Get< StaticModelAsset >( "models/test_model.hsm" );

		m_Comp->SetMaterial( mat, 0 );
		m_Comp->SetModel( mdl );

		AddComponent( m_Comp, "model" );

		bRequiresTick = true;
	}


	void TransparencyTestEntity::OnDestroy()
	{

	}


	void TransparencyTestEntity::OnSpawn( const HypPtr< World >& inWorld )
	{

	}


	void TransparencyTestEntity::OnDespawn( const HypPtr< World >& inWorld )
	{

	}


	void TransparencyTestEntity::Tick( double inDelta )
	{
		//m_Comp->Rotate( Angle3D( 0.f, 1.f, 0.f ) );
	}

}


/*
*	Register Type
*/
HYPERION_REGISTER_OBJECT_TYPE( TransparencyTestEntity, Entity );