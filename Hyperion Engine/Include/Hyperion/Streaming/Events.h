/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/Events.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Streaming/DataTypes.h"


namespace Hyperion
{
	// Forward Declarations
	class AdaptiveAssetManager;


	/*----------------------------------------------------------------------
		Event Baseclass
	----------------------------------------------------------------------*/
	struct AdaptiveAssetManagerEventBase
	{
		virtual void Execute( AdaptiveAssetManager& ) = 0;
	};


	/*----------------------------------------------------------------------
		Spawn Event
	----------------------------------------------------------------------*/
	struct AdaptiveAssetManagerSpawnEvent : public AdaptiveAssetManagerEventBase
	{
		AdaptiveAssetManagerObjectInfo ObjectInfo;
		std::vector< AdaptiveTextureInfo > Textures;
		std::vector< AdaptiveModelInfo > Models;

		void Execute( AdaptiveAssetManager& ) final;
	};


	/*----------------------------------------------------------------------
		Despawn Event
	----------------------------------------------------------------------*/
	struct AdaptiveAssetManagerDespawnEvent : public AdaptiveAssetManagerEventBase
	{
		uint32 ObjectIdentifier;

		void Execute( AdaptiveAssetManager& ) final;
	};


	/*----------------------------------------------------------------------
		Resource Change Event
	----------------------------------------------------------------------*/
	struct AdaptiveAssetManagerResourceChangeEvent : public AdaptiveAssetManagerEventBase
	{
		AdaptiveAssetManagerObjectInfo Object;

		std::vector< AdaptiveTextureInfo > NewTextures;
		std::vector< AdaptiveModelInfo > NewModels;

		std::vector< uint32 > RemovedAssets;

		void Execute( AdaptiveAssetManager& ) final;
	};


	/*----------------------------------------------------------------------
		Object Update Entry
	----------------------------------------------------------------------*/
	struct AdaptiveAssetManagerObjectUpdateEntry
	{
		uint32 Identifier;
		Vector3D Position;
		float Radius;
	};


	/*----------------------------------------------------------------------
		Object Update Event
	----------------------------------------------------------------------*/
	struct AdaptiveAssetManagerObjectUpdateEvent : public AdaptiveAssetManagerEventBase
	{
		std::vector< AdaptiveAssetManagerObjectUpdateEntry > Entries;

		void Execute( AdaptiveAssetManager& ) final;
	};


	/*----------------------------------------------------------------------
		Camera Update Event 
	----------------------------------------------------------------------*/
	struct AdaptiveAssetManagerCameraUpdateEvent : public AdaptiveAssetManagerEventBase
	{
		AdaptiveAssetManagerCameraInfo CameraInfo;

		void Execute( AdaptiveAssetManager& ) final;
	};

	/*----------------------------------------------------------------------
		World Reset Event
	----------------------------------------------------------------------*/
	struct AdaptiveAssetManagerWorldResetEvent : public AdaptiveAssetManagerEventBase
	{
		void Execute( AdaptiveAssetManager& ) final;
	};

}