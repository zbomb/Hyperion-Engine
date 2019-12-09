/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Engine.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

// Hyperion 
#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Object.h"
#include "Hyperion/Core/Threading.h"
#include "Hyperion/Core/InputManager.h"
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Core/String.h"

// STL
#include <type_traits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#ifdef HYPERION_DEBUG_OBJECT
#include <iostream>
#endif

#ifdef HYPERION_OS_WIN32
#include <Windows.h>
#endif

namespace Hyperion
{
	/*
		Typedefs
	*/
	typedef std::map< ObjectID, std::weak_ptr< Object > > ObjectCache;

	class Entity;
	class World;
	class InputManager;
	class RenderMarshal;

	enum class EngineStatus
	{
		NotStarted,
		Init,
		Running,
		Shutdown
	};

	struct GameInfo
	{
		String DisplayName;
		String FolderName;
		String AuthorName;
		String AuthorContact;
		uint8 MajorVersion;
		uint8 MinorVersion;
		uint8 BuildNumber;
	};

	/*
		Engine Declaration
	*/
	class Engine
	{

		/*
			Singleton Access Pattern
		*/
	private:

		static std::unique_ptr< Engine > m_Instance;

	public:

		template< typename _EngTy >
		static Engine& CreateInstance()
		{
			if( m_Instance )
			{
				throw std::runtime_error( "Attempt to create engine instance, when one already exists!" );
			}

			m_Instance = std::make_unique< _EngTy >();
			return *m_Instance;
		}

		static Engine& GetInstance()
		{
			if( !m_Instance )
			{
				throw std::runtime_error( "Attempt to get engine singleton before initialization!" );
			}

			return *m_Instance;
		}

	public:

		Engine();
		Engine( Engine const& ) = delete;
		void operator=( Engine const& ) = delete;

		/*
			Engine Info
		*/
		virtual String GetGameName() const = 0;
		virtual String GetDocumentsFolderName() const = 0;
		virtual String GetAuthorName() const = 0;
		virtual String GetVersionName() const = 0;

		virtual void OnInitialize() {}
		virtual void OnShutdown() {}

		virtual void RegisterAssetLoaders();

		/*=======================================================================================================
				Object System
		=======================================================================================================*/

	private:

		ObjectID m_LastObjectID;
		std::map< size_t, ObjectCache > m_ObjectCache;

	public:

		/*
			Cache Lookup
		*/

		/*
			Helpers to determine if a class has a custom object group assigned or not
		*/
		template< typename, typename >
		struct has_custom_object_group;

		template< typename T, typename Ret, typename... Args >
		struct has_custom_object_group< T, Ret( Args... ) > {
			template< typename U, U > struct Check;

			template< typename U >
			static std::true_type Test( Check< Ret(*)( Args... ), &U::GetCacheGroup >* );

			template< typename U >
			static std::false_type Test(...);

			static const bool value = decltype( Test< T >( 0 ) )::value;
		};

		// Get the cache group for a particular type
		template< typename T >
		ObjectCacheID GetCacheIdentifier()
		{
			// We want to make sure the template parameter is valid at compile time
			static_assert( std::is_base_of< Object, T >::value, "Invalid object type specified, must derive from Object!" );
			static_assert( !std::is_same< Object, T >::value, "Invalid object type specified, Object baseclass specified?" );
			static_assert( !std::is_same< Entity, T >::value, "Invalid object type specified, Entity baseclass specified?" );

			// We also need to check if the class actually has the static function defined to get the cache identifier
			// If not, we will return the default cache identifier, and it will be put in the 'main' object group
			bool bHasCustomGroup = has_custom_object_group< T, ObjectCacheID( void ) >::value;
			return bHasCustomGroup ? T::GetCacheGroup() : CACHE_NONE;
		}

		// Lookup object cache via a type
		template< typename T >
		ObjectCache* GetObjectCache()
		{
			// Ensure the template parameter is valid at compile time
			static_assert( std::is_base_of< Object, T >::value, "Invalid object type specified, must derive from Object!" );
			static_assert( !std::is_same< Object, T >::value, "Invalid object type specified, Object baseclass specified?" );
			static_assert( !std::is_same< Entity, T >::value, "Invalid object type specified, Entity baseclass specified?" );

			// Get the cache group from the static method in the object derived class
			return GetObjectCache( GetCacheIdentifier< T >() );
		}

		// Lookup object cache via a cache group ID
		ObjectCache* GetObjectCache( ObjectCacheID CacheID );

		/*
			Single Object Lookup
		*/

		// Lookup single object via type and ID (fastest)
		template< typename T >
		std::shared_ptr< Object > QuickObjectLookup( ObjectID Identifier )
		{
			// Ensure the template parameter is valid at compile time
			static_assert( std::is_base_of< Object, T >::value, "Invalid object type specified, must derive from Object!" );
			static_assert( !std::is_base_of< Entity, T >::value, "Invalid object type specified, must not be an Entity!" );
			static_assert( !std::is_same< Object, T >::value, "Invalid object type specified, Object baseclass specified?" );
			static_assert( !std::is_same< Entity, T >::value, "Invalid object type specified, Entity baseclass specified?" );

			return QuickObjectLookup( Identifier, GetCacheIdentifier< T >() );
		}

		// Lookup single object via ID alone (slowest)
		std::shared_ptr< Object > FullObjectLookup( ObjectID Identifier );

		// Lookup single object via cache identifier, and object identifier (fastest)
		std::shared_ptr< Object > QuickObjectLookup( ObjectID Identifier, ObjectCacheID CacheIdentifier );

		/*
			Validity Testing
		*/

		// Check if an object is valid, via type and ID (fastest)
		template< typename T >
		bool IsObjectValid( ObjectID Identifier )
		{
			// Ensure the template parameter is valid at compile time
			static_assert( std::is_base_of< Object, T >::value, "Invalid object type specified, must derive from Object!" );
			static_assert( !std::is_base_of< Entity, T >::value, "Invalid object type specified, must not be an Entity!" );
			static_assert( !std::is_same< Object, T >::value, "Invalid object type specified, Object baseclass specified?" );
			static_assert( !std::is_same< Entity, T >::value, "Invalid object type specified, Entity baseclass specified?" );
		
			return IsObjectValid( Identifier, GetCacheIdentifier< T >() );
		}

		// Check if an object is valid, via ID alone (slowest)
		bool IsObjectValid( ObjectID Identifier );

		// Check if an object is valid, via cache group and identifier (fastest)
		bool IsObjectValid( ObjectID Identifier, ObjectCacheID CacheIdentifier );

		/*
			Object Creation
		*/
	private:

		/*
			Engine::CreateRawObject< T >()
			* Creates an object, with the only compile-time check being, it MUST be derived from object
		*/
		template< typename T >
		std::shared_ptr< T > CreateRawObject()
		{
			// Ensure were only creating Object derived classes with this function
			static_assert( std::is_base_of< Object, T >::value, "Invalid object type specified, must derive from Object!" );

			// Generate new unique id, get type hash, and get the cache group for this object type
			auto objIdentifier = ++m_LastObjectID;
			auto objTypeHash = typeid( T ).hash_code();
			auto objGroup = GetCacheIdentifier< T >();

			// Create new shared_ptr with the custom deleter that will call Engine::DestroyObject
			std::shared_ptr< T > newObj = std::shared_ptr< T >( new T(), [=] ( T* Obj )
				{
					// Custom Deleter
					auto& gameEngine = Engine::GetInstance();
					gameEngine.DestroyObject( Obj );

					delete Obj;
				} );

			// Downcast shared_ptr to an Object ptr
			std::shared_ptr< Object > objPtr = std::dynamic_pointer_cast< Object >( newObj );
			if( !objPtr )
			{
				std::cout << "[ERROR] Engine: Failed to downcast created object...\n";
				return nullptr; // Created object will go out of scope and be destroyed
			}

			// Store the new object as a weak_ptr
			m_ObjectCache[ objGroup ][ objIdentifier ] = std::weak_ptr< Object >( objPtr );

			// Setup Object Fields
			objPtr->m_Identifier	= objIdentifier;
			objPtr->m_TypeHash		= objTypeHash;
			objPtr->m_IsValid		= true;
			objPtr->m_WeakThis		= std::weak_ptr< Object >( objPtr );

			// Allow Object to bind user input
			objPtr->BindUserInput( m_InputManager.get() );

			// Init Object
			objPtr->PerformObjectInit();

			// Return new Object as target type
			return newObj;
		}

	public:

		// Create an object of the desired type
		template< typename T >
		std::shared_ptr< T > CreateObject()
		{
			// Ensure the template parameter is valid at compile time (i.e. Derived from object, not an object itself, not derived from entity, and not an entity itself)
			static_assert( !std::is_base_of< Entity, T >::value, "Invalid object type specified, must not be an Entity!" );
			static_assert( !std::is_same< Object, T >::value, "Invalid object type specified, Object baseclass specified?" );
			static_assert( !std::is_same< Entity, T >::value, "Invalid object type specified, Entity baseclass specified?" );

			return CreateRawObject< T >();
		}

		/*
			Object Destruction
		*/

		// Destroy target object
		bool DestroyObject( Object* Target );

		friend class World;

		/*
			Object Tick System
		*/
		void TickObjects();

		/*
			Thread Manager
		*/
	private:

		std::shared_ptr< ThreadManager > m_ThreadManager;

	public:

		inline const std::shared_ptr< ThreadManager >& GetThreadManager() const { return m_ThreadManager; }

		/*
			Initialization System
		*/
	private:

		EngineStatus m_Status;
		std::chrono::time_point< std::chrono::high_resolution_clock > m_lastTick;

		std::shared_ptr< InputManager > m_InputManager;

		void InitEngine();
		void TickEngine();
		void ShutdownEngine();

	public:

		inline EngineStatus GetStatus() const { return m_Status; }

		bool Startup();
		bool Shutdown();

		inline const std::shared_ptr< InputManager >& GetInputManager() const { return m_InputManager; }


		/*=======================================================================================================
				Engine Initialization
		=======================================================================================================*/
	private:

		std::shared_ptr< World > m_World;

	public:

		inline std::weak_ptr< World > GetWorld() { return std::weak_ptr< World >( m_World ); }

		/*=======================================================================================================
				Render System
		=======================================================================================================*/
	private:

		std::shared_ptr< Renderer > m_Renderer;

#ifdef HYPERION_OS_WIN32
		HWND m_renderTarget;
#endif

	public:

		inline std::weak_ptr< Renderer > GetRenderer() { return std::weak_ptr< Renderer >( m_Renderer ); }

#ifdef HYPERION_OS_WIN32
		void SetRenderTarget( HWND inRenderTarget )
		{
			m_renderTarget = inRenderTarget;
			if( m_Renderer )
			{
				auto params = m_Renderer->GetParameters();
				params.OutputWindow = inRenderTarget;
				m_Renderer->UpdateParameters( params );
			}
		}

		inline HWND GetRenderTarget()
		{
			if( m_Renderer )
			{
				return m_Renderer->GetParameters().OutputWindow;
			}

			return NULL;
		}
#endif

	};

}