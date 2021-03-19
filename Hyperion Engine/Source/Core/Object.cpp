/*==================================================================================================
	Hyperion Engine
	Source/Core/Object.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/Object.h"
#include "Hyperion/Core/Engine.h"

#if HYPERION_OS_WIN32
#include "Hyperion/Win32/Win32Headers.h"
#endif


namespace Hyperion
{
	/*
	*	TODO: Make these values console variables
	*/
	constexpr uint32 GC_TRIGGER_OBJ_COUNT		= 5;
	constexpr uint32 GC_MIN_COLLECTION_COUNT	= 10;
	constexpr float GC_COLLECTION_PERCENTAGE	= 0.5f;
	constexpr uint32 GC_MAX_COLLECTION_WAIT_MS	= 10000;


	GarbageCollector::GarbageCollector()
		: m_bIsRunning( false ), m_Counter( 0 ), m_Queue(), m_bFlush( false ), m_bShutdown( false )
	{
	#if HYPERION_OS_WIN32
		m_hCollectEvent		= CreateEvent( NULL, TRUE, FALSE, NULL );
		m_hFlushEvent		= CreateEvent( NULL, TRUE, FALSE, NULL );
	#endif
	}


	GarbageCollector::~GarbageCollector()
	{
		// Ensure there are no objects left to process on shutdown
		HYPERION_VERIFY( m_Counter.load() == 0 && m_bIsRunning.load() == false, "[Core] Garbage collector wasnt finished on shutdown! Ensure 'Flush' is being called in OS layer" );

	#if HYPERION_OS_WIN32
		CloseHandle( m_hCollectEvent );
		CloseHandle( m_hFlushEvent );
	#else 
		HYPERION_NOT_IMPLEMENTED( "[Core] Non win-32 verion of this destructor" );
	#endif

	}


	bool GarbageCollector::Initialize()
	{
		// Ensure we arent already running...
		if( m_Thread != nullptr )
		{
			return false;
		}
		
		// Create our thread
		m_Thread = std::make_unique< std::thread >( std::bind( &GarbageCollector::ThreadBody, this ) );
		m_Thread->detach();

		return true;
	}


	void GarbageCollector::Shutdown()
	{
		if( m_Thread != nullptr )
		{
			// Trigger the thread to destroy all remaining objects before shutting down
			m_bShutdown.store( true );

		#if HYPERION_OS_WIN32
			SetEvent( m_hCollectEvent );
		#else
			HYPERION_NOT_IMPLEMENTED( "[Core] Non win-32 version of this function!" );
		#endif

			if( m_Thread->joinable() )
			{
				m_Thread->join();
			}

			m_Thread.reset();
		}
	}


	void GarbageCollector::Flush()
	{
		// Ensure all objects get destroyed before this function returns
		if( m_Thread != nullptr )
		{
			m_bFlush.store( true );

		#if HYPERION_OS_WIN32
			SetEvent( m_hCollectEvent );
			WaitForSingleObject( m_hFlushEvent, INFINITE );
			ResetEvent( m_hFlushEvent );
		#else
			HYPERION_NOT_IMPLEMENTED( "[Core] Non win-32 version of this function" );
		#endif
		}
	}


	void GarbageCollector::ThreadBody()
	{
		while( true )
		{
			m_bIsRunning.store( true );

			// Determine the number of objects to destroy this iteration
			uint32 objCount			= m_Counter.load();
			uint32 halfObjCount		= (uint32) roundf( GC_COLLECTION_PERCENTAGE * (float) objCount );
			uint32 destCount		= min( max( halfObjCount, GC_MIN_COLLECTION_COUNT ), objCount );

			// We want to also implement flushing.. the idea being we mark the flush boolean as true
			// Then, we trigger an event
			// This causes an iteration, where we detect at some point were flushing, so we continue to iterate until there are no objects left
			
			uint32 count		= 0;
			bool bDidFlush		= m_bFlush.load();
			bool bDidShutdown	= m_bShutdown.load();

			while( count < destCount || bDidFlush || bDidShutdown )
			{
				auto nextEntry = m_Queue.PopValue();
				if( !nextEntry.first || nextEntry.second.ptr == nullptr )
				{
					break;
				}

				nextEntry.second.ptr->DestroyObject();
				delete nextEntry.second.ptr;

				count++;

				bDidFlush		= bDidFlush || m_bFlush.load();
				bDidShutdown	= bDidShutdown || m_bShutdown.load();
			}
			
			// Check if we need to trigger the flush event
			if( bDidFlush )
			{
			#if HYPERION_OS_WIN32
				SetEvent( m_hFlushEvent );
				m_bFlush.store( false );
			#else
				HYPERION_NOT_IMPLEMENTED( "[Core] Non win-32 version of thread body!" );
			#endif
			}

			m_bIsRunning.store( false );

			// Check if we got the shutdown signal
			if( bDidShutdown )
			{
				return;
			}

			// Wait for the next iteration
		#if HYPERION_OS_WIN32
			WaitForSingleObject( m_hCollectEvent, GC_MAX_COLLECTION_WAIT_MS );
			ResetEvent( m_hCollectEvent );
		#else
			HYPERION_NOT_IMPLEMENTED( "[Core] Non win-32 version of thread body!" );
		#endif
		}
	}


	void GarbageCollector::CollectObject( Object* inObj )
	{
		HYPERION_VERIFY( inObj != nullptr, "[Core] Attempt to collect null object!" );

		bool bWasGarbage = inObj->m_bIsGarbage.exchange( true );
		HYPERION_VERIFY( bWasGarbage == false, "[Core] Object was already marked as garbage" );

		inObj->MarkAsGarbage();
		
		Entry e {};
		e.ptr - inObj;

		m_Queue.Push( std::move( e ) );
		uint32 newCount = m_Counter.fetch_add( 1 ) + 1;

		// Check if we should trigger the event
		bool bWasRunning = m_bIsRunning.exchange( true );
		if( !bWasRunning && newCount >= GC_TRIGGER_OBJ_COUNT )
		{
		#if HYPERION_OS_WIN32
			SetEvent( m_hCollectEvent );
		#else
			HYPERION_NOT_IMPLEMENTED( "[Core] Non win-32 version of this function" );
		#endif
		}
	}


	void GarbageCollector::CollectObjectImmediate( Object* inObj )
	{
		HYPERION_VERIFY( inObj != nullptr, "[Core] Attempt to collect null object!" );

		bool bWasGarbage = inObj->m_bIsGarbage.exchange( true );
		HYPERION_VERIFY( bWasGarbage == false, "[Core] Object was already marked as garbage" );

		inObj->MarkAsGarbage();
		inObj->DestroyObject();

		delete inObj;
	}


	bool GarbageCollector::IsCleanupRunning() const
	{
		return m_bIsRunning.load();
	}


	void _performGC( std::atomic< Object* >& inGC )
	{
		// Attempt to steal the pointer, if we do, then perform collection
		auto* objPtr = inGC.exchange( nullptr );
		if( objPtr != nullptr )
		{
			auto gc = Engine::Get().GetGC();
			HYPERION_VERIFY( gc != nullptr, "[Core] GC is shutdown before all objects were finalized!" );

			switch( objPtr->GetGarbageCollectionMethod() )
			{
				case GarbageCollectionMethod::Immediate:
				{
					gc->CollectObjectImmediate( objPtr );
					break;
				}
				case GarbageCollectionMethod::Deferred:
				default:
				{
					gc->CollectObject( objPtr );
					break;
				}
			}
		}
	}


	/*
	*	Object - GetType
	*/
	HypPtr< Type > Object::GetType() const
	{
		HYPERION_VERIFY( m_ObjState != nullptr, "[Core] Object: State was null!" );

		auto typeInfo = RTTI::GetTypeInfo( m_ObjState->rtti_id );
		HYPERION_VERIFY( typeInfo != nullptr, "[Core] Object: Type info was null!" );

		return CreateObject< Type >( typeInfo );
	}


	/*------------------------------------------------------------------------------------------------------
		Type Class Definition
	------------------------------------------------------------------------------------------------------*/

	bool Type::IsParentTo( const HypPtr< Type >& Other ) const
	{
		if( !Other ) { return false; }
		HYPERION_VERIFY( m_TypeInfo && Other->m_TypeInfo, "[RTTI] Type class had invalid info pointer" );

		// Check if we are a parent to other

		// Object is a parent to every registered 'type'
		if( m_TypeInfo->Identifier == typeid( Object ).hash_code() ) { return true; }

		for( auto It = Other->m_TypeInfo->ParentChain.begin(); It != Other->m_TypeInfo->ParentChain.end(); It++ )
		{
			if( *It == m_TypeInfo->Identifier ) { return true; }
		}

		return false;
	}


	bool Type::IsDirectParentTo( const HypPtr< Type >& Other ) const
	{
		if( !Other ) { return false; }
		HYPERION_VERIFY( m_TypeInfo && Other->m_TypeInfo, "[RTTI] Type class had invalid info pointer" );

		// Check if we are direct parent to other

		// If an inheritance list is empty, it means its parent class is 'Object', so we check that case here
		if( Other->m_TypeInfo->ParentChain.size() == 0 && m_TypeInfo->Identifier == typeid( Object ).hash_code() ) { return true; }
		else return( *Other->m_TypeInfo->ParentChain.begin() == m_TypeInfo->Identifier );
	}


	bool Type::IsDerivedFrom( const HypPtr< Type >& Other ) const
	{
		if( !Other ) { return false; }
		HYPERION_VERIFY( m_TypeInfo && Other->m_TypeInfo, "[RTTI] Type class had invalid info pointer" );

		if( m_TypeInfo->Identifier == typeid( Object ).hash_code() ) { return false; }

		// Check if other is a parent to this
		// if 'Other' is an 'Object', then we must be derived from it
		if( typeid( Object ).hash_code() == Other->m_TypeInfo->Identifier ) { return true; }

		for( auto It = m_TypeInfo->ParentChain.begin(); It != m_TypeInfo->ParentChain.end(); It++ )
		{
			if( *It == Other->m_TypeInfo->Identifier ) { return true; }
		}

		return false;
	}


	bool Type::IsDirectlyDerivedFrom( const HypPtr< Type >& Other ) const
	{
		if( !Other ) { return false; }
		HYPERION_VERIFY( m_TypeInfo && Other->m_TypeInfo, "[RTTI] Type class had invalid info pointer" );

		if( m_TypeInfo->Identifier == typeid( Object ).hash_code() ) { return false; }

		// Check if other is a direct parent to this
		// If our inheritance list is empty, it means were derived directly from 'Object'
		if( m_TypeInfo->ParentChain.size() == 0 && Other->m_TypeInfo->Identifier == typeid( Object ).hash_code() ) { return true; }
		else return( *m_TypeInfo->ParentChain.begin() == Other->m_TypeInfo->Identifier );
	}


	std::vector< HypPtr< Type > > Type::GetParentList() const
	{
		HYPERION_VERIFY( m_TypeInfo != nullptr, "[RTTI] Type object was missing info pointer!" );

		std::vector< HypPtr< Type > > result;
		result.reserve( m_TypeInfo->ParentChain.size() + 1 );

		// First, loop through and add parent types in order (closest parent -> furthest parent)
		for( auto It = m_TypeInfo->ParentChain.begin(); It != m_TypeInfo->ParentChain.end(); It++ )
		{
			auto type = CreateObject< Type >( RTTI::GetTypeInfo( *It ) );
			HYPERION_VERIFY( type != nullptr, "[RTTI] Couldnt find type entry for parent" );

			result.push_back( type );
		}

		// Add base 'Object' type to list
		result.push_back( CreateObject< Type >( RTTI::GetObjectType() ) );

		return result;
	}


	HypPtr< Type > Type::GetParent() const
	{
		HYPERION_VERIFY( m_TypeInfo != nullptr, "[RTTI] Type object was missing info pointer!" );

		if( m_TypeInfo->ParentChain.size() == 0 )
		{
			return CreateObject< Type >( RTTI::GetObjectType() );
		}
		else
		{
			auto parent_info = RTTI::GetTypeInfo( m_TypeInfo->ParentChain.front() );
			HYPERION_VERIFY( parent_info != nullptr, "[RTTI] Couldnt find type entry for parent" );

			return CreateObject< Type >( parent_info );
		}
	}


	std::vector< HypPtr< Type > > Type::GetDirectChildren() const
	{
		HYPERION_VERIFY( m_TypeInfo != nullptr, "[RTTI] Type object was missing info pointer!" );

		std::vector< HypPtr< Type > > output;
		output.reserve( m_TypeInfo->ChildrenList.size() );

		for( auto It = m_Info->ChildrenList.begin(); It != m_Info->ChildrenList.end(); It++ )
		{
			auto typePtr = CreateObject< Type >( RTTI::GetTypeInfo( *It ) );
			HYPERION_VERIFY( typePtr && typePtr->m_TypeInfo, "[RTTI] Failed to get child type instance!" );

			output.push_back( typePtr );
		}

		return output;
	}

	/*
	*	Private helper function
	*/
	void ProcessChildTreeNode( const std::shared_ptr< RTTI::TypeInfo >& inNode, std::vector< HypPtr< Type > >& outList )
	{
		HYPERION_VERIFY( inNode != nullptr, "[RTTI] Failed to build full subclass list.. a type was invalid" );

		// Add this node to the list
		auto thisType = CreateObject< Type >( inNode );
		outList.push_back( thisType );

		for( auto It = inNode->ChildrenList.begin(); It != inNode->ChildrenList.end(); It++ )
		{
			ProcessChildTreeNode( RTTI::GetTypeInfo( *It ), outList );
		}
	}

	std::vector< HypPtr< Type > > Type::GetChildren() const
	{
		HYPERION_VERIFY( m_TypeInfo != nullptr, "[RTTI] Type object was missing info pointer!" );

		std::vector< HypPtr< Type > > output;

		// Loop through children and build list
		for( auto It = m_TypeInfo->ChildrenList.begin(); It != m_TypeInfo->ChildrenList.end(); It++ )
		{
			ProcessChildTreeNode( RTTI::GetTypeInfo( *It ), output );
		}

		return output;
	}


	HypPtr< Object > Type::CreateInstance() const
	{
		HYPERION_VERIFY( m_TypeInfo != nullptr, "[RTTI] Type class had invalid info pointer" );
		return m_TypeInfo->CreateFunc ? m_TypeInfo->CreateFunc() : nullptr;
	}


	HypPtr< Type > Type::Get( size_t inIdentifier )
	{
		// Check for 'object' type
		if( inIdentifier == typeid( Object ).hash_code() )
		{
			return CreateObject< Type >( RTTI::GetObjectType() );
		}

		// Find entry in RTTI table
		auto infoPtr = RTTI::GetTypeInfo( inIdentifier );

		// Create and return type instance
		return infoPtr ? CreateObject< Type >( infoPtr ) : nullptr;
	}


	HypPtr< Type > Type::Get( const String& inIdentifier )
	{
		// Check if parameter is valid
		if( inIdentifier.IsWhitespaceOrEmpty() )
		{
			return nullptr;
		}

		// Typenames are always stored lowercase
		String lowerName = String::ToLower( inIdentifier );

		// Check for 'Object' type
		if( String::Equals( lowerName, "object" ) )
		{
			return CreateObject< Type >( RTTI::GetObjectType() );
		}

		// Lookup info from RTTI
		auto infoPtr = RTTI::GetTypeInfo( inIdentifier );

		return infoPtr ? CreateObject< Type >( infoPtr ) : nullptr;
	}


	HypPtr< Type > Type::Get( const HypPtr< Object >& inObj )
	{
		return inObj->GetType();
	}

}


/*
*	Register the 'Type' type with RTTI
*	- If instantiated through the type system (i.e. myType.CreateInstance()), by default
*		it will refer to the base 'Object' type
*/
HYPERION_REGISTER_OBJECT_TYPE( Type, Object, Hyperion::RTTI::GetObjectType() );