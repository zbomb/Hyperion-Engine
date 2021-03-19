/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Object.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"
#include "Hyperion/Core/RTTI.h"
#include "Hyperion/Core/Types/ConcurrentQueue.h"

#include <chrono>
#include <memory>
#include <map>

#undef min

/*--------------------------------------------------------------------
	Updated Object System
--------------------------------------------------------------------*/

namespace Hyperion
{
	/*
	*	Forward Decl.
	*/
	class Object;
	class Type;

	/*======================================================================================================
	*	class GarbageCollector
	*	- Shouldnt be used outside of the engine codebase
	*	- Holds a list of objects that need to be destroyed
	*	- Performs the destruction asyncronously on a seperate thread
	*	- We have a dedicated thread, that waits for the pending object count to hit a threshold, and when
	*	it does, we add a task to delete a percentage of the pending objects, at least the min number set
	======================================================================================================*/
	class GarbageCollector
	{

	private:

		struct Entry
		{
			Object* ptr;

		};

		ConcurrentQueue< Entry > m_Queue;
		std::atomic< uint32 > m_Counter;
		std::atomic< bool > m_bIsRunning;
		std::atomic< bool > m_bFlush;
		std::atomic< bool > m_bShutdown;

		std::unique_ptr< std::thread > m_Thread;

	#if HYPERION_OS_WIN32
		void* m_hCollectEvent;
		void* m_hFlushEvent;
	#endif

		void ThreadBody();

	public:

		GarbageCollector();
		~GarbageCollector();

		/*
		*	Initialize
		*	- Only call from the main 'os' thread
		*/
		bool Initialize();

		/*
		*	Shutdown
		*	- Only call from the main 'os' thread
		*/
		void Shutdown();

		/*
		*	Flush
		*	- NOT thread-safe, only a single thread can wait for the GC to flush at a time
		*	- TODO: Make this thread-safe, whenever its called a new syncronization primitive is created and added to a queue?
		*/
		void Flush();

		/*
		*	CollectObject
		*	- Marks an object to be destroyed by the garbage collector
		*	- Runs the 'OnGarbageCollected' hook
		*	- Once the object is actually asyncronously shutdown, the Shutdown 
		*/
		void CollectObject( Object* inObj );

		/*
		*	CollectObjectImmediate
		*	- Immediatley shutdown and cleanup the object on the thread that is calling this function
		*/
		void CollectObjectImmediate( Object* inObj );

		/*
		*	IsCleanupRunning
		*	- Checks if the GC is currently disposing of objects on its thread
		*/
		bool IsCleanupRunning() const;
	};


	/*======================================================================================================
	*	class _ObjectState [INTERNAL]
	*	- Should only be used internally, within the Object system
	======================================================================================================*/
	struct _ObjectState
	{
		std::atomic< Object*> ptr;
		std::atomic< int32 > refcount;
		const size_t rtti_id;

		_ObjectState() = delete;

		_ObjectState( Object* inPtr, size_t inTypeId )
			: ptr( inPtr ), refcount( 1 ), rtti_id( inTypeId )
		{
			HYPERION_VERIFY( inPtr != nullptr, "[Object] Attempt to create an 'object state' with a null object ptr!" );
		}

		~_ObjectState()
		{
			// Ensure the object has been properly shut down
			HYPERION_VERIFY( ptr == nullptr, "[Object] State is shutting down, but the object was never destroyed?" ); // TODO: Can we remove this?

			ptr.store( nullptr );
			refcount.store( 0 );
		}

		_ObjectState() = delete;
		_ObjectState( const _ObjectState& ) = delete;
		void operator=( const _ObjectState& ) = delete;
	};

	/*======================================================================================================
	*	function _performGC
	*	- Helper function called from HypPtr to send the contained object to the garbage collector
	*	- Not for use outside of engine code
	======================================================================================================*/
	void _performGC( std::atomic< Object* >& inObj );

	/*======================================================================================================
	*	class HypPtr
	*	- Strong reference to an object, supports multi-threading
	======================================================================================================*/
	template< typename _ObjType,
		typename = typename std::enable_if_t< std::is_base_of< Object, _ObjType >::value || std::is_same< Object, _ObjType >::value > >
	class HypPtr
	{

		/*
		*	HypPtr Notes:
		*	- We want to make a thread-safe 'Object' baseclass/system, and have a custom smart pointer type so we can integrate well with our RTTI system 
		*	- The 'HypPtr' behaves like a standard library 'shared_ptr', holds a strong ref to the object, and it will not be collected by GC until there are no strong refs 
		*	- The 'HypWeakPtr' behaves like a standard library 'weak_ptr', doesnt hold strong ref
		*	- We also have to implement a full GC system, to run the destructor for Objects when there is no refs left, on a seperate thread as well.
		*	- The new object system isnt going to have a central cache, although we will have a GC queue we push dead objects to
		*	- Also, were getting rid of 'unique identifiers' for the Object base-class. GameObject will implement unique identifiers 
		*/

	private:

		/*
		*	Data Members
		*/
		std::shared_ptr< _ObjectState > state;
		_ObjType* ptr;


		/*
		*	Private/Helper Functions
		*/
		bool _verifyContentsConst() const
		{
			return( ptr != nullptr && state != nullptr && ptr->IsValid() && state->refcount.load() > 0 );
		}

		void _checkConstruction()
		{
			if( !_verifyContentsConst() )
			{
				ptr		= nullptr;
				state	= nullptr;
			}
		}

		void _incRefAtomic()
		{
			if( state )
			{
				auto prevValue = state->refcount.fetch_add( 1 );
				HYPERION_VERIFY( prevValue > 0, "[Core] Incremented ref count for dead object!" );
			}
		}

		void _decRefAtomic()
		{
			if( state )
			{
				auto prevValue = state->refcount.fetch_sub( 1 );
				if( prevValue <= 0 )
				{
					prevValue.store( 0 );
				}

				if( prevValue <= 1 )
				{
					// Call helper function to send the object to the garbage collector
					_performGC( state->ptr );
					state.reset();
				}
			}
		}

	public:

		/*
		*	Default Constructor
		*/
		HypPtr()
			: state( nullptr ), ptr( nullptr )
		{

		}

		/*
		*	Implicit conversion from nullptr
		*/
		HypPtr( nullptr_t )
			: state( nullptr ), ptr( nullptr )
		{

		}

		/*
		*	Copy Constructor
		*/
		HypPtr( const HypPtr& other )
			: state( other.state ), ptr( other.ptr )
		{
			_incRefAtomic();
			_checkConstruction();
		}

		/*
		*	Construct from raw pointer
		*/
		HypPtr( _ObjType* inPtr, const std::shared_ptr< _ObjectState >& inState )
			: HypPtr()
		{
			if( inPtr == nullptr || inState == nullptr )
			{
				Console::WriteLine( "[ERROR] Core: Failed to create HypPtr, the parameters were invalid!" );
			}
			else
			{
				ptr		= inPtr;
				state	= inState;

				_checkConstruction();
			}
		}

		/*
		*	Implicit conversion between related pointer types
		*/
		template< typename _OtherType,
			typename = typename std::enable_if< std::is_base_of< _ObjType, _OtherType  >::value >::type >
			HypPtr( const HypPtr< _OtherType >& inDerived )
		{
			if( inDerived.IsValid() )
			{
				// If the object isnt valid, create a default null ptr
				ptr		= nullptr;
				state	= nullptr;
			}
			else
			{
				// Perform the cast
				auto* newObject = dynamic_cast<_ObjType*>( inDerived.ptr );
				HYPERION_VERIFY( newObject != nullptr, "[Core] Failed to perform implicit cast?" );

				ptr = newObject;
				state = inDerived.state;

				_incRefAtomic();
				_checkConstruction();
			}
		}

		/*
		*	Destructor
		*/
		~HypPtr()
		{
			Clear();
		}

		/*
		*	Assignment Operator
		*/
		HypPtr< _ObjType >& operator=( const HypPtr< _ObjType >&inOther )
		{
			// If we currently have something were pointing to, we need to ensure the ref count gets updated
			if( state != nullptr )
			{
				Clear();
			}

			if( inOther._verifyContentsConst() )
			{
				ptr		= inOther.ptr;
				state	= inOther.state;

				_incRefAtomic();
			}
		}

		template< typename _OtherType,
			typename = typename std::enable_if< std::is_base_of< _ObjType, _OtherType  >::value >::type >
			HypPtr< _ObjType >& operator=( const HypPtr< _OtherType >& inOther )
		{
			// If we currently have something were pointing to, we need to decrement the ref counter
			if( state != nullptr )
			{
				Clear();
			}

			if( inOther._verifyContentsConst() )
			{
				auto newPtr = dynamic_cast<_ObjType*>( inOther.ptr );
				HYPERION_VERIFY( newPtr != nullptr, "[Core] Implicit cast assignment failed?" );

				ptr		= newPtr;
				state	= inOther.state;

				_incRefAtomic();
			}
		}

		/*
		*	bool Object:IsValid() const
		*	-  Checks to see if an object is being pointed to, and that object is valid
		*/
		bool IsValid() const
		{
			return _verifyContentsConst();
		}

		/*
		*	void Object::Clear()
		*	- Sets the pointer back to the null-state
		*/
		void Clear()
		{
			_decRefAtomic();

			state	= nullptr;
			ptr		= nullptr;
		}

		/*
		*	void Swap( const HypPtr< .. >& )
		*	- Swaps the contents of the two hyperion pointers
		*/
		void Swap( HypPtr< _ObjType >& inOther )
		{
			auto tmpState	= state;
			auto tmpPtr		= ptr;

			state	= inOther.state;
			ptr		= inOther.ptr;

			other.state		= tmpState;
			other.ptr		= tmpPtr;

			_checkConstruction();
			inOther._checkConstruction();
		}

		/*
		*	Member access oeprator
		*/
		_ObjType* operator->() const
		{
			HYPERION_VERIFY( ptr != nullptr, "Object being pointed to is no longer valid!" );
			return ptr;
		}

		/*
		*	Dereference operator
		*/
		_ObjType& operator*() const
		{
			return *( operator->() );
		}

		/*
		*	_ObjType* GetAddress() const
		*	- Gets a pointer to the object this HypPtr refers to
		*/
		_ObjType* GetAddress() const
		{
			return ptr;
		}

		/*
		*	Boolean conversion
		*/
		explicit operator bool() const
		{
			return _verifyContentsConst();
		}

		/*
		*	Equality Operators
		*/
		bool operator==( const HypPtr< _ObjType >& inOther ) const
		{
			bool bThisValid = IsValid();
			bool bOtherValid = inOther.IsValid();

			if( !bThisValid && !bOtherValid ) 
			{
				// Both are nullptr
				return true;
			}
			else if( bThisValid != bOtherValid )
			{
				// Only one pointer is non-null
				return false;
			}
			else
			{
				// Check object identifiers
				return ptr == inOther.ptr;
			}
		}

		bool operator!=( const HypPtr< _ObjType >& inOther ) const
		{
			return !( this->operator==( inOther ) );
		}

		bool operator==( nullptr_t ) const
		{
			return !_verifyContentsConst();
		}

		bool operator!=( nullptr_t ) const
		{
			return _verifyContentsConst();
		}

		template< typename _WeakObjType, typename _EIF >
		friend class HypWeakPtr;

	};


	/*======================================================================================================
	*	class HypWeakPtr
	*	- Weak reference to an object, supports multi-threading
	======================================================================================================*/
	template< typename _ObjType,
		typename = typename std::enable_if_t< std::is_base_of< Object, _ObjType >::value || std::is_same< Object, _ObjType >::value > >
	class HypWeakPtr
	{

	private:

		std::shared_ptr< _ObjectState > state;

		/*
		*	Private/Helper Functions
		*/
		bool _verifyContentsConst() const
		{
			return( state != nullptr && state->ptr.load() != nullptr && state->refcount.load() > 0 );
		}

		void _checkConstruction()
		{
			if( !_verifyContentsConst() )
			{
				state = nullptr;
			}
		}

	public:

		HypWeakPtr()
			: state( nullptr )
		{

		}

		HypWeakPtr( nullptr_t )
			: state( nullptr )
		{

		}

		HypWeakPtr( const HypPtr< _ObjType >& inStrongRef )
			: state( inStrongRef.state )
		{
			_checkConstruction();
		}

		HypWeakPtr( const HypWeakPtr< _ObjType >& inWeakRef )
			: state( inWeakRef.state )
		{
			_checkConstruction();
		}

		HypWeakPtr( const std::shared_ptr< _ObjectState >& inState )
			: state( inState )
		{
			_checkConstruction();
		}

		/*
		*	Implicit conversion between related pointer types
		*/
		template< typename _OtherType,
			typename = typename std::enable_if< std::is_base_of< _ObjType, _OtherType  >::value >::type >
		HypWeakPtr( const HypWeakPtr< _OtherType >& inDerived )
			: state( inDerived.state )
		{
			_checkConstruction();
		}

		template< typename _OtherType,
			typename = typename std::enable_if< std::is_base_of< _ObjType, _OtherType  >::value >::type >
		HypWeakPtr( const HypPtr< _OtherType >& inDerived )
			: state( inDerived.state )
		{
			_checkConstruction();
		}

		/*
		*	bool IsValid() const
		*	- Checks if we hold a weak ref to a valid object
		*/
		bool IsValid() const
		{
			return _verifyContentsConst();
		}

		/*
		*	void Clear()
		*	- Clears the contents of the weak ptr
		*/
		void Clear()
		{
			state = nullptr;
		}

		/*
		*	HypPtr< > AquirePtr()
		*	- Gets a strong ref to the object we refer to
		*/
		HypPtr< _ObjType > AquirePtr()
		{
			if( _verifyContentsConst() )
			{
				auto basePtr = state->ptr.load();
				if( basePtr == nullptr )
				{
					state = nullptr;
					return HypPtr< _ObjType >( nullptr );
				}

				auto* objPtr = dynamic_cast< _ObjType* >( basePtr );
				HYPERION_VERIFY( objPtr != nullptr, "[Core] Failed to aquire strong pointer to object, it couldnt be casted to weak ptr type?" );

				// Were going to incrememnt the ref counter atomically, ensure object is still live
				uint32 oldRefCount = state->refcount.fetch_add( 1 );
				if( oldRefCount <= 0 )
				{
					// The object is dead!
					state = nullptr;
					return HypPtr< _ObjType >( nullptr );
				}
				else
				{
					return HypPtr< _ObjType >( objPtr, state );
				}
			}
			else
			{
				return HypPtr< _ObjType >( nullptr );
			}
		}

		/*
		*	Assignment Operators
		*/
		HypWeakPtr< _ObjType >& operator=( const HypWeakPtr< _ObjType >& inOther )
		{
			state = inOther._verifyContentsConst() ? inOther.state : nullptr;
		}

		HypWeakPtr< _ObjType >& operator=( const HypPtr< _ObjType >& inOther )
		{
			state = inOther._verifyContentsConst() ? inOther.state : nullptr;
		}

		template< typename _OtherType,
			typename = typename std::enable_if< std::is_base_of< _ObjType, _OtherType  >::value >::type >
		HypWeakPtr< _ObjType >& operator=( const HypWeakPtr< _ObjType >& inOther )
		{
			state = inOther._verifyContentsConst() ? inOther.state : nullptr;
		}

		template< typename _OtherType,
			typename = typename std::enable_if< std::is_base_of< _ObjType, _OtherType  >::value >::type >
		HypWeakPtr< _ObjType >& operator=( const HypPtr< _ObjType >& inOther )
		{
			state = inOther._verifyContentsConst() ? inOther.state : nullptr;
		}

		template< typename _StrongObjType, typename _EIF >
		friend class HypPtr;
	};


	/*======================================================================================================
	*	class Object
	*	- Object base-class, derived by most classes in the engine
	*	- Allows for automatic memory managment, using garbage collection
	*	- Use with HypPtr & HypWeakPtr
	======================================================================================================*/
	class Object
	{

	private:

		std::shared_ptr< _ObjectState > m_ObjState;
		std::atomic< bool > m_bIsGarbage;
		std::atomic< GarbageCollectionMethod > m_gcMethod;

		/*
		*	Private/Helper Functions
		*/
		void SetupObject( std::shared_ptr< _ObjectState >& inState )
		{
			// This is called internally to set the objects state, and call the init hook
			m_ObjState = inState;
			Initialize();
		}

		void MarkAsGarbage()
		{
			HYPERION_VERIFY( m_ObjState != nullptr, "[Core] Object state was null when marked for garbage collection" );

			OnGarbageCollected();
			m_ObjState->refcount.store( 0 );
		}

		void DestroyObject()
		{
			HYPERION_VERIFY( m_ObjState != nullptr, "[Core] Object state was null on destruction!" );

			Shutdown();
			m_ObjState = nullptr;
		}

	protected:

		virtual void Initialize()
		{

		}

		virtual void Shutdown()
		{

		}

		virtual void OnGarbageCollected()
		{

		}

	public:

		/*
		*	Constructor
		*/
		Object()
			: m_ObjState( nullptr ), m_bIsGarbage( false )
		{

		}

		/*
		*	Destructor
		*/
		virtual ~Object()
		{
			HYPERION_VERIFY( m_ObjState == nullptr, "[Core] Object wasnt destroyed properly!" );
		}

		/*
		*	Member Functions
		*/
		bool IsGarbage() const
		{
			return m_bIsGarbage.load();
		}

		GarbageCollectionMethod GetGarbageCollectionMethod() const
		{
			return m_gcMethod.load();
		}

		void SetGarbageCollectionMethod( const GarbageCollectionMethod& in )
		{
			m_gcMethod.store( in );
		}

		/*
		*	HypPtr< T > AquireStrongPtr()
		*	- Can be called to get a strong ref to the object, if the ref-count of the object is zero..
		*		for example, if called from destructor or shutdown.. you get a null pointer
		*	- Also, if it isnt possible to cast to the template type, you also get a null pointer
		*/
		template< typename _ToType >
		HypPtr< _ToType >&& AquireStrongPtr()
		{
			// We need to ensure the ref count is valid before constructing the pointer
			auto basePtr = m_ObjState->ptr.load();

			if( m_ObjState != nullptr && basePtr != nullptr )
			{
				// Also, ensure we can cast to that type
				_ToType* newPtr = dynamic_cast<_ToType*>( basePtr );
				if( newPtr != nullptr )
				{
					uint32 oldRefCount = m_ObjState->refcount.fetch_add( 1 );
					if( oldRefCount > 0 )
					{
						return HypPtr< _ToType >( newPtr, m_ObjState );
					}
					else
					{
						m_ObjState.store( 0 );
					}
				}
			}

			return HypPtr< _ToType >( nullptr );
		}

		/*
		*	HypWeakPtr< T > AquireWeakPtr()
		*	- Can be called to get a weak ref to the object, if the ref-count of the object is zero..
		*		for example, if called from destructor or shutdown.. you get a null pointer
		*	- If the object cannot be casted to the template type, you also get a null ptr
		*/
		template< typename _ToType >
		HypWeakPtr< _ToType >&& AquireWeakPtr()
		{
			// We need to ensure the ref count is valid before constructing the pointer
			auto basePtr = m_ObjState->ptr.load();

			if( m_ObjState != nullptr && basePtr != nullptr; )
			{
				// Also, ensure we can cast to that type, although we dont use this pointer
				_ToType* newPtr = dynamic_cast<_ToType*>( m_ObjState->ptr );
				if( newPtr != nullptr )
				{
					if( m_ObjState->refcount.load() > 0 )
					{
						return HypWeakPtr< _ToType >( m_ObjState );
					}
				}
			}

			return HypWeakPtr< _ToType >( nullptr );
		}

		/*
		*	HypPtr< Type > GetType() const
		*	- Gets the type information for the top-level type of this object using the RTTI system
		*	- NOTE: Gets the TOP LEVEL TYPE! Not the type its currently being accessed through
		*	- Implemented in the .cpp file to avoid circular dependence with RTTI.h
		*/
		HypPtr< Type > GetType() const;

		template< typename _ObjType, class... Args >
		friend HypPtr< _ObjType > CreateObject( Args&& ... args );
		
		friend class GarbageCollector;
	};


	/*======================================================================================================
	*	function CreateObject
	*	- Creates an object of the specified type
	*	- The type MUST be derived from object and not Object itself
	*	- Takes a parameter list and calls a constructor of the specified type matching the arg list
	======================================================================================================*/
	template< typename _ObjType, class... Args >
	HypPtr< _ObjType > CreateObject( Args&& ... args )
	{
		static_assert( !std::is_same( _ObjType, Object )::value, "[Core] Cannot create a base 'Object'" );

		// Construct the object
		_ObjType* newObj = new _ObjType( std::forward< Args >( args ) ... );

		// Cast down to base-class
		Object* baseObj = dynamic_cast< Object* >( newObj );
		HYPERION_VERIFY( baseObj != nullptr, "[Core] Failed to create object!" );

		// Create our state
		auto objState = std::make_shared< _ObjectState >( baseObj, typeid( _ObjType ).hash_code() );
		
		// Initialize our object
		baseObj->SetupObject( objState );

		return HypPtr< _ObjType >( newObj, objState );
	}



	/*======================================================================================================
	*	class Type
	*	- Represents a registered 'Object' type
	*	- Can get from an object instance by calling 'GetType'
	*	- Is an object derived class as well (and you can get the type of 'Type')
	*	- All methods are thread-safe
	======================================================================================================*/
	class Type : public Object
	{

	private:

		const std::shared_ptr< RTTI::TypeInfo > m_TypeInfo;

	public:

		Type() = delete;
		Type( const std::shared_ptr< RTTI::TypeInfo >& inInfo )
			: m_TypeInfo( inInfo )
		{
			HYPERION_VERIFY( inInfo != nullptr, "Attempt to construct 'Type' with null info!" );
		}

		String GetTypeName() const
		{
			HYPERION_VERIFY( m_TypeInfo != nullptr, "[RTTI] Type info was null" );
			return m_TypeInfo->Name;
		}

		size_t GetTypeIdentifier() const
		{
			HYPERION_VERIFY( m_TypeInfo != nullptr, "[RTTI] Type info was null" );
			return m_TypeInfo->Identifier;
		}

		/*
		*	bool Type::IsParentTo( const HypPtr< Type >& )
		*	- Checks if this type is a parent to another type
		*	- IMPORTANT: This function checks if the other type is either derived from this class, or derived from a derived class (and so on)
		*/
		bool IsParentTo( const HypPtr< Type >& other ) const;


		template< typename _Ty, typename = typename std::enable_if< std::is_base_of< Object, _Ty >::value || std::is_same< Object, _Ty >::value >::type >
		bool IsParentTo() const
		{
			return IsParentTo( Type::Get< _Ty >() );
		}

		/*
		*	bool Type::IsDirectParentTo( const HypPtr< Type >& )
		*	- Checks if this type is the DIRECT parent of another type
		*	- Doesnt check if this is a 'grandparent' or 'great-grandparent' (etc..) class, only if its the direct parent
		*/
		bool IsDirectParentTo( const HypPtr< Type >& other ) const;

		template< typename _Ty, typename = typename std::enable_if< std::is_base_of< Object, _Ty >::value || std::is_same< Object, _Ty >::value >::type >
		bool IsDirectParentTo()
		{
			return IsDirectParentTo( Type::Get< _Ty >() );
		}

		/*
		*	bool IsDerivedFrom( const HypPtr< Type >& )
		*	- Checks if this type is derived from the other type in any way, wether the target type is a 'direct' parent class, or a 'distant' parent class
		*/
		bool IsDerivedFrom( const HypPtr< Type >& other ) const;

		template< typename _Ty, typename = typename std::enable_if< std::is_base_of< Object, _Ty >::value || std::is_same< Object, _Ty >::value >::type >
		bool IsDerivedFrom()
		{
			return IsDerivedFrom( Type::Get< _Ty >() );
		}

		/*
		*	bool IsDirectlyDerivedFrom( const HypPtr< Type >& )
		*	- Checks if tis type is immediatley derived from the target type. This function doesnt return true if the target is a 'grandparent' or 'great-grandparent' (etc..) class
		*/
		bool IsDirectlyDerivedFrom( const HypPtr< Type >& other ) const;

		template< typename T, typename = typename std::enable_if< std::is_base_of< Object, T >::value || std::is_same< Object, T >::value >::type >
		bool IsDirectlyDerivedFrom()
		{
			return IsDirectlyDerivedFrom( Type::Get< T >() );
		}


		/*
		*	std::vector< HypPtr< Type > > GetParentList() const;
		*/
		std::vector< HypPtr< Type > > GetParentList() const;

		/*
		*	HypPtr< Type > GetParent() const;
		*/
		HypPtr< Type > GetParent() const;

		/*
		*	std::vector< HypPtr< Type > > GetDirectChildren()
		*	- Gets a list of classes that are directly derived from this type
		*/
		std::vector< HypPtr< Type > > GetDirectChildren() const;

		/*
		*	std::vector< HypPtr< Type > > GetChildren() const
		*	- Gets a list of classes that are derived from this type, including classes derived from derived types, and so on..
		*/
		std::vector< HypPtr< Type > > GetChildren() const;


		/*
		*	HypPtr< Object > CreateInstance()
		*	- Creates an instance of this type
		*	- NOTE: Only works when the constructor has an overload with zero arguments
		*	- Returns the result as the base 'Object' type
		*/
		HypPtr< Object > CreateInstance() const;

		/*
		*	Static Methods
		*/
		static HypPtr< Type > Get( size_t inIdentifier );
		static HypPtr< Type > Get( const String& inName );
		static HypPtr< Type > Get( const HypPtr< Object >& inObj );

		template< typename T, typename = typename std::enable_if< std::is_base_of< Object, T >::value || std::is_same< Object, T >::value >::type >
		static HypPtr< Type > Get()
		{
			return Get( typeid( T ).hash_code() );
		}
		
		friend class Object;
	};

}
