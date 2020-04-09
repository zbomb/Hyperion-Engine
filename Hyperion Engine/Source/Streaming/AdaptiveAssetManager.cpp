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
#include "Hyperion/Core/RenderManager.h"
#include "Hyperion/File/UnifiedFileSystem.h"



namespace Hyperion
{
	/*=============================================================================================================
		Console Variables
	=============================================================================================================*/
	ConsoleVar< uint32 > g_CVar_AdaptiveTexturePoolSize(
		"r_adaptive_texture_pool_size", "The amount of memory available to the adaptive texture pool in MB",
		2048, 1024, 64 * 1024 ); // 64GB Max?

	ConsoleVar< uint32 > g_CVar_AdaptiveModelPoolSize(
		"r_adaptive_model_pool_size", "The amount of memory available to the adaptive model pool in MB",
		1024, 256, 32 * 1024 ); // 32GB Max?

	// TODO: Cant really hook 'onChanged' since we dont have access here to the aa manager instance..
	// So, changing this after aa manager inits, wont actually do anything.. startup only!
	ConsoleVar< uint32 > g_CVar_AdaptiveTextureWorkerCount(
		"r_adaptive_texture_workers", "The number of worker threads to use when loading in textures",
		2, 1, 8 );

	// Same here!
	ConsoleVar< uint32 > g_CVar_AdaptiveModelWorkerCount(
		"r_adaptive_model_workers", "The number of worker threads to use when loading in models",
		1, 1, 8 );

	ConsoleVar< float > g_CVar_TextureLODMult_Global(
		"r_texture_lod_mult_global", "Multiplier applied when selecting an LOD for a texture",
		1.f, 0.01f, 10.f );

	ConsoleVar< float > g_CVar_TextureLODMult_StaticModel(
		"r_texture_lod_mult_static_model", "Multiplier applied when selecting an LOD for a texture applied to a static model",
		1.f, 0.01f, 10.f );

	ConsoleVar< float > g_CVar_TextureLODMult_DynamicModel(
		"r_texture_lod_mult_dynamic_model", "Multiplier applied when selecting an LOD for a texture applied to a dynamic model",
		1.f, 0.01f, 10.f );

	ConsoleVar< float > g_CVar_TextureLODMult_Level(
		"r_texture_lod_mult_level", "Multiplier applied when selecting an LOD for a texture applied to the level",
		1.f, 0.01f, 10.f );

	ConsoleVar< float > g_CVar_TextureLODMult_Character(
		"r_texture_lod_mult_character", "Multiplier applied when selecting an LOD for a texture applied to a character",
		1.f, 0.01f, 10.f );

	ConsoleVar< uint32 > g_CVar_TextureMaxResidentMemory(
		"r_texture_max_resident_memory", "Maximum amount of resident mip memory per texture? In kB",
		64, 0, 10000000 );


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
					entry->second->RemoveObjectReference( Object.m_Identifier );
				}
			}
			else
			{
				auto mdl_entry = inManager.m_Models.find( id );
				if( mdl_entry != inManager.m_Models.end() )
				{
					if( mdl_entry->second )
					{
						mdl_entry->second->RemoveObjectReference( Object.m_Identifier );
					}
				}
			}

		}

		// Add new refs, if the asset doesnt exist, we have to create it like normal
		for( auto& tex : NewTextures )
		{
			if( tex.m_Asset )
			{
				inManager.CreateOrUpdateTexture( tex, objectPtr );
			}
		}

		for( auto& mdl : NewModels )
		{
			if( mdl.m_Asset )
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
		Sort Functions
	=============================================================================================================*/

	bool AdaptiveAssetManagerTextureSort::operator()( const std::shared_ptr< AdaptiveTexture >& lhs, const std::shared_ptr< AdaptiveTexture >& rhs )
	{
		// Sort by priority, low to high
		return lhs->GetPriority() < rhs->GetPriority();
	}


	bool AdaptiveAssetManagerTextureSort::operator()( const std::weak_ptr< AdaptiveTexture >& lhs, const std::weak_ptr< AdaptiveTexture >& rhs )
	{
		auto l = lhs.lock();
		auto r = rhs.lock();

		if( !l ) { return true; }
		else if( !r ) { return false; }

		return l->GetPriority() < r->GetPriority();
	}


	bool AdaptiveAssetManagerTextureLoadRequestSort::operator()( const std::shared_ptr< AdaptiveTextureLoadRequest >& lhs, const std::shared_ptr< AdaptiveTextureLoadRequest >& rhs )
	{
		// Sort by priority, high to low
		return lhs->GetPriority() > rhs->GetPriority();
	}


	bool AdaptiveAssetManagerTextureUnloadRequestSort::operator()( const std::shared_ptr< AdaptiveTextureUnloadRequest >& lhs, const std::shared_ptr< AdaptiveTextureUnloadRequest >& rhs )
	{
		// Sort by memory, high to low
		return lhs->GetMemory() > rhs->GetMemory();
	}


	/*=============================================================================================================
		Adaptive Asset Manager Class
	=============================================================================================================*/

	/*-------------------------------------------------------------------------------
		AdaptiveAssetManager::AdaptiveAssetManager
		- Constructor, creates threads to run this system
	-------------------------------------------------------------------------------*/
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
			workerParams.ThreadFunction			= std::bind( &AdaptiveAssetManager::TextureWorker_Main, this, std::placeholders::_1 );

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


	/*-------------------------------------------------------------------------------
		AdaptiveAssetManager::~AdaptiveAssetManager
		- Destructor, shuts down system and destroys all threads
	-------------------------------------------------------------------------------*/
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


	/*-------------------------------------------------------------------------------
		AdaptiveAssetManager::ThreadMain
		- Main thread function for the AA system
	-------------------------------------------------------------------------------*/
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


	/*-------------------------------------------------------------------------------
		AdaptiveAssetManager::Initialize
		- Main Thread
		- Initialization function, called before anything else
	-------------------------------------------------------------------------------*/
	bool AdaptiveAssetManager::Initialize()
	{
		Console::WriteLine( "[INIT] AdaptiveAssetManager: Initializing..." );
		return true;
	}


	/*-------------------------------------------------------------------------------
		AdaptiveAssetManager::Shutdown
		- Main Thread
		- Shutdown function, called right before thread destruction
	-------------------------------------------------------------------------------*/
	void AdaptiveAssetManager::Shutdown()
	{
		Console::WriteLine( "[SHUTDOWN] AdaptiveAssetManager: Shutting down..." );
	}


	/*-------------------------------------------------------------------------------
		AdaptiveAssetManager::UpdateSceneView
		- Main Thread
		- Processes all events pending in the queue from the game thread
	-------------------------------------------------------------------------------*/
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

				// In the future, there might be a better way to accomplish this, were currently using the diameter of the boudning sphere to approximate
				// the size of a texture needed to cover this object, at a 1:1 ratio between texture pixels and screen pixels
				// OPTIMIZE OPTIMIZE OPTIMIZE
				auto diameterPx = Geometry::CalculateScreenSizeInPixels( m_CameraInfo.Position, m_CameraInfo.FOV, bounds, m_CameraInfo.ScreenHeight );

				It->second->m_ScreenSize	= diameterPx * Math::PIf;
				It->second->m_Dirty			= false;

				It++;
			}
		}
	}


	/*---------------------------------------------------------------------------------------
		AdaptiveAssetManager::CreateOrUpdateTexture
		- Main Thread
		- Called to create a new texture, or update the state if the texture already
		  exists, mainly called from the spawn, resource changed, and update events
		- Returns true if the texture was able to be created/updated, false if not
	---------------------------------------------------------------------------------------*/
	bool AdaptiveAssetManager::CreateOrUpdateTexture( const AdaptiveTextureInfo& inInfo, const std::shared_ptr< AdaptiveAssetManagerObjectInfo >& refObj )
	{
		// Validate parameters
		if( !inInfo.m_Asset || !refObj )
		{
			Console::WriteLine( "[ERROR] AdaptiveAssetManager: Attempt to create/update texture, but the specified asset/object was invalid!" );
			return false;
		}

		// Check if this texture already exists
		auto identifier		= inInfo.m_Asset->GetIdentifier();
		auto entry			= m_Textures.find( identifier );

		if( entry == m_Textures.end() || !entry->second )
		{
			// This is a new texture (or existing was null), so we need to create it
			auto newTexture = std::make_shared< AdaptiveTexture >( inInfo.m_Asset );
			newTexture->AddObjectReference( refObj );

			m_Textures.emplace( identifier, newTexture );
			m_SortedTextures.push_back( std::weak_ptr( newTexture ) );
		}
		else
		{
			// This is an existing texture, so add this object to the ref list
			entry->second->AddObjectReference( refObj );
		}

		return true;
	}


	/*---------------------------------------------------------------------------------------
		AdaptiveAssetManager::CreateOrUpdateModel
		- Main Thread
		- Called to create a new model, or if it already exists, it will update the
		  model, mainly called from spawn, resource changed, and update events
		- Returns true if the model was able to be created/update, false if not
	---------------------------------------------------------------------------------------*/
	bool AdaptiveAssetManager::CreateOrUpdateModel( const AdaptiveModelInfo& inInfo, const std::shared_ptr< AdaptiveAssetManagerObjectInfo >& refObj )
	{
		/*
		// Validate the parameters
		if( !inInfo.m_Asset || !refObj )
		{
			Console::WriteLine( "[ERROR] AdaptiveAssetManager: Attempt to create/update model, but the specified asset/object was invalid!" );
			return false;
		}

		uint32 identifier	= inInfo.m_Asset->GetIdentifier();
		auto entry			= m_Models.find( identifier );

		if( entry == m_Models.end() || !entry->second )
		{
			// This is a new model (or the existing one is null), so were going to create a model isntance
			std::shared_ptr< AdaptiveModel > modelPtr;

			if( inInfo.m_Dynamic )
			{
				auto castedAsset = AssetCast< DynamicModelAsset >( inInfo.m_Asset );
				if( !castedAsset.IsValid() )
				{
					Console::WriteLine( "[ERROR] AdaptiveAssetManager: Attempt to create/update dynamic model instance.. but the asset was invalid/wrong type!" );
					return false;
				}

				modelPtr = std::make_shared< AdaptiveDynamicModel >( castedAsset );
			}
			else
			{
				auto castedAsset = AssetCast< StaticModelAsset >( inInfo.m_Asset );
				if( !castedAsset.IsValid() )
				{
					Console::WriteLine( "[ERROR] AdaptiveAssetManager: Attempt to create/update static model instance.. but the asset was invalid/wrong type!" );
					return false;
				}

				modelPtr = std::make_shared< AdaptiveStaticModel >( castedAsset );
			}

			modelPtr->AddObjectReference( refObj );
			m_Models.emplace( identifier, modelPtr );
			m_SortedModels.push_back( std::weak_ptr( modelPtr ) );

		}
		else
		{
			entry->second->AddObjectReference( refObj );
		}

		return true;
		*/

		return false;
	}


	/*---------------------------------------------------------------------------------------
		AdaptiveAssetManager::PerformTexturePass
		- Main Thread
		- Called to update the textures during the main tick phase
	---------------------------------------------------------------------------------------*/
	void AdaptiveAssetManager::PerformTexturePass()
	{
		// Aquire a lock on the mutex
		std::lock_guard< std::mutex > lock( m_TextureMutex );

		// Get multiplier values
		float globalMult		= g_CVar_TextureLODMult_Global.GetValue();
		float staticMult		= g_CVar_TextureLODMult_StaticModel.GetValue();
		float dynamicMult		= g_CVar_TextureLODMult_DynamicModel.GetValue();
		float levelMult			= g_CVar_TextureLODMult_Level.GetValue();
		float characterMult		= g_CVar_TextureLODMult_Character.GetValue();

		// Loop through all textures
		for( auto It = m_Textures.begin(); It != m_Textures.end(); )
		{
			// Check for locked, or invalid textures
			if( It->second && It->second->IsLocked() )
			{
				continue;
			}
			else if( !It->second || !It->second->GetAsset() )
			{
				It = m_Textures.erase( It );
				FinishDestroyTexture( *( It->second ) );
				continue;
			}

			// Next, we need to iterate all refs for this texture
			// Were going to calculate screensize, and valid ref count 
			auto& texture			= It->second;

			if( !texture->Update( globalMult * m_AdaptiveQualityMult, characterMult, dynamicMult, levelMult, staticMult, (uint32)m_Objects.size() ) ||
				texture->GetTotalScreenSize() <= 0.f )
			{
				// Ref-count is zero, or not visible at all, so were going to erase this texture
				It = m_Textures.erase( It );
				FinishDestroyTexture( *texture );
				continue;
			}

			// Next, we need to calculate the desired LOD for this texture, this will also cancel any invalid loads/unloads if the target LOD changes
			if( !texture->UpdateTargetLOD() )
			{
				It = m_Textures.erase( It );
				FinishDestroyTexture( *texture );
				continue;
			}

			// DONT FORGET: HAVE TO CAUCLATE PRIORITY AT SOME POINT

			// Check if we can generate a load or unload request
			auto loadReq = texture->GenerateLoadRequest( texture );
			if( loadReq )
			{
				m_TextureLoadQueue.push_back( loadReq ); // TODO: Sorted insert
			}
			else
			{
				auto dropReq = texture->GenerateDropRequest( texture );
				if( dropReq )
				{
					m_TextureUnloadQueue.push_back( dropReq ); // TODO: Sorted insert
				}
			}

			texture->UpdatePriority();
			It++;
		}

		// Sort any texture load requests
		SortTextureLoadRequests();
		SortTextureUnloadRequests();
		SortTextures();
	}


	/*---------------------------------------------------------------------------------------
		AdaptiveAssetManager::SortTextures
		- Main Thread
		- Called to sort all textures by priority, from low to high
		- Also, performs a pre-pass where invalid textures are removed from the list
	---------------------------------------------------------------------------------------*/
	void AdaptiveAssetManager::SortTextures()
	{
		// First, we need to ensure there is no null/invalid entries in this list, the worker thread ignores these entries, but we want
		// to ensure we are getting rid of them as well.. so were going to do that before sorting
		for( auto It = m_SortedTextures.begin(); It != m_SortedTextures.end(); )
		{
			auto t = It->lock();
			if( !t )
			{
				It = m_SortedTextures.erase( It );
			}
			else
			{
				It++;
			}
		}

		std::sort( m_SortedTextures.begin(), m_SortedTextures.end(), AdaptiveAssetManagerTextureSort() );
	}


	/*---------------------------------------------------------------------------------------
		AdaptiveAssetManager::SortTextureLoadRequests
		- Main Thread
		- Called to sort load requests by priority, high to low
		- Also, performs pre-pass where invalid requests are removed, and valid ones have
		  their priority recalculated before performing the sort
	---------------------------------------------------------------------------------------*/
	void AdaptiveAssetManager::SortTextureLoadRequests()
	{
		// We need to update texture load request priority, and sort the list, high to low
		for( auto It = m_TextureLoadQueue.begin(); It != m_TextureLoadQueue.end(); )
		{
			auto req = *It;
			if( !req || !req->IsValid() )
			{
				It = m_TextureLoadQueue.erase( It );
			}
			else
			{
				req->UpdatePriority();
				It++;
			}
		}

		std::sort( m_TextureLoadQueue.begin(), m_TextureLoadQueue.end(), AdaptiveAssetManagerTextureLoadRequestSort() );
	}


	/*---------------------------------------------------------------------------------------
		AdaptiveAssetManager::SortTextureUnloadRequests
		- Main Thread
		- Called to sort unload requests by their memory savings, from high to low
		- Also performs pre-pass to remove invalid requests
		- We dont need to recalculate memory savings each iteration, because this value
		  will only change when the texture is updated, in which case, the request gets
		  invalidated anyway, and removed
	---------------------------------------------------------------------------------------*/
	void AdaptiveAssetManager::SortTextureUnloadRequests()
	{
		// We only sort pure unloads by memory, from high to low
		for( auto It = m_TextureUnloadQueue.begin(); It != m_TextureUnloadQueue.end(); )
		{
			auto req = *It;
			if( !req || !req->IsValid() )
			{
				It = m_TextureUnloadQueue.erase( It );
			}
			else
			{
				It++;
			}
		}

		std::sort( m_TextureUnloadQueue.begin(), m_TextureUnloadQueue.end(), AdaptiveAssetManagerTextureUnloadRequestSort() );
	}


	/*---------------------------------------------------------------------------------------
		AdaptiveAssetManager::FinishDestroyTexture
		- Main Thread
		- Called to finalize the destruction of a texture instance
		- Still needs to be removed from the list(s) before/after this call
	---------------------------------------------------------------------------------------*/
	void AdaptiveAssetManager::FinishDestroyTexture( AdaptiveTexture& inTexture )
	{
		// Properly remove texture from this system
		inTexture.CancelPendingRequest();
		RenderManager::GetRenderer().RemoveTextureAsset( inTexture.GetIdentifier() );

		m_MemoryUsage -= inTexture.GetActiveMemory();
	}


	/*---------------------------------------------------------------------------------------
		AdaptiveAssetManager::TextureWorker_Main
		- Texture Worker Thread
		- Thread function for the texture worker, performs the selection, loading, and 
		  unloading of texture LODs.
		- We want to try and break this up into more functions to clean up the code
	---------------------------------------------------------------------------------------*/
	void AdaptiveAssetManager::TextureWorker_Main( CustomThread& thisThread )
	{
		Console::WriteLine( "[DEBUG] TextureWorker: Initializing..." );
		std::chrono::time_point< std::chrono::high_resolution_clock > lastTick = std::chrono::high_resolution_clock::now();

		while( thisThread.IsRunning() )
		{
			std::shared_ptr< AdaptiveTextureLoadRequest > loadRequest;
			std::vector< std::shared_ptr< AdaptiveTexture > > forceDropList;
			std::vector< std::shared_ptr< AdaptiveTextureUnloadRequest > > pureDropList;

			/*
				Aquire lock so we can access the request/texture lists
			*/
			{
				std::lock_guard< std::mutex > lock( m_TextureMutex );

				/*
					First, call function to select the load/unload requests for us to process this iteration
				*/
				TextureWorker_SelectRequests( loadRequest, forceDropList, pureDropList );

				/*
					Next, we need to lock all textures were going to be working with before releasing the mutex
					Also, we will update the memory usage statistic
				*/
				if( loadRequest && loadRequest->IsValid() )
				{
					loadRequest->GetTarget()->SetLock( true );
					m_MemoryUsage += loadRequest->GetMemory();
				}

				for( auto& drop : pureDropList )
				{
					if( drop && drop->IsValid() )
					{
						drop->GetTarget()->SetLock( true );
						m_MemoryUsage -= drop->GetMemory();
					}
				}

				for( auto& drop : forceDropList )
				{
					if( drop )
					{
						drop->SetLock( true );
						m_MemoryUsage -= drop->GetTopLevelMemoryUsage();
					}
				}
			} // Mutex Release

			/*
				Now we want to process the unloads first. Start with the pure unloads, then the forced unloads
				Also, we want to keep track of the failed loads/unloads, so we can correct the memory usage later
			*/
			auto& r = RenderManager::GetRenderer();
			uint32 failedUnloadMemory	= 0;
			uint32 failedLoadMemory		= 0;

			for( auto& drop : pureDropList )
			{
				auto target		= drop->GetTarget();
				auto level		= drop->GetLevel();

				HYPERION_VERIFY( target && target->GetAsset(), "Pure drop target became invalid?" );

				if( r.LowerTextureAssetLOD( target->GetAsset(), level ) )
				{
					// If the drop was successful, update our texture instance to reflect this, and unlock it
					// so other threads are able to access it right away
					target->PerformDrop( level );
					target->SetLock( false );
				}
				else
				{
					// Drop Failed! We need to add the memory back in, and reset the texture instance
					target->PerformDrop( target->GetActiveLevel() );
					target->SetLock( false );

					failedUnloadMemory += drop->GetMemory();
				}
			}

			for( auto& tex : forceDropList )
			{
				auto level = tex->GetActiveLevel() - 1;
				HYPERION_VERIFY( tex && tex->GetActiveLevel() > tex->GetMinimumLevel() && tex->GetAsset(), "Force drop target became invalid?" );

				if( r.LowerTextureAssetLOD( tex->GetAsset(), level ) )
				{
					// Force drop was successful, so update texture to reflect this, and unlock it
					tex->PerformDrop( level );
					tex->SetLock( false );
				}
				else
				{
					// Force drop failed! So we are going to reset the texture and track the memory we need to add back
					tex->PerformDrop( tex->GetActiveLevel() );
					tex->SetLock( false );

					failedUnloadMemory += tex->GetTopLevelMemoryUsage();
				}
			}

			/*
				Now that the loads and unload have been processed, were going to process the load request
			*/
			if( loadRequest )
			{
				if( TextureWorker_PerformLoad( loadRequest ) )
				{
					loadRequest->GetTarget()->PerformIncrease( loadRequest->GetLevel() );
				}
				else
				{
					failedLoadMemory += loadRequest->GetMemory();
					loadRequest->GetTarget()->CancelPendingRequest( true );
				}

				loadRequest->GetTarget()->SetLock( false );
			}

			if( failedUnloadMemory > 0 || failedUnloadMemory > 0 )
			{
				std::lock_guard< std::mutex > lock( m_TextureMutex );

				// If we failed to load some stuff, aquire a lock and add the memory back in
				m_MemoryUsage += failedLoadMemory;
				m_MemoryUsage -= failedUnloadMemory;
			}

			// TODO: Anything else?

			// Ensure this worker doesnt run more than 60Hz, it shouldnnt unless there are no pending laods/unloads
			std::this_thread::sleep_until( lastTick + std::chrono::milliseconds( 16 ) );
		}

		Console::WriteLine( "[DEBUG] TextureWorker: Shutting down..." );
	}


	void AdaptiveAssetManager::TextureWorker_SelectRequests( std::shared_ptr< AdaptiveTextureLoadRequest >& outLoad,
															 std::vector< std::shared_ptr< AdaptiveTexture > >& outForceDrops,
															 std::vector< std::shared_ptr< AdaptiveTextureUnloadRequest > >& outPureDrops )
	{
		// Get the next pending load request (if there is one)
		auto loadIter		= TextureWorker_PopNextLoadRequest();
		auto memPoolSize	= g_CVar_AdaptiveTexturePoolSize.GetValue();
		uint32 neededMem	= 0;
		uint32 savedMem		= 0;
		uint32 dropCount	= 0;

		static const uint32 minPureDrops	= 2;
		static const uint32 maxDropsTotal	= 8;

		if( loadIter != m_TextureLoadQueue.end() ) // Assume this is a valid pointer, otherwise the function would return 'end()'
		{
			neededMem = ( *loadIter )->GetMemory() - ( memPoolSize - m_MemoryUsage );
		}

		// Pull some pure drops to save some memory
		for( auto It = m_TextureUnloadQueue.begin(); It != m_TextureUnloadQueue.end(); It++ )
		{
			auto trgt = *It;
			if( trgt && trgt->IsValid() )
			{
				outPureDrops.push_back( trgt );
				savedMem += trgt->GetMemory();
				dropCount++;

				if( dropCount >= maxDropsTotal ||
					( outPureDrops.size() >= minPureDrops && savedMem >= neededMem ) )
				{
					break;
				}
			}
		}

		// Check if we have enough memory to perform this load, without having to perform force drops
		if( savedMem >= neededMem )
		{
			if( loadIter != m_TextureLoadQueue.end() )
			{
				outLoad = *loadIter;
				m_TextureLoadQueue.erase( loadIter );
			}

			return;
		}

		// Store info about the load request, incase we end up modifying it
		auto originalLevel		= ( *loadIter )->GetLevel();
		auto originalMemory		= ( *loadIter )->GetMemory();
		auto originalPriority	= ( *loadIter )->GetPriority();

		// Now, check if we hit the maximum number of drops for this selection cycle
		if( dropCount >= maxDropsTotal )
		{
			// We cant perform anymore drops, so lets lower the quality level of the load, until were under budget
			// If we cant get under budget in this fasion, we will just perform the pure drops

			while( TextureWorker_DropLevelFromLoad( *loadIter ) )
			{
				neededMem = ( *loadIter )->GetMemory() - ( memPoolSize - m_MemoryUsage );
				if( savedMem >= neededMem )
				{
					// Managed to get under budget, so output the load as well
					outLoad = *loadIter;
					m_TextureLoadQueue.erase( loadIter );
					return;
				}
			}

			// We didnt get under budget, so we need to reset the load before returning
			( *loadIter )->Reset( originalLevel, originalMemory, originalPriority );
			return;
		}

		// Perform 'force' drops to try and get under budget
		while( neededMem > savedMem && !TextureWorker_GenerateForceDropList( maxDropsTotal - dropCount, ( *loadIter )->GetPriority(), neededMem - savedMem, outPureDrops, outForceDrops ) )
		{
			// At this point, we couldnt generate a list of force drops to get under budget, so were going to drop a level from the load request and try again
			if( !TextureWorker_DropLevelFromLoad( *loadIter ) )
			{
				// Were unable to drop the load any further, or come up with a list of unloads to make enough memory
				// So, were still going to allow the force drops to go ahead, and the pure drops
				( *loadIter )->Reset( originalLevel, originalMemory, originalPriority );
				return;
			}

			neededMem = ( *loadIter )->GetMemory() - ( memPoolSize - m_MemoryUsage );
		}

		// We found a list of force drops (and/or lowered the request) to make enough space
		outLoad = *loadIter;
		m_TextureLoadQueue.erase( loadIter );
	}


	bool AdaptiveAssetManager::TextureWorker_PerformLoad( const std::shared_ptr< AdaptiveTextureLoadRequest >& loadRequest )
	{
		// Validate the load request
		if( !loadRequest || !loadRequest->IsValid() )
			return false;

		auto& targetAsset	= loadRequest->GetTarget()->GetAsset();
		auto targetHeader	= targetAsset->GetHeader();
		auto targetPath		= targetAsset->GetPath();

		auto f = UnifiedFileSystem::OpenFile( targetPath );
		if( !f || !f->IsValid() )
		{
			Console::WriteLine( "[ERROR] AdaptiveAssetManager: Failed to load LOD level(s) for texture '", targetPath, "' because the file couldnt be opened" );
		}
		else
		{
			DataReader reader( f );

			auto fileSize			= reader.Size();
			uint8 globalLODCount	= (uint8) targetHeader.LODs.size();
			uint8 localLODCount		= (uint8) targetHeader.LODs.size() - loadRequest->GetLevel();

			uint32 dataOffset	= targetHeader.LODs.at( loadRequest->GetLevel() ).FileOffset;

			// For size, we can just take the offset of the last LOD, and its size and calculate the total size
			uint32 lowestOffset		= targetHeader.LODs.back().FileOffset;
			uint32 lowestSize		= targetHeader.LODs.back().LODSize;

			uint32 dataSize = ( lowestOffset + lowestSize ) - dataOffset;
			HYPERION_VERIFY( lowestOffset + lowestSize <= fileSize, "Texture data out of file bounds?" );

			// Now, read all of the data in a single read call.
			// TODO: Interuptable prioritized loads?
			std::vector< byte > allData;
			reader.SeekOffset( dataOffset );

			if( reader.ReadBytes( allData, dataSize ) != DataReader::ReadResult::Success )
			{
				Console::WriteLine( "[ERROR] AdaptiveAssetManager: Failed to read in texture \"", targetPath, "\"" );
			}
			else
			{
				// Read success! Now we need to split the data into seperate LOD slices
				std::vector< std::vector< byte > > LODData;

				for( uint8 i = loadRequest->GetLevel(); i < targetHeader.LODs.size(); i++ )
				{
					// Get begin and end iterators to the target data set for this LOD
					auto& lodInfo = targetHeader.LODs.at( i );
					uint32 localOffset = lodInfo.FileOffset - dataOffset;

					auto beginIt = allData.begin();
					auto endIt = allData.begin();

					std::advance( beginIt, localOffset );
					std::advance( endIt, localOffset + lodInfo.LODSize );

					LODData.emplace_back( beginIt, endIt );
				}

				// Clear original data set
				std::vector< byte >().swap( allData );

				// Now, make the renderer call to create this new texture
				if( !RenderManager::GetRenderer().IncreaseTextureAssetLOD( targetAsset, loadRequest->GetLevel(), LODData ) )
				{
					Console::WriteLine( "[ERROR] AdaptiveAssetManager: Failed to increase LOD level for texture \"", targetPath, "\", because the render thread failed to update the texture!" );
				}
				else
				{
					return true;
				}
			}
		}

		return false;
	}


	/*----------------------------------------------------------------------------------------------
		AdaptiveAssetManager::TextureWorker_PopNextLoadRequest
		* Texture worker thread helper function
		* Finds the next valid load request to process
		* Returns an 'end()' iterator of the texture load queue if none were found
	----------------------------------------------------------------------------------------------*/
	std::deque< std::shared_ptr< AdaptiveTextureLoadRequest > >::iterator AdaptiveAssetManager::TextureWorker_PopNextLoadRequest()
	{
		return m_TextureLoadQueue.begin();
	}

	/*----------------------------------------------------------------------------------------------
		AdaptiveAssetManager::TextureWorker_DropLevelFromLoad
		* Texture worker thread helper function
		* Takes the load request, and drops an LOD from the desired level, recalculates
		  memory, and priority
		* Returns false if unable to drop down another level
	----------------------------------------------------------------------------------------------*/
	bool AdaptiveAssetManager::TextureWorker_DropLevelFromLoad( std::shared_ptr< AdaptiveTextureLoadRequest >& inReq )
	{
		return false;
	}

	/*----------------------------------------------------------------------------------------------
		AdaptiveAssetManager::TextureWorker_GenerateForceDropList
		* Texture worker thread helper function
		* Comes up with a list of force drops, within the set parameters to make the desired 
		  amount of space in the texture pool
	----------------------------------------------------------------------------------------------*/
	bool AdaptiveAssetManager::TextureWorker_GenerateForceDropList( uint32 dropCount, float loadPriority, uint32 neededMemory, 
																	const std::vector< std::shared_ptr< AdaptiveTextureUnloadRequest > >& selectedDrops, std::vector< std::shared_ptr< AdaptiveTexture > >& outList )
	{
		// TODO: Even when we fail, we want to output a list of the 'best' drops possible, to get us as close as possible with as low memory as possible
		// because, even if we failed to reach the memory target, we might still perform them, to avoid a deadlock
		return false;
	}




	/*---------------------------------------------------------------------------------------
		AdaptiveAssetManager::PerformModelPass
		- Main Thread
		- Called to sort model lists, generate requests, and update model instances
		- TODO still
	---------------------------------------------------------------------------------------*/
	void AdaptiveAssetManager::PerformModelPass()
	{

	}



	void AdaptiveAssetManager::ModelWorker_Main( CustomThread& thisThread )
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
			std::move( std::make_unique< AdaptiveAssetManagerSpawnEvent >( inEvent ) )
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
			std::move( std::make_unique< AdaptiveAssetManagerDespawnEvent >( inEvent ) )
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
			std::move( std::make_unique< AdaptiveAssetManagerResourceChangeEvent >( inEvent ) )
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
			std::move( std::make_unique< AdaptiveAssetManagerObjectUpdateEvent >( inEvent ) )
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
			std::move( std::make_unique< AdaptiveAssetManagerCameraUpdateEvent >( inEvent ) )
		);
	}

	void AdaptiveAssetManager::OnWorldReset( AdaptiveAssetManagerWorldResetEvent& inEvent )
	{
		m_EventQueue.Push(
			std::move( std::make_unique< AdaptiveAssetManagerWorldResetEvent >( inEvent ) )
		);
	}



}
