/*==================================================================================================
	Hyperion Engine
	Source/Streaming/AdaptiveAssetManager.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Streaming/AdaptiveAssetManager.h"
#include "Hyperion/Streaming/AdaptiveTexture.h"
#include "Hyperion/Streaming/AdaptiveStaticModel.h"
#include "Hyperion/Streaming/AdaptiveDynamicModel.h"
#include "Hyperion/Library/Geometry.h"
#include "Hyperion/Assets/StaticModelAsset.h"
#include "Hyperion/Assets/DynamicModelAsset.h"
#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Library/Math.h"



namespace Hyperion
{
	/*=============================================================================================================
		Console Variables
	=============================================================================================================*/
	static ConsoleVar< uint32 > g_CVar_AdaptiveTexturePoolSize(
		"r_adaptive_texture_pool_size", "The amount of memory available to the adaptive texture pool in MB",
		2048, 1024, 64 * 1024 ); // 64GB Max?

	static ConsoleVar< uint32 > g_CVar_AdaptiveModelPoolSize(
		"r_adaptive_model_pool_size", "The amount of memory available to the adaptive model pool in MB",
		1024, 256, 32 * 1024 ); // 32GB Max?

	// TODO: Cant really hook 'onChanged' since we dont have access here to the aa manager instance..
	// So, changing this after aa manager inits, wont actually do anything.. startup only!
	static ConsoleVar< uint32 > g_CVar_AdaptiveTextureWorkerCount(
		"r_adaptive_texture_workers", "The number of worker threads to use when loading in textures",
		2, 1, 8 );

	// Same here!
	static ConsoleVar< uint32 > g_CVar_AdaptiveModelWorkerCount(
		"r_adaptive_model_workers", "The number of worker threads to use when loading in models",
		1, 1, 8 );

	static ConsoleVar< float > g_CVar_TextureLODMult_Global(
		"r_texture_lod_mult_global", "Multiplier applied when selecting an LOD for a texture",
		0.01f, 10.f, 1.f );

	static ConsoleVar< float > g_CVar_TextureLODMult_StaticModel(
		"r_texture_lod_mult_static_model", "Multiplier applied when selecting an LOD for a texture applied to a static model",
		0.01f, 10.f, 1.f );

	static ConsoleVar< float > g_CVar_TextureLODMult_DynamicModel(
		"r_texture_lod_mult_dynamic_model", "Multiplier applied when selecting an LOD for a texture applied to a dynamic model",
		0.01f, 10.f, 1.f );

	static ConsoleVar< float > g_CVar_TextureLODMult_Level(
		"r_texture_lod_mult_level", "Multiplier applied when selecting an LOD for a texture applied to the level",
		0.01f, 10.f, 1.f );

	static ConsoleVar< float > g_CVar_TextureLODMult_Character(
		"r_texture_lod_mult_character", "Multiplier applied when selecting an LOD for a texture applied to a character",
		0.01f, 10.f, 1.f );



	/*=============================================================================================================
		Event Handler Functions
	=============================================================================================================*/


	// Spawn Event
	void AdaptiveAssetManagerSpawnEvent::Execute( AdaptiveAssetManager& inManager )
	{
		// Validate object info (just check identifier)
		if( ObjectInfo.m_Identifier == 0 )
		{
			return;
		}

		// Update or create the object if needed
		auto entry = inManager.m_Objects.find( ObjectInfo.m_Identifier );
		std::shared_ptr< AdaptiveAssetManagerObjectInfo > ptr( nullptr );

		if( entry != inManager.m_Objects.end() )
		{
			if( !entry->second )
			{
				entry->second = std::make_shared< AdaptiveAssetManagerObjectInfo >();
			}

			ptr = entry->second;
		}
		else
		{
			ptr = inManager.m_Objects[ ObjectInfo.m_Identifier ] = std::make_shared< AdaptiveAssetManagerObjectInfo >();
		}

		*ptr			= ObjectInfo;

		ptr->m_Valid	= true;
		ptr->m_Dirty	= true;

		// Now, add or update the assets in the event structure
		for( auto It = Textures.begin(); It != Textures.end(); It++ )
		{
			inManager.CreateOrUpdateTexture( *It, ptr );
		}

		for( auto It = Models.begin(); It != Models.end(); It++ )
		{
			inManager.CreateOrUpdateModel( *It, ptr );
		}
	}

	// Despawn Event
	void AdaptiveAssetManagerDespawnEvent::Execute( AdaptiveAssetManager& inManager )
	{
		// Validate object info
		if( ObjectIdentifier == 0 )
		{
			return;
		}

		// Find this object
		auto entry = inManager.m_Objects.find( ObjectIdentifier );
		if( entry != inManager.m_Objects.end() )
		{
			// Set it to invalid, and erase it
			if( entry->second )
			{
				entry->second->m_Valid = false;
			}

			inManager.m_Objects.erase( entry );

			// We dont have to loop through the asset list and erase this object from each assets reference list because
			// they use weak_ptrs, and should be null after this call, if not, then m_Valid is false
			// When we loop through all assets to calculate the desired LOD, if a references object is invalid or null, we remove it then
		}
		else
		{
			Console::WriteLine( "[WARNING] AdaptiveAssetManager: Attempt to remove object from index, but it couldnt be found, Identifier: ", ObjectIdentifier );
		}
	}

	// Resource Change Event
	void AdaptiveAssetManagerResourceChangeEvent::Execute( AdaptiveAssetManager& inManager )
	{
		// Validate object info
		if( Object.m_Identifier == 0 )
		{
			return;
		}

		// Find this object, if it doesnt exist, then create it
		// If we create it, then we dont have to worry about removing old asset references
		std::shared_ptr< AdaptiveAssetManagerObjectInfo > objectPtr( nullptr );

		auto entry = inManager.m_Objects.find( Object.m_Identifier );
		if( entry != inManager.m_Objects.end() )
		{
			objectPtr = entry->second;
			if( !objectPtr )
			{
				objectPtr = std::make_shared< AdaptiveAssetManagerObjectInfo >();
			}
		}
		else
		{
			objectPtr = inManager.m_Objects[ Object.m_Identifier ] = std::make_shared< AdaptiveAssetManagerObjectInfo >();
		}
		
		objectPtr->operator=( Object );

		// Find each asset in secondary index and add/remove this object
		// If the asset doesnt exist, then we have to create it
		for( auto& id : RemovedAssets )
		{
			auto entry = inManager.m_Textures.find( id );
			if( entry != inManager.m_Textures.end() )
			{
				if( entry->second )
				{
					entry->second->m_Refs.erase( Object.m_Identifier );
				}
			}
			else
			{
				auto mdl_entry = inManager.m_Models.find( id );
				if( mdl_entry != inManager.m_Models.end() )
				{
					if( mdl_entry->second )
					{
						mdl_entry->second->m_Refs.erase( Object.m_Identifier );
					}
				}
			}

		}

		// Add new refs, if the asset doesnt exist, we have to create it like normal
		for( auto& tex : NewTextures )
		{
			if( tex.m_Asset.IsValid() )
			{
				inManager.CreateOrUpdateTexture( tex, objectPtr );
			}
		}

		for( auto& mdl : NewModels )
		{
			if( mdl.m_Asset.IsValid() )
			{
				inManager.CreateOrUpdateModel( mdl, objectPtr );
			}
		}

	}

	// Object Update Event
	void AdaptiveAssetManagerObjectUpdateEvent::Execute( AdaptiveAssetManager& inManager )
	{
		// Update info about a list of objects
		for( auto& info : Entries )
		{
			if( info.Identifier != 0 )
			{
				// Find this object
				auto entry = inManager.m_Objects.find( info.Identifier );
				if( entry != inManager.m_Objects.end() && entry->second )
				{
					entry->second->m_Position	= info.Position;
					entry->second->m_Radius		= info.Radius;
					entry->second->m_Dirty		= true;
				}
			}
		}
	}

	// Camera Update
	void AdaptiveAssetManagerCameraUpdateEvent::Execute( AdaptiveAssetManager& inManager )
	{
		// Update the camera structure
		if( CameraInfo.FOV > 0.f && CameraInfo.ScreenHeight > 0 )
		{
			inManager.m_CameraInfo.FOV				= CameraInfo.FOV;
			inManager.m_CameraInfo.Position			= CameraInfo.Position;
			inManager.m_CameraInfo.ScreenHeight		= CameraInfo.ScreenHeight;
			inManager.m_CameraInfo.bDirty			= true;
		}	
	}

	// World Reset Event
	void AdaptiveAssetManagerWorldResetEvent::Execute( AdaptiveAssetManager& inManager )
	{
		// We just remove all objects, then on asset iteration, they all hit ref count 0, and everything should reset
		for( auto& obj : inManager.m_Objects )
		{
			if( obj.second )
			{
				obj.second->m_Valid = false;
			}
		}

		inManager.m_Objects.clear();
		inManager.m_CameraInfo.bDirty = true;
	}

	/*=============================================================================================================
		Adaptive Asset Class
	=============================================================================================================*/


	/*=============================================================================================================
		Adaptive Asset Manager Class
	=============================================================================================================*/
	AdaptiveAssetManager::AdaptiveAssetManager()
		: m_AdaptiveQualityMult( 1.f )
	{
		// Create main update thread
		CustomThreadParameters threadParams;

		threadParams.AllowTasks				= false;
		threadParams.Identifier				= "aa_manager";
		threadParams.StartAutomatically		= true;
		threadParams.ThreadFunction			= std::bind( &AdaptiveAssetManager::ThreadMain, this, std::placeholders::_1 );

		m_Thread = ThreadManager::CreateThread( threadParams );
		HYPERION_VERIFY( m_Thread, "Failed to create thread to manager render assets!" );

		// Create texture worker threads
		uint32 textureWorkerCount = g_CVar_AdaptiveTextureWorkerCount.GetValue();

		for( uint32 i = 0; i < textureWorkerCount; i++ )
		{
			CustomThreadParameters workerParams;

			workerParams.AllowTasks				= false;
			workerParams.Identifier				= "aa_manager_texture_worker_";
			workerParams.StartAutomatically		= true;
			workerParams.ThreadFunction			= std::bind( &AdaptiveAssetManager::TextureWorkerThread, this, std::placeholders::_1 );

			workerParams.Identifier.append( std::to_string( i ) );

			auto newWorker = ThreadManager::CreateThread( workerParams );
			HYPERION_VERIFY( newWorker, "Failed to create texture worker thread!" );

			m_TextureWorkerThreads.push_back( newWorker );
		}

		// Create model worker threads
		uint32 modelWorkerCount = g_CVar_AdaptiveModelWorkerCount.GetValue();

		for( uint32 i = 0; i < modelWorkerCount; i++ )
		{
			// TODO
		}
	}


	AdaptiveAssetManager::~AdaptiveAssetManager()
	{
		// Shutdown worker threads
		for( auto& worker : m_TextureWorkerThreads )
		{
			if( worker )
			{
				worker->Stop();
				worker.reset();
			}
		}

		for( auto& worker : m_ModelWorkerThreads )
		{
			if( worker )
			{
				worker->Stop();
				worker.reset();
			}
		}

		// Shutdown main thread
		if( m_Thread )
		{
			m_Thread->Stop();
			m_Thread.reset();
		}
	}


	void AdaptiveAssetManager::ThreadMain( CustomThread& inThread )
	{
		// Perform init
		if( !Initialize() )
		{
			Console::WriteLine( "[ERROR] AdaptiveAssetManager: Failed to initialize.. shutting down!" );
			return;
		}

		std::chrono::high_resolution_clock::time_point lastTick = std::chrono::high_resolution_clock::now();

		// Thread loop
		while( inThread.IsRunning() )
		{
			// Update our view of the scene by running all events that are queued, and then perform a pass of asset allocation/deallocation
			UpdateSceneView();
			PerformTexturePass();
			PerformModelPass();

			// Mark the camera as clean before the next pass
			m_CameraInfo.bDirty = false;

			// Ensure were not running again too quickly if we dont need to
			// And check for shutdown before we sleep
			if( !inThread.IsRunning() ) break;
			std::this_thread::sleep_until( lastTick + std::chrono::milliseconds( (long)( 1000.f / 30.f ) ) );
			lastTick = std::chrono::high_resolution_clock::now();
		}

		// Perform shutdown
		Shutdown();
	}


	bool AdaptiveAssetManager::Initialize()
	{
		Console::WriteLine( "[INIT] AdaptiveAssetManager: Initializing..." );
		return true;
	}



	void AdaptiveAssetManager::Shutdown()
	{
		Console::WriteLine( "[SHUTDOWN] AdaptiveAssetManager: Shutting down..." );
	}


	void AdaptiveAssetManager::UpdateSceneView()
	{
		// Read events from queue until no more are returned
		auto nextEvent = m_EventQueue.PopValue();
		while( nextEvent.first && nextEvent.second )
		{
			// Process this event
			nextEvent.second->Execute( *this );
			nextEvent = m_EventQueue.PopValue();
		}

		// Now, purge any invalid objects from the list
		for( auto It = m_Objects.begin(); It != m_Objects.end(); )
		{
			if( !It->second || !It->second->m_Valid )
			{
				It = m_Objects.erase( It );
			}
			else if( m_CameraInfo.bDirty || It->second->m_Dirty )
			{
				// Also, calculate screen size of this object, if its changed since last iteration or the camera moved
				BoundingSphere bounds;
				bounds.Center = It->second->m_Position;
				bounds.Radius = It->second->m_Radius;

				// This gives us the diameter of the bounding sphere in pixels
				// We basically want to figure out, what resolution texture would we need to match 1:1 to the screen resolution
				// We can use the circumference, as the approximate width/height of the needed texture, but in some situations the mapping of a texture can greatly affect
				// the required resolution. So we should probably throw a mult in each object? Or think of a better approximation in the future
				auto diameterPx = Geometry::CalculateScreenSizeInPixels( m_CameraInfo.Position, m_CameraInfo.FOV, bounds, m_CameraInfo.ScreenHeight );

				It->second->m_ScreenSize	= diameterPx * Math::PIf;
				It->second->m_Dirty			= false;

				It++;
			}
		}
	}


	bool AdaptiveAssetManager::CreateOrUpdateTexture( const AdaptiveTextureInfo& inInfo, const std::shared_ptr< AdaptiveAssetManagerObjectInfo >& refObj )
	{
		// First, determine if the object and asset are both valid
		if( !inInfo.m_Asset.IsValid() )
		{
			Console::WriteLine( "[WARNING] AAManager: Attempt to create/update texture, but the specified asset was invalid" );
			return false;
		}

		if( !refObj )
		{
			Console::WriteLine( "[WARNING] AAManager: Attempt to create/update texture, but the specified object was invalid" );
			return false;
		}

		uint32 identifier = inInfo.m_Asset->GetAssetIdentifier();
		auto entry = m_Textures.find( identifier );

		if( entry == m_Textures.end() || !entry->second )
		{
			// This is a new texture, so we need to create its entry and setup the reference from the object
			auto& newTexture = m_Textures[ identifier ] = std::make_shared< AdaptiveTexture >();

			newTexture->m_Identifier = identifier;
			newTexture->m_Refs[ refObj->m_Identifier ] = std::weak_ptr( refObj );
			newTexture->m_Asset = inInfo.m_Asset;

			// TODO: Set more info about texture? Call init function? Move some of this into a constructor?
		}
		else
		{
			// This is an existing texture, so we just need to ensure the reference is set to this object
			entry->second->m_Refs[ refObj->m_Identifier ] = std::weak_ptr( refObj );
		}

		return true;
	}


	bool AdaptiveAssetManager::CreateOrUpdateModel( const AdaptiveModelInfo& inInfo, const std::shared_ptr< AdaptiveAssetManagerObjectInfo >& refObj )
	{
		// First, determine if object is valid, can cast the asset, and the object ref is valid
		if( !refObj )
		{
			Console::WriteLine( "[WARNING] AAManager: Attempt to create/update model, but the specified object was invalid" );
			return false;
		}

		if( !inInfo.m_Asset.IsValid() )
		{
			Console::WriteLine( "[WARNING] AAManager: Attempt to create/update model, but the specified asset was invalid" );
			return false;
		}

		uint32 identifier = inInfo.m_Asset->GetAssetIdentifier();
		auto entry = m_Models.find( identifier );

		if( entry == m_Models.end() || !entry->second )
		{
			// This is a new model.. create the instance!
			if( inInfo.m_Dynamic )
			{
				auto castedAsset = AssetCast< DynamicModelAsset >( inInfo.m_Asset );
				if( !castedAsset.IsValid() )
				{
					Console::WriteLine( "[WARNING] AAManager: Attempt to create/update dynamic model.. but the asset couldnt be casted to the correct type!" );
					return false;
				}

				auto& newModel = m_Models[ identifier ] = std::make_shared< AdaptiveDynamicModel >();
				newModel->m_Identifier = identifier;
				newModel->m_Refs[ refObj->m_Identifier ] = std::weak_ptr( refObj );
				newModel->m_Asset = inInfo.m_Asset;

				// TODO: Something else?
			}
			else
			{
				auto castedAsset = AssetCast< StaticModelAsset >( inInfo.m_Asset );
				if( !castedAsset.IsValid() )
				{
					Console::WriteLine( "[WARNING] AAManager: Attempt to create/update static model.. but the asset wasnt the correct type" );
					return false;
				}

				auto& newModel = m_Models[ identifier ] = std::make_shared< AdaptiveStaticModel >();
				newModel->m_Identifier = identifier;
				newModel->m_Refs[ refObj->m_Identifier ] = std::weak_ptr( refObj );
				newModel->m_Asset = inInfo.m_Asset;

				// TODO: Something else?
			}
		}
		else
		{
			// This is an existing instance, just ensure the object ref is set
			entry->second->m_Refs[ refObj->m_Identifier ] = std::weak_ptr( refObj );
		}

		return true;
	}


	void AdaptiveAssetManager::PerformTexturePass()
	{
		// Updated Procedure
		// 1. Update Scene
		// 2. Lock Mutex
		// 3. Re-sort load request list
		// 4. Loop through each texture...
		//	a. Select largest screen size
		//	b. Select 'perfect' LOD level for this texture
		//	c. Apply quality mult to get 'desired' LOD level for this texture
		//	d. Calculate LOD bias (used for drop priority sorting)
		//	e. Compare 'desired' to 'pending' LOD levels...
		//		- If so.. cancel pending LOD requests, set 'pending' to 'current' LOD level
		//		- Then, if 'deisred' > 'current' 
		//			* Create load request set to 'desired' LOD, insert to queue
		//		- If, 'desired' < 'current'
		//			* Unload any lecelas higher than the desired level instantly
		//			* Set both 'pending' and 'current' levels to the 'desired' level
		//		- If 'desired' == 'current'
		//			* Do nothing.. sice the pending load was already canceled
		// 5. Unlock Mutex

		// Aquire a lock on the mutex
		std::lock_guard< std::mutex > lock( m_TextureMutex );

		// Sort any texture load requests
		SortTextureLoadRequests();

		// Get multiplier values
		float globalMult		= g_CVar_TextureLODMult_Global.GetValue();
		float staticMult		= g_CVar_TextureLODMult_StaticModel.GetValue();
		float dynamicMult		= g_CVar_TextureLODMult_DynamicModel.GetValue();
		float levelMult			= g_CVar_TextureLODMult_Level.GetValue();
		float characterMult		= g_CVar_TextureLODMult_Character.GetValue();

		// Loop through all textures
		for( auto It = m_Textures.begin(); It != m_Textures.end(); )
		{
			// Check for null textures, remove entry
			if( !It->second || !It->second->m_Asset )
			{
				It = m_Textures.erase( It );
				// TODO: Tell render thread?

				continue;
			}

			// We also need to check for textures without any valid object refs
			// But to save cycles, we combine this with determining largest screen size
			auto& texture		= It->second;
			float screenSize	= -1.f;

			for( auto objIt = texture->m_Refs.begin(); objIt != texture->m_Refs.end(); )
			{
				auto obj = objIt->second.lock();

				if( obj && obj->m_Valid )
				{
					// Apply multipliers to object size when determining the size of the texture on screen
					float objMult = globalMult * m_AdaptiveQualityMult;

					switch( obj->m_Type )
					{
					case AdaptiveAssetObjectType::Static:
						objMult *= staticMult;
						break;
					case AdaptiveAssetObjectType::Dynamic:
						objMult *= dynamicMult;
						break;
					case AdaptiveAssetObjectType::Character:
						objMult *= characterMult;
						break;
					case AdaptiveAssetObjectType::Level:
						objMult *= levelMult;
						break;
					}

					screenSize = Math::Max( screenSize, obj->m_ScreenSize * objMult );
					objIt++;
				}
				else
				{
					objIt = texture->m_Refs.erase( objIt );
				}
			}

			// Check for no valid refs 
			if( screenSize < 0.f )
			{
				It = m_Textures.erase( It );
				// TODO: Tell render thread were erasing this texture?

				continue;
			}

			// Next, we need to calculate the 'target' LOD level for this texture
			// 'screenSize' is actually the minimum texture width/height needed to texture this object
			// TODO: Better solution?

			// We also need to calculate the LOD 'bias'
			// AKA: How close is this texture to requiring a larger LOD
			// So, the closer to the next highest LOD.. the higher the LOD bias
			uint8 targetLOD		= 0;
			float LODBias		= 0.5f;

			auto& LODList = texture->m_Asset->GetHeader().LODs;

			// First, ensure there is at least a single LOD available
			if( LODList.size() == 0 )
			{
				It = m_Textures.erase( It );
				// TODO: Tell render thread were eraseing this texture
				continue;
			}

			for( uint8 i = LODList.size() - 1; i >= 0; i-- )
			{
				auto& LOD = LODList.at( i );
				float LODSize = Math::Max( LOD.Width, LOD.Height );

				// Now, determine if this LOD is good enough for this texture
				if( LODSize > screenSize )
				{
					targetLOD = i;

					// Calculate our 'LOD bias'
					// If were the highest quality LOD already, we calculate this differently, or if there isnt a 'lower quality' LOD 
					if( i == 0 || LODList.size() <= ( i + 1 ) )
					{
						LODBias = Math::Max( 1.f, ( screenSize - LODSize ) / LODSize );
					}
					else
					{
						// We need to get the size of the next lower-quality LOD 
						auto& lowerLOD		= LODList.at( i + 1 );
						float lowerLODSize	= Math::Max( lowerLOD.Width, lowerLOD.Height );

						LODBias = Math::Clamp( ( screenSize - lowerLODSize ) / ( LODSize - lowerLODSize ), 0.f, 1.f );
					}

					break;
				}
			}

			texture->m_TargetLOD	= targetLOD;
			texture->m_LODBias		= LODBias;

			// Now, we have our LOD bias & target LOD
			// So lets compare the target LOD, to the currently pending LOD
			if( texture->m_TargetLOD != texture->m_PendingLOD )
			{
				// We need to update this textures loaded LODs
				// First, lets cancel any pending load requests
				// Then, set the pending level to the currently active level
				texture->m_PendingLOD = texture->m_ActiveLOD;

				auto loadReq = texture->m_LoadRequest.lock();
				if( loadReq )
				{
					loadReq->m_Valid = false;
				}

				texture->m_LoadRequest.reset();

				// Now, if the load request is already running, the worker thread will see the difference once the load completes
				// and it wont send the data over to the render thread. If a new level gets requested during this time, we will have to
				// generate a new load request, since we dont know that the old one is already running
				// But, if the load completes, and the pending level is different than the load requests, it can regenerate the request
				// This is a bit complicated, but we will get to it eventually

				if( texture->m_TargetLOD > texture->m_ActiveLOD )
				{
					// Generate load request
				}
				else if( texture->m_TargetLOD < texture->m_ActiveLOD )
				{
					// Unload un-needed LODs
				}


			}

			It++;
		}
	}


	void AdaptiveAssetManager::SortTextureLoadRequests()
	{

	}


	void AdaptiveAssetManager::PerformModelPass()
	{

	}


	void AdaptiveAssetManager::TextureWorkerThread( CustomThread& thisThread )
	{
		Console::WriteLine( "[DEBUG] TextureWorker: Initializing..." );
		// TODO

		Console::WriteLine( "[DEBUG] TextureWorker: Shutting down..." );
	}


	void AdaptiveAssetManager::ModelWorkerThread( CustomThread& thisThread )
	{
		Console::WriteLine( "[DEBUG] ModelWorker: Initializing..." );

		// TODO

		Console::WriteLine( "[DEBUG] ModelWorker: Shutting down..." );
	}





	void AdaptiveAssetManager::OnPrimitiveSpawned( AdaptiveAssetManagerSpawnEvent& inEvent )
	{
		// Validate the event, and insert into the queue
		if( inEvent.ObjectInfo.m_Identifier == 0 )
		{
			Console::WriteLine( "[WARNING] AdaptiveAssetManager: Attempt to fire 'OnSpawn' event with an invalid object" );
			return;
		}

		if( inEvent.Textures.size() == 0 && inEvent.Models.size() == 0 )
		{
			// If there are no assets, then dont even fire event..
			// If an asset does get assigned to this primitive in the future, the resource update event will create this primitive at that time
			return;
		}

		m_EventQueue.Push(
			std::move( std::unique_ptr< AdaptiveAssetManagerSpawnEvent >( &inEvent ) )
		);
	}

	void AdaptiveAssetManager::OnPrimitiveDeSpawned( AdaptiveAssetManagerDespawnEvent& inEvent )
	{
		if( inEvent.ObjectIdentifier == 0 )
		{
			Console::WriteLine( "[WARNING] AdaptiveAssetManager: Attempt to fire 'OnDespawn' with an invalid object" );
			return;
		}

		m_EventQueue.Push(
			std::move( std::unique_ptr< AdaptiveAssetManagerDespawnEvent >( &inEvent ) )
		);
	}

	void AdaptiveAssetManager::OnPrimitiveChangedResources( AdaptiveAssetManagerResourceChangeEvent& inEvent )
	{
		// Validate the event structure
		if( inEvent.Object.m_Identifier == 0 )
		{
			Console::WriteLine( "[WARNING] AdaptiveAssetManager: Attempt to fire 'OnResourceChange' with an invalid object" );
			return;
		}

		if( inEvent.NewTextures.size() == 0 && inEvent.NewModels.size() == 0 && inEvent.RemovedAssets.size() == 0 )
		{
			Console::WriteLine( "[WARNING] AdaptiveAssetManager: Attempt to fire 'OnResourceChange' with no asset(s) added or removed" );
			return;
		}


		m_EventQueue.Push(
			std::move( std::unique_ptr< AdaptiveAssetManagerResourceChangeEvent >( &inEvent ) )
		);
	}

	void AdaptiveAssetManager::OnPrimitiveUpdate( AdaptiveAssetManagerObjectUpdateEvent& inEvent )
	{
		for( auto& e : inEvent.Entries )
		{
			if( e.Identifier == 0 )
			{
				// Still allow function to continue
				Console::WriteLine( "[WARNING] AdaptiveAssetManager: Attempt to fire 'OnPrimitiveUpdate' with invalid primitive(s)" );
			}
		}

		m_EventQueue.Push(
			std::move( std::unique_ptr< AdaptiveAssetManagerObjectUpdateEvent >( &inEvent ) )
		);
	}

	void AdaptiveAssetManager::OnCameraUpdate( AdaptiveAssetManagerCameraUpdateEvent& inEvent )
	{
		if( inEvent.CameraInfo.FOV == 0.f ||
			inEvent.CameraInfo.ScreenHeight == 0 )
		{
			Console::WriteLine( "[WARNING] AdaptiveAssetManager: Attempt to fire 'OnCameraUpdate' with invalid camera info!" );
			return;
		}

		m_EventQueue.Push(
			std::move( std::unique_ptr< AdaptiveAssetManagerCameraUpdateEvent >( &inEvent ) )
		);
	}

	void AdaptiveAssetManager::OnWorldReset( AdaptiveAssetManagerWorldResetEvent& inEvent )
	{
		m_EventQueue.Push(
			std::move( std::unique_ptr< AdaptiveAssetManagerWorldResetEvent >( &inEvent ) )
		);
	}



}
