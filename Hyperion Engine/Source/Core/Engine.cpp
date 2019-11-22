/*==================================================================================================
	Hyperion Engine
	Source/Core/Engine.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Framework/Entity.h"
#include "Hyperion/Core/InputManager.h"
#include "Hyperion/Framework/World.h"
#include "Hyperion/Renderer/RenderFactory.h"
#include "Hyperion/Renderer/RenderMarshal.h"
#include "Hyperion/Core/File.h"
#include "Hyperion/Core/Platform.h"
#include <typeindex>
#include <type_traits>

// DEBUG
#include <thread>
#include <chrono>

std::unique_ptr< Hyperion::Engine > Hyperion::Engine::m_Instance;

namespace Hyperion
{

	/*
		Constructor
	*/
	Engine::Engine()
	{
		m_LastObjectID	= OBJECT_INVALID;
		m_Status		= EngineStatus::NotStarted;

		// Create sub-objects
		m_ThreadManager		= CreateObject< ThreadManager >();
		m_InputManager		= CreateObject< InputManager >();

		// Initialize any platform services
		Platform::Init();
	}


	/*
		Engine::GetObjectCache( ObjectCacheID )
		* Lookup an object cache via a cache identifier
	*/
	ObjectCache* Engine::GetObjectCache( ObjectCacheID Identifier )
	{
		if( Identifier == CACHE_INVALID )
			return nullptr;

		// Lookup the group in the object cache
		auto cacheGroup = m_ObjectCache.find( Identifier );
		return cacheGroup == m_ObjectCache.end() ? nullptr : std::addressof( cacheGroup->second );
	}

	/*
		Engine::FullObjectLookup( ObjectID )
		* Look through all object caches for the desired target object (slow)
	*/
	std::shared_ptr< Object > Engine::FullObjectLookup( ObjectID Identifier )
	{
		if( Identifier == OBJECT_INVALID )
			return nullptr;

		for( auto cacheIter = m_ObjectCache.begin(); cacheIter != m_ObjectCache.end(); cacheIter++ )
		{
			for( auto objIter = cacheIter->second.begin(); objIter != cacheIter->second.end(); objIter++ )
			{
				if( objIter->first == Identifier )
				{
					// Ensure the object is valid
					if( !objIter->second.expired() )
					{
						auto objCast = std::dynamic_pointer_cast< Object >( objIter->second.lock() );
						if( objCast )
						{
							return objCast;
						}
					}

					return nullptr;
				}
			}
		}

		return nullptr;
	}

	/*
		Engine::QuickObjectLookupByCache( ObjectID, ObjectCacheID )
		* Lookup an object by the object identifier, and cache identifier
	*/
	std::shared_ptr< Object > Engine::QuickObjectLookup( ObjectID ObjIdentifier, ObjectCacheID CacheIdentifier )
	{
		if( ObjIdentifier == OBJECT_INVALID )
			return nullptr;
		else if( CacheIdentifier == CACHE_INVALID )
			return FullObjectLookup( ObjIdentifier );

		// Get the proper cache group
		auto* objCache = GetObjectCache( CacheIdentifier );
		if( !objCache )
		{
			// Print error?
			return nullptr;
		}

		for( auto It = objCache->begin(); It != objCache->end(); It++ )
		{
			if( It->first == ObjIdentifier )
			{
				// Check if object is valid
				if( !It->second.expired() )
				{
					auto objCast = std::dynamic_pointer_cast< Object >( It->second.lock() );
					if( objCast )
					{
						return objCast;
					}
				}

				return nullptr;
			}
		}

		return nullptr;
	}

	/*
		Engine::IsObjectValid( ObjectID )
		* Check through the entire object cache, for this object and check if its valid (slow!)
	*/
	bool Engine::IsObjectValid( ObjectID Identifier )
	{
		return FullObjectLookup( Identifier ) != nullptr;
	}

	/*
		Engine::IsObjectValid( ObjectID, ObjectCacheID )
		* Check if an object is valid, given the cache group and object identifier
	*/
	bool Engine::IsObjectValid( ObjectID Identifier, ObjectCacheID CacheIdentifier )
	{
		return QuickObjectLookup( Identifier, CacheIdentifier ) != nullptr;
	}


	/*
		Engine::DestroyObject( Object* )
		* Destroy the target object, this wont invalidate any hanging shared_ptr's 
		* Until all shared_ptr's are out of scope, the object actually wont be freed
		* To check if the object has been destroyed, call Object::IsValid()
	*/
	bool Engine::DestroyObject( Object* Target )
	{
		// Check parameter
		if( !Target )
			return false;

		auto objIdentifier = Target->GetID();

		// Remove any input mappings for this object
		if( m_InputManager && Target != m_InputManager.get() )
		{
			m_InputManager->ClearBindings( Target );
		}

		// Perform shutdown on this object
		if( Target->m_IsValid )
		{
			Target->PerformObjectShutdown();
			Target->m_IsValid = false;
		}

		// We need to remove the object from the cache
		for( auto CacheIter = m_ObjectCache.begin(); CacheIter != m_ObjectCache.end(); CacheIter++ )
		{
			for( auto ObjIter = CacheIter->second.begin(); ObjIter != CacheIter->second.end(); ObjIter++ )
			{
				if( ObjIter->first == objIdentifier )
				{
					// Clean the object from the cache
					CacheIter->second.erase( ObjIter->first );
					return true;
				}
			}
		}

		return false;
	}

	/*
		Engine::TickObjects()
		* Loops through the list of all objects, and tick each one
		* Only objects that have ticking enabled will have the method actually called
	*/
	void Engine::TickObjects()
	{
		for( auto cIt = m_ObjectCache.begin(); cIt != m_ObjectCache.end(); cIt++ )
		{
			for( auto oIt = cIt->second.begin(); oIt != cIt->second.end(); oIt++ )
			{
				if( !oIt->second.expired() )
				{
					// Get pointer to the object
					auto target = oIt->second.lock();
					if( target && target->IsValid() && target->b_RequiresTick )
					{
						target->PerformObjectTick();
					}
				}
			}
		}
	}

	/*
		Engine Main Thread Functions
	*/
	void Engine::TickEngine()
	{
		// TODO: Events/Async Tasks/User Input/etc...

		// Calculate time since the last tick
		std::chrono::duration< double, std::milli > tickDelta = std::chrono::high_resolution_clock::now() - m_lastTick;

		// We want to aim for around 10ms or more between ticks, otherwise we will sleep
		// TODO: Evaluate this decision
		if( tickDelta.count() < 10.0 )
		{
			std::this_thread::sleep_until( m_lastTick + std::chrono::milliseconds( 10 ) );
		}
		
		// Recalc more accurate delta
		auto Now = std::chrono::high_resolution_clock::now();
		std::chrono::duration< double > newDelta = Now - m_lastTick;

		// Dispatch input events
		m_InputManager->DispatchQueue( newDelta.count() );

		// Store current time
		m_lastTick = Now;

		// Tick all objects
		TickObjects();
	}

	void Engine::InitEngine()
	{
		std::cout << "[DEBUG] Engine: Main engine thread init...\n";
		m_lastTick = std::chrono::high_resolution_clock::now();

		// Create the game world
		m_World = CreateObject< World >();

		// Allow derived class to initialize
		OnInitialize();
	}

	void Engine::ShutdownEngine()
	{
		std::cout << "[DEBUG] Engine: Main engine thread shutdown...\n";

		// Allow derived class to shutdown
		OnShutdown();

		// Shutdown the world
		m_World.reset();
	}

	/*
		Engine::Startup()
		* Starts the engine, creates threads, etc...
	*/
	bool Engine::Startup()
	{
		// Check status
		if( m_Status != EngineStatus::NotStarted )
		{
			std::cout << "[ERROR] Engine: Attempt to start the engine, while the engine is in the wrong state!\n";
			return false;
		}

		// Update Status
		m_Status = EngineStatus::Init;

		// TODO: Load which type of renderer we want to use
		// DEBUG: For now.. were simply setting the renderer type to DirectX11
		IRenderFactory::SetActiveRenderer( RendererType::DirectX11 );

		IRenderFactory& RenderFactory = IRenderFactory::GetInstance();

		m_Renderer			= RenderFactory.CreateRenderer();
		m_RenderMarshal		= CreateObject< RenderMarshal >();

		// Store weak ref to renderer in the Marshaler, so it can quickly access it
		m_RenderMarshal->m_RendererRef = std::weak_ptr< Renderer >( m_Renderer );

		// Create all of the threads needed
		m_ThreadManager->m_EngineThread = m_ThreadManager->CreateThread( "engine_main", 
			std::bind( &Engine::TickEngine, this ), 
			std::bind( &Engine::InitEngine, this ), 
			std::bind( &Engine::ShutdownEngine, this ) 
		);

		m_ThreadManager->m_RenderThread = m_ThreadManager->CreateThread( "render_main", 
			std::bind( &Renderer::TickThread, m_Renderer.get() ), 
			std::bind( &Renderer::InitThread, m_Renderer.get() ), 
			std::bind( &Renderer::ShutdownThread, m_Renderer.get() ) 
		);

		m_ThreadManager->m_RenderMarshalThread = m_ThreadManager->CreateThread( "render_marshal",
			std::bind( &RenderMarshal::TickThread, m_RenderMarshal.get() ),
			std::bind( &RenderMarshal::InitThread, m_RenderMarshal.get() ),
			std::bind( &RenderMarshal::ShutdownThread, m_RenderMarshal.get() )
		);

		// Run threads
		bool bEngThrSuccess = m_ThreadManager->m_EngineThread->Start();
		bool bRndThrSuccess = m_ThreadManager->m_RenderThread->Start();
		bool bRndMshSuccess = m_ThreadManager->m_RenderMarshalThread->Start();

		if( !bEngThrSuccess || !bRndThrSuccess || !bRndMshSuccess )
		{
			// TODO: Error out, close down engine
			std::cout << "[ERROR] Engine: Failed to create threads!\n";
			Shutdown();
			return false;
		}

		// Store thread id's for the main threads
		__gGameThreadId				= m_ThreadManager->m_EngineThread->GetSystemIdentifier();
		__gRenderThreadId			= m_ThreadManager->m_RenderThread->GetSystemIdentifier();
		__gRenderMarshalThreadId	= m_ThreadManager->m_RenderMarshalThread->GetSystemIdentifier();

		// Update status
		m_Status = EngineStatus::Running;

		return true;
	}

	bool Engine::Shutdown()
	{
		if( m_Status != EngineStatus::Running )
		{
			std::cout << "[ERROR] Engine: Attempt to stop the engine when it wasnt running!\n";
			//return false;
		}

		// Update Status
		m_Status = EngineStatus::Shutdown;

		// Clear all input mappings
		if( m_InputManager )
		{
			m_InputManager->ClearAllBindings();
		}

		// Stop all of our threads.. for now, this is a blocking call and it will call thread shutdown functions
		if( m_ThreadManager )
		{
			m_ThreadManager->StopThreads();
		}

		// Clear stored thread id's
		__gGameThreadId				= std::thread::id();
		__gRenderThreadId			= std::thread::id();
		__gRenderMarshalThreadId	= std::thread::id();

		return true;
	}


	void Engine::RegisterAssetLoaders()
	{

	}

}