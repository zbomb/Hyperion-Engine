/*==================================================================================================
	Hyperion Engine
	Source/Streaming/DynamicQualityAssetManager.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Streaming/DynamicQualityAssetManager.h"
#include "Hyperion/Framework/StaticModelComponent.h"
#include "Hyperion/Framework/DynamicModelComponent.h"
#include "Hyperion/Assets/MaterialAsset.h"
#include "Hyperion/Assets/StaticModelAsset.h"
#include "Hyperion/Material/MaterialInstance.h"
#include "Hyperion/Framework/World.h"



namespace Hyperion
{

	bool DynamicQualityAssetManager::RegisterComponent( const HypPtr< StaticModelComponent >& inComponent )
	{
		// Validate parameter
		if( !inComponent || !inComponent->IsActive() )
		{
			Console::WriteLine( "[ERROR] DynamicQualityAssetManager: Failed to register static model component.. it wasnt valid!" );
			return false;
		}

		// Figure out what the model file and texture files used by this asset are
		auto modelAsset		= inComponent->GetModelAsset();
		auto material		= inComponent->GetMaterial(); // TODO: Allow multiple materials

		if( !material )
		{
			auto defMaterialAsset = modelAsset->GetDefaultMaterial();

			// We need to instantiate this material
			// TODO
			material = std::make_shared< MaterialInstance >();

			if( !material )
			{
				Console::WriteLine( "[WARNING] DynamicQualityAssetManager: Failed to get material for model '", modelAsset->GetAssetPath().GetPath(), "'" );
			}
		}

		// Now we need to come up with a list of textures
		std::vector< uint64 > textureIds = material ? material->BuildNeededTextureList() : std::vector< uint64 >();

		// Calculate screen size of this component based on the model asset
		auto world = inComponent->GetWorld();
		HYPERION_VERIFY( world, "World was invalid for registered component" );

		auto viewport = world->GetViewState();

		// TODO: Using viewport and component bounding sphere, calculate a screen percentage
		// How to create boudning sphere? From metadata inside of our model file


		// Based on the size of the primitive, we can calculate an 'ideal' texture resolution, and
		// using model parameters, determine an ideal LOD level for the model

		// Check cache for these textures and the model
		// If something is already cached, checked if we have the proper LODs loaded
		// Either way, we return the cached instance
		// If we dont have the LODs, we can enqueue work to load in the ones we want

		// We then create a StaticModel class and a TextureInstance that are empty, only containing metadata
		// Insert these into the cache
		
		// Then, we kick off an async thread to load the desired LODs in

		return false;
	}

	bool DynamicQualityAssetManager::RegisterComponent( const HypPtr< DynamicModelComponent >& inComponent )
	{
		return false;
	}

	bool DynamicQualityAssetManager::DeRegisterComponent( const HypPtr< StaticModelComponent >& inComponent )
	{
		return false;
	}

	bool DynamicQualityAssetManager::DeRegisterComponent( const HypPtr< DynamicModelComponent >& inComponent )
	{
		return false;
	}

	void DynamicQualityAssetManager::OnWorldChanged( const HypPtr< World >& inNewWorld )
	{

	}

}
