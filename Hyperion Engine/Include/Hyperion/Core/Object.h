/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/ObjectBase.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"

#include <chrono>
#include <memory>
#include <map>

#undef min

/*--------------------------------------------------------------------
	Updated Object System
--------------------------------------------------------------------*/

namespace Hyperion
{
	class Object;
	class InputManager;

	/*
		Try to forward declare the destroy object function so we can call it from HypPtr
	*/
	template< typename _Ty >
	class HypPtr;

	template< typename _Ty >
	void DestroyObject( HypPtr< _Ty >& );


	struct _ObjectState
	{
		Object* ptr;
		uint32 refcount;
		bool valid;
		bool shutdown_started;

		_ObjectState() = delete;
		_ObjectState( Object* inPtr )
			: ptr( inPtr ), refcount( 0 ), valid( true ), shutdown_started( false )
		{
			HYPERION_VERIFY( inPtr != nullptr, "Cant create object meta state with null ptr!" );
		}

		~_ObjectState()
		{
			HYPERION_VERIFY( ptr == nullptr, "Object state went out of scope without the contained object being destroyed!" );

			ptr			= nullptr;
			valid		= false;
			refcount	= 0;
		}
	};

	template< typename _Ty >
	class HypPtr
	{

	private:

		_Ty* ptr;
		std::shared_ptr< _ObjectState > state;
		uint32 id;

		void _IncRefCount()
		{
			if( state )
			{
				state->refcount++;
			}
		}

		void _DecRefCount( std::shared_ptr< _ObjectState >& inState )
		{
			if( inState )
			{
				uint32 cur_count = inState->refcount;
				inState->refcount = cur_count > 1 ? cur_count - 1 : 0;

				if( inState->refcount <= 0 )
				{
					if( !inState->shutdown_started )
					{
						DestroyObject( *this );
					}
				}
			}
		}

		void _DecRefCount()
		{
			_DecRefCount( state );
		}

		bool _VerifyState( const std::shared_ptr< _ObjectState >& inState )
		{
			return( inState && inState->valid && inState->ptr );
		}

		void _ValidateConstruction( bool bIncRefCount )
		{
			if( !ptr || !_VerifyState( state ) )
			{
				ptr		= nullptr;
				state	= nullptr;
			}
			else if( bIncRefCount )
			{
				_IncRefCount();
			}
		}

		bool _VerifyContents()
		{
			if( !ptr )
			{
				return false;
			}
			else if( !state )
			{
				ptr = nullptr;
				return false;
			}
			else if( !state->valid )
			{
				ptr		= nullptr;
				state	= nullptr;

				return false;
			}
			else return true;
		}

		bool _VerifyContents_Const() const
		{
			return( ptr && state && state->valid );
		}

	public:

		HypPtr()
			: ptr( nullptr ), state( nullptr ), id( 0 )
		{}

		HypPtr( nullptr_t )
			: HypPtr()
		{}

		HypPtr( _Ty* inPtr, std::shared_ptr< _ObjectState >& inState, uint32 inId )
			: ptr( inPtr ), state( inState ), id( inId )
		{
			_ValidateConstruction( true );
		}

		HypPtr( const HypPtr& inOther )
			: ptr( inOther.ptr ), state( inOther.state ), id( inOther.id )
		{
			_ValidateConstruction( true );
		}

		HypPtr( HypPtr&& inOther ) noexcept
			: ptr( std::move( inOther.ptr ) ), state( std::move( inOther.state ) ), id( std::move( inOther.id ) )
		{
			inOther.ptr		= nullptr;
			inOther.state	= nullptr;
			inOther.id		= 0;

			_ValidateConstruction( false );
		}

		template< typename _Ty2,
			typename = typename std::enable_if< std::is_base_of< _Ty, _Ty2 >::value >::type >
		HypPtr( const HypPtr< _Ty2 >& inDerived )
		{
			if( !inDerived.IsValid() )
			{
				ptr		= nullptr;
				state	= nullptr;
				id		= 0;
			}
			else
			{
				_Ty* myPtr = dynamic_cast< _Ty* >( inDerived.__GetAddress() );
				HYPERION_VERIFY( myPtr != nullptr, "Failed to downcast derived pointer!" );

				ptr		= myPtr;
				state	= inDerived.__GetState_Const();
				id		= inDerived.__GetIdentifier();

				_ValidateConstruction( true );
			}
		}

		~HypPtr()
		{
			Clear();
		}

		inline std::shared_ptr< _ObjectState >& __GetState() { return state; }
		inline const std::shared_ptr< _ObjectState >& __GetState_Const() const { return state; }
		inline uint32 __GetIdentifier() const { return id; }
		inline _Ty* __GetAddress() const { return ptr; }

		HypPtr& operator=( const HypPtr& Other )
		{
			auto old_state = state;

			if( Other.ptr && _VerifyState( Other.state ) )
			{
				ptr		= Other.ptr;
				state	= Other.state;
				id		= Other.id;

				_IncRefCount();
			}
			else
			{
				ptr		= nullptr;
				state	= nullptr;
				id		= 0;
			}

			if( old_state )
			{
				_DecRefCount( old_state );
			}

			return *this;
		}

		HypPtr& operator=( HypPtr&& Other )
		{
			auto old_state = state;

			if( Other.ptr && _VerifyState( Other.state ) )
			{
				ptr		= std::move( Other.ptr );
				state	= std::move( Other.state );
				id		= std::move( Other.id );
			}
			else
			{
				ptr		= nullptr;
				state	= nullptr;
				id		= 0;
			}

			Other.ptr		= nullptr;
			Other.state		= nullptr;
			Other.id		= 0;

			if( old_state )
			{
				_DecRefCount( old_state );
			}

			return *this;
		}

		void Clear()
		{
			_DecRefCount();

			ptr		= nullptr;
			state	= nullptr;
			id		= 0;
		}

		void Swap( HypPtr& Other )
		{
			auto old_ptr	= ptr;
			auto old_state	= state;
			auto old_id		= id;

			ptr		= Other.ptr;
			state	= Other.state;
			id		= Other.id;

			Other.ptr		= old_ptr;
			Other.state		= old_state;
			Other.id		= old_id;

			_ValidateConstruction( false );
			Other._ValidateConstruction( false );
		}

		_Ty* operator->() const
		{
			HYPERION_VERIFY( _VerifyContents_Const(), "Object being pointed to is no longer valid!" );
			return ptr;
		}

		_Ty& operator*() const
		{
			return *( operator->() );
		}

		_Ty* GetAddress() const
		{
			return ptr;
		}

		explicit operator bool() const
		{
			return _VerifyContents_Const();
		}

		bool IsValid() const
		{
			return _VerifyContents_Const();
		}

		uint32 GetIdentifier() const
		{
			return IsValid() ? id : 0;
		}

		// Equality checks
		bool operator==( const HypPtr& inOther ) const
		{
			bool thisValid		= _VerifyContents_Const();
			bool otherValid		= inOther._VerifyContents_Const();

			if( !thisValid && !otherValid ) return true;
			else if( thisValid != otherValid ) return false;
			else // thisValid && otherValid
			{
				// Lets check if the object identifiers are the same
				return id == inOther.id;
			}
		}

		bool operator==( nullptr_t ) const
		{
			return !_VerifyContents_Const();
		}

		bool operator!=( const HypPtr& inOther ) const
		{
			return !( operator==( inOther ) );
		}

		bool operator!=( nullptr_t ) const
		{
			return _VerifyContents_Const();
		}

		/*
			HypPtr Casting
			- We declare it here using 'friend', but its not actually a class member function
			- Its contained directly in the Hyperion namespace
			- We do this because of how tricky template friend functions inside template classes are
		*/

		/*
		template< typename _To, typename _From >
		friend HypPtr< _To > CastPtr( const HypPtr< _From >& inPtr )
		{
			// First, check if the source pointer is invalid
			if( !inPtr._VerifyContents_Const() )
			{
				return nullptr;
			}

			// Attempt to cast the pointer (dynamically) to the desired type
			_To* casted = dynamic_cast< _To* >( inPtr.ptr );
			if( !casted )
			{
				return nullptr;
			}

			// This construction will incremement the ref count properly
			return HypPtr< _To >( casted, inPtr.state );
		}
		*/

	};

	class Object
	{

		/*--------------------------------------------------------------------------------
			Non-Static Members and Functions
		--------------------------------------------------------------------------------*/
	private:

		uint32 m_Identifier;
		std::chrono::time_point< std::chrono::high_resolution_clock > m_LastTick;
		bool m_IsValid;
		std::shared_ptr< _ObjectState > m_ThisState;

		void PerformTick()
		{
			if( bRequiresTick )
			{
				// Check if this is the first tick, if so, then delta will be 0
				double Delta = 0.0;
				auto Now = std::chrono::high_resolution_clock::now();

				// Calculate the delta since last tick
				if( m_LastTick != std::chrono::time_point< std::chrono::high_resolution_clock >::min() )
				{
					std::chrono::duration< double > duration = Now - m_LastTick;
					Delta = duration.count();
				}

				// Call Tick
				Tick( Delta );

				// Update last tick
				m_LastTick = Now;
			}
		}

		void PerformInitialize()
		{
			HYPERION_VERIFY( m_Identifier != 0 && m_ThisState && m_IsValid, "Attempt to initialize object with invalid state" );

			m_IsValid	= true;
			m_LastTick	= std::chrono::high_resolution_clock::now();

			Initialize();
		}

		void PerformShutdown()
		{
			Shutdown();

			m_IsValid		= false;
			bRequiresTick	= false;
		}

	protected:

		bool bRequiresTick;

		virtual void Initialize()
		{
		}

		virtual void Shutdown()
		{
		}

		virtual void Tick( double Delta )
		{
		}

	public:

		Object()
			: bRequiresTick( false ), m_IsValid( true ), m_LastTick( std::chrono::high_resolution_clock::now() ), m_Identifier( 0 )
		{}

		virtual ~Object()
		{
			// Ensure the Shutdown function was ran
			HYPERION_VERIFY( !m_IsValid, "It appears the object Shutdown function wasnt called before the destructor!" );
		}

		inline uint32 GetIdentifier() const		{ return m_Identifier; }
		inline auto GetLastTick() const			{ return m_LastTick; }
		inline bool IsValid() const				{ return m_IsValid; }
		inline bool RequiresTick() const		{ return bRequiresTick; }

		template< typename _To >
		HypPtr< _To > AquirePointer()
		{
			// Were going to attempt to build a HypPtr for this object, casted to the desired type
			// So first, check if we have the meta state we need and if this object is valid
			if( !m_IsValid || !m_ThisState || !m_ThisState->valid )
				return nullptr;

			_To* casted = dynamic_cast< _To* >( this );
			if( !casted )
				return nullptr;

			return HypPtr< _To >( casted, m_ThisState, m_Identifier );
		}

		virtual String GetDebugName() const
		{
			return "unknown_object";
		}

		/*
			Friend in the create/destroy functions
		*/
		template< typename _Ty, class... Args >
		friend HypPtr< _Ty > CreateObject( Args&& ... args );

		template< typename _TTy >
		friend void DestroyObject( HypPtr< _TTy >& inPtr );

		friend void TickObjects();
	};

	extern std::map< uint32, std::shared_ptr< _ObjectState > > __objCache;
	extern uint32 __objIdCounter;

	template< typename _Ty, class... Args >
	HypPtr< _Ty > CreateObject( Args&& ... args )
	{
		// Validate template argument
		static_assert( std::is_base_of< Object, _Ty >::value && !std::is_same< Object, _Ty >::value, "Can only use this function to create object derived classes" );

		// Generate identifier
		auto newId = ++__objIdCounter;

		// Construct Object
		_Ty* newObj			= new _Ty( std::forward< Args >( args ) ... );
		Object* baseObj		= dynamic_cast< Object* >( newObj );

		HYPERION_VERIFY( baseObj != nullptr, "Couldnt cast new object back to base type?" );

		// Create shared state to hold object and do ref counting
		auto state = std::make_shared< _ObjectState >( baseObj );
		__objCache[ newId ] = state;

		// Initialize
		baseObj->m_ThisState = state;
		baseObj->m_Identifier = newId;
		baseObj->PerformInitialize();

		return HypPtr< _Ty >( newObj, state, newId );
	}

	template< typename _TTy >
	void DestroyObject( HypPtr< _TTy >& inPtr )
	{
		auto state = inPtr.__GetState();
		if( state && state->valid && !state->shutdown_started )
		{
			auto id = inPtr.GetIdentifier();
			state->shutdown_started = true;

			Object* target	= state->ptr;

			HYPERION_VERIFY( target, "Attempt to destroy invalid object pointer.. state was valid.. but pointer was null" );

			target->PerformShutdown();
			delete target;

			state->ptr		= nullptr;
			state->valid	= false;

			inPtr.Clear();

			auto cacheEntry = __objCache.find( id );
			if( cacheEntry == __objCache.end() )
			{
				std::cout << "[ERROR] Object System: Couldnt find cache entry for object (id: " << id << ") when destroying!\n";
			}
			else
			{
				__objCache.erase( cacheEntry );
			}
		}
	}

	void TickObjects();

}
