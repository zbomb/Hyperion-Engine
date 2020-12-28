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

// DEBUG: Comment this out to turn off debug console prints
#define HYPERION_TEXTURE_STREAMING_DEBUG
#define HYPERION_MODEL_STREAMING_DEBUG


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
	extern ConsoleVar< uint32 > g_CVar_TextureMaxResidentMemory;
	extern ConsoleVar< uint32 > g_CVar_AdaptiveModel_MaxUnloadsPerTick;
	extern ConsoleVar< uint32 > g_CVar_AdaptiveModel_MaxLoadsPerTick;
	extern ConsoleVar< float > g_CVar_ModelLODMult_Global;
	extern ConsoleVar< float > g_CVar_ModelLODMult_StaticModel;
	extern ConsoleVar< float > g_CVar_ModelLODMult_DynamicModel;
	extern ConsoleVar< float > g_CVar_ModelLODMult_Level;
	extern ConsoleVar< float > g_CVar_ModelLODMult_Character;



	// Forward Declarations
	class AdaptiveTexture;
	class AdaptiveModel;

	struct AdaptiveTextureRequestBase;
	struct AdaptiveTextureLoadRequest;
	struct AdaptiveTextureUnloadRequest;
	struct AdaptiveModelLoadRequest;
	struct AdaptiveModelUnloadRequest;
	

	class AdaptiveAssetManager
	{

	private:

		std::vector< std::shared_ptr< Thread > > m_TextureWorkerThreads;
		std::vector< std::shared_ptr< Thread > > m_ModelWorkerThreads;

		std::map< uint32, std::shared_ptr< AdaptiveTexture > > m_Textures;
		std::map< uint32, std::shared_ptr< AdaptiveModel > > m_Models;

		std::vector< std::weak_ptr< AdaptiveTexture > > m_SortedTextures;
		std::vector< std::weak_ptr< AdaptiveModel > > m_SortedModels;

		std::deque< std::shared_ptr< AdaptiveTextureLoadRequest > > m_TextureLoadQueue;
		std::deque< std::shared_ptr< AdaptiveTextureUnloadRequest > > m_TextureUnloadQueue;

		std::vector< std::shared_ptr< AdaptiveModelLoadRequest > > m_ModelLoadQueue;
		std::vector< std::shared_ptr< AdaptiveModelUnloadRequest > > m_ModelUnloadQueue;

		std::mutex m_TextureMutex;
		std::mutex m_ModelMutex;

		uint32 m_MemoryUsage;

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

		void SortTextures();
		void SortTextureLoadRequests();
		void SortTextureUnloadRequests();
		void FinishDestroyTexture( AdaptiveTexture& inTexture );

		void SortModelLoadRequests();
		void SortModelUnloadRequests();
		void FinishDestroyModel( AdaptiveModel& inModel );

		bool ModelWorker_PerformLoad( const std::shared_ptr< AdaptiveModelLoadRequest >& inRequest );
		void ModelWorker_PerformUnload( const std::shared_ptr< AdaptiveModelUnloadRequest >& inRequest );

		void TextureWorker_Main( CustomThread& );
		void ModelWorker_Main( CustomThread& );

		// Friend in all event classes
		friend struct AdaptiveAssetManagerSpawnEvent;
		friend struct AdaptiveAssetManagerDespawnEvent;
		friend struct AdaptiveAssetManagerResourceChangeEvent;
		friend struct AdaptiveAssetManagerObjectUpdateEvent;
		friend struct AdaptiveAssetManagerCameraUpdateEvent;
		friend struct AdaptiveAssetManagerWorldResetEvent;

		// Helper functions for texture worker
		void TextureWorker_SelectRequests( std::shared_ptr< AdaptiveTextureLoadRequest >& outLoad, 
										   std::vector< std::shared_ptr< AdaptiveTexture > >& outForceDrops, std::vector< std::shared_ptr< AdaptiveTextureUnloadRequest > >& outPureDrops );

		bool TextureWorker_PerformLoad( const std::shared_ptr< AdaptiveTextureLoadRequest >& inRequest );


		std::deque< std::shared_ptr< AdaptiveTextureLoadRequest > >::iterator TextureWorker_PopNextLoadRequest();

		bool TextureWorker_DropLevelFromLoad( std::shared_ptr< AdaptiveTextureLoadRequest >& inReq );

		bool TextureWorker_GenerateForceDropList( uint32 dropCount, float loadPriority, uint32 neededMemory, 
												  const std::vector< std::shared_ptr< AdaptiveTextureUnloadRequest > >& selectedUnloads, 
												  std::vector< std::shared_ptr< AdaptiveTexture > >& outList );

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


	struct AdaptiveAssetManagerTextureSort
	{
		inline bool operator()( const std::shared_ptr< AdaptiveTexture >& lhs, const std::shared_ptr< AdaptiveTexture >& rhs );
		inline bool operator()( const std::weak_ptr< AdaptiveTexture >& lhs, const std::weak_ptr< AdaptiveTexture >& rhs );
	};

	struct AdaptiveAssetManagerTextureLoadRequestSort
	{
		inline bool operator()( const std::shared_ptr< AdaptiveTextureLoadRequest >& lhs, const std::shared_ptr< AdaptiveTextureLoadRequest >& rhs );
	};

	struct AdaptiveAssetManagerTextureUnloadRequestSort
	{
		inline bool operator()( const std::shared_ptr< AdaptiveTextureUnloadRequest >& lhs, const std::shared_ptr< AdaptiveTextureUnloadRequest >& rhs );
	};

	struct AdaptiveAssetManagerModelLoadRequestSort
	{
		inline bool operator()( const std::shared_ptr< AdaptiveModelLoadRequest >& lhs, const std::shared_ptr< AdaptiveModelLoadRequest >& rhs );
	};

	struct AdaptiveAssetManagerModelUnloadRequestSort
	{
		inline bool operator()( const std::shared_ptr< AdaptiveModelUnloadRequest >& lhs, const std::shared_ptr< AdaptiveModelUnloadRequest >& rhs );
	};

}