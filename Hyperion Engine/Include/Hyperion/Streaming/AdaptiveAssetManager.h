/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/AdaptiveAssetManager.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Console/ConsoleVar.h"
#include "Hyperion/Streaming/DataTypes.h"
#include "Hyperion/Core/ThreadManager.h"
#include "Hyperion/Core/Types/ConcurrentQueue.h"
#include "Hyperion/Streaming/Events.h"



namespace Hyperion
{
	// Console variables
	extern ConsoleVar< uint32 > g_CVar_AdaptiveTexturePoolSize;
	extern ConsoleVar< uint32 > g_CVar_AdaptiveModelPoolSize;
	extern ConsoleVar< uint32 > g_CVar_AdaptiveTextureWorkerCount;
	extern ConsoleVar< uint32 > g_CVar_AdaptiveModelWorkerCount;
	extern ConsoleVar< float > g_CVar_TextureLODMult_Global;
	extern ConsoleVar< float > g_CVar_TextureLODMult_StaticModel;
	extern ConsoleVar< float > g_CVar_TextureLODMult_DynamicModel;
	extern ConsoleVar< float > g_CVar_TextureLODMult_Level;
	extern ConsoleVar< float > g_CVar_TextureLODMult_Character;


	// Forward Declarations
	class AdaptiveTexture;
	class AdaptiveModel;


	class AdaptiveAssetManagerTextureLoadRequest
	{

	public:

	};


	class AdaptiveAssetManagerModelLoadRequest
	{

	public:

	};
	

	class AdaptiveAssetManager
	{

	private:

		std::vector< std::shared_ptr< Thread > > m_TextureWorkerThreads;
		std::vector< std::shared_ptr< Thread > > m_ModelWorkerThreads;

		std::map< uint32, std::shared_ptr< AdaptiveTexture > > m_Textures;
		std::map< uint32, std::shared_ptr< AdaptiveModel > > m_Models;

		std::vector< std::weak_ptr< AdaptiveTexture > > m_SortedTextures;
		std::vector< std::weak_ptr< AdaptiveModel > > m_SortedModels;

		std::vector< std::unique_ptr< AdaptiveAssetManagerTextureLoadRequest > > m_TextureLoadQueue;
		std::vector< std::unique_ptr< AdaptiveAssetManagerModelLoadRequest > > m_ModelLoadQueue;

		std::mutex m_TextureMutex;
		std::mutex m_ModelMutex;

		std::map< uint32, std::shared_ptr< AdaptiveAssetManagerObjectInfo > > m_Objects;

		AdaptiveAssetManagerCameraInfo m_CameraInfo;
		ConcurrentQueue< std::unique_ptr< AdaptiveAssetManagerEventBase > > m_EventQueue;

		float m_AdaptiveQualityMult;

		std::shared_ptr< Thread > m_Thread;
		void ThreadMain( CustomThread& );

		void UpdateSceneView();

		bool Initialize();
		void Shutdown();

		bool CreateOrUpdateTexture( const AdaptiveTextureInfo& inTexture, const std::shared_ptr< AdaptiveAssetManagerObjectInfo >& refObject );
		bool CreateOrUpdateModel( const AdaptiveModelInfo& inModel, const std::shared_ptr< AdaptiveAssetManagerObjectInfo >& refObject );

		void PerformTexturePass();
		void PerformModelPass();

		void SortTextureLoadRequests();


		void TextureWorkerThread( CustomThread& );
		void ModelWorkerThread( CustomThread& );

		// Friend in all event classes
		friend class AdaptiveAssetManagerSpawnEvent;
		friend class AdaptiveAssetManagerDespawnEvent;
		friend class AdaptiveAssetManagerResourceChangeEvent;
		friend class AdaptiveAssetManagerObjectUpdateEvent;
		friend class AdaptiveAssetManagerCameraUpdateEvent;
		friend class AdaptiveAssetManagerWorldResetEvent;

	public:

		AdaptiveAssetManager();
		~AdaptiveAssetManager();

		void OnPrimitiveSpawned( AdaptiveAssetManagerSpawnEvent& inEvent );
		void OnPrimitiveDeSpawned( AdaptiveAssetManagerDespawnEvent& inEvent );
		void OnPrimitiveChangedResources( AdaptiveAssetManagerResourceChangeEvent& inEvent );
		void OnPrimitiveUpdate( AdaptiveAssetManagerObjectUpdateEvent& inEvent );
		void OnCameraUpdate( AdaptiveAssetManagerCameraUpdateEvent& inEvent );
		void OnWorldReset( AdaptiveAssetManagerWorldResetEvent& inEvent );
	};

}