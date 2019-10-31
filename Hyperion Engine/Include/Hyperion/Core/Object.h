/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Object.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"

#include <chrono>
#include <memory>

/*
	Cache Grouping Macro
*/
#define HYPERION_GROUP_OBJECT( objGroup ) \
inline static Hyperion::ObjectCacheID GetCacheGroup() \
{ \
	return objGroup; \
}

#define HYPERION_GROUP_OBJECT_DEFAULT() \
inline static Hyperion::ObjectCacheID GetCacheGroup() \
{ \
	return Hyperion::CACHE_NONE; \
}

namespace Hyperion
{
	class InputManager;

	class Object
	{

	private:

		// Private Data Members
		ObjectID m_Identifier;
		size_t m_TypeHash;
		bool m_IsValid;
		std::chrono::time_point< std::chrono::high_resolution_clock > m_LastTick;
		std::weak_ptr< Object > m_WeakThis;

		void PerformObjectShutdown();
		void PerformObjectInit();
		void PerformObjectTick();

	protected:

		// Protected Data Members
		bool b_RequiresTick;
		virtual void Tick( double Delta );

		inline std::weak_ptr< Object > GetWeakPtr() { return m_WeakThis; }

		template< typename T >
		std::weak_ptr< T > GetWeakPtr()
		{
			// Since we cant directly get a weak_ptr casted to the derived class type, were just going to instead,
			// have it so we lock the weak_ptr, cast up to the current class type, and then get a weak_ptr from that
			auto sharedThis = m_WeakThis.lock();
			if( !sharedThis )
				return std::weak_ptr< T >();

			return std::weak_ptr< T >( std::dynamic_pointer_cast< T >( sharedThis ) );
		}

	public:

		Object();
		~Object();

		virtual void Initialize();
		virtual void Shutdown();

		virtual void BindUserInput( InputManager* );

		/*
			Inline Getters
		*/
		inline ObjectID GetID() const	{ return m_Identifier; }
		inline bool IsValid() const		{ return m_IsValid; }

		/*
			Object Grouping
		*/
		HYPERION_GROUP_OBJECT( CACHE_NONE );

		friend class Engine;

	};

}
