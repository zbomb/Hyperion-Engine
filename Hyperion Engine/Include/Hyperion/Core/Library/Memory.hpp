/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Library/Memory.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"

#include <unordered_map>
#include <atomic>
#include <memory>
#include <iostream>

// Debug Switch
//#define HYPERION_FLYWEIGHT_DEBUG

namespace std
{
	inline std::string to_string( std::string In ) { return In; }
}

namespace Hyperion
{
	template< typename _Ty >
	class _Impl_DefaultFlyweightCache
	{

	public:

		struct Instance
		{
			std::shared_ptr< const _Ty > m_Ref;
			std::atomic< uint32 > m_RefCount;

			inline std::shared_ptr< const _Ty > Get() { return m_Ref; }
		};

		static std::unordered_map< _Ty, std::shared_ptr< Instance > > m_Values;
		static std::shared_ptr< Instance > NewValue( const _Ty& In )
		{
			// Check if this value already exists
			auto Iter = m_Values.find( In );
			if( Iter == m_Values.end() )
			{
				// Create new instance for this value
				auto newInstance			= std::make_shared< Instance >();
				newInstance->m_Ref			= std::make_shared< const _Ty >( In );
				newInstance->m_RefCount		= 1;

				auto newPair = m_Values.emplace( In, newInstance );
#ifdef HYPERION_FLYWEIGHT_DEBUG
				std::cout << "[DEBUG] Flyweight: Created new instance for value!\n\t";
				std::cout << "Value: " << std::to_string( In ) << "\n\t";
				std::cout << "Type Count: " << m_Values.size() << "\n";
#endif
				return newPair.first->second;
			}
			else
			{
				if( Iter->second )
				{
					// Increment ref count
					Iter->second->m_RefCount++;

#ifdef HYPERION_FLYWEIGHT_DEBUG
					std::cout << "[DEBUG] Flyweight Cache: Found value in cache!\n\t";
					std::cout << "Value: " << std::to_string( In ) << "\n\t";
					std::cout << "Ref Count: " << Iter->second->m_RefCount << "\n\t";
					std::cout << "Type Count: " << m_Values.size() << "\n";
#endif
					return Iter->second;
				}
				else
				{
					std::cout << "[ERROR] Flyweight Cache: Found invalid entry in cache.. returning null!\n";
					return nullptr;
				}
			}
		}

		static void FreeValue( const std::shared_ptr< Instance >& In )
		{
			// Ensure we were passed a valid shared_ptr
			if( !In )
			{
#ifdef HYPERION_FLYWEIGHT_DEBUG
				std::cout << "[DEBUG] Flyweight Cache: Free called on null value.. might be intended\n";
#endif
				return;
			}

			// We want to operate on the map iterator, so first were going to search for that...
			auto Iter = m_Values.find( *In->Get() );
			if( Iter == m_Values.end() )
			{
				// Bad condition
				std::cout << "[ERROR] Flyweight Cache: Failed to free value.. couldnt find cache entry by key!\n";

				// Were still going to decrement the instance ref counter
				if( In->m_RefCount > 0 )
					In->m_RefCount--;
			}
			else
			{
				if( Iter->second )
				{
					if( Iter->second->m_RefCount <= 1 )
					{
#ifdef HYPERION_FLYWEIGHT_DEBUG
						std::cout << "[DEBUG] Flyweight Cache: Removing cached value.. ref count < 1\n\t";
						std::cout << "Value: " << std::to_string( *Iter->second->m_Ref ) << "\n\t";
						std::cout << "Ref Count: " << Iter->second->m_RefCount - 1 << "\n\t";
						std::cout << "Type Count: " << m_Values.size() << "\n";
#endif
						// Destroy instance
						m_Values.erase( Iter );
					}
					else
					{
						Iter->second->m_RefCount--;

#ifdef HYPERION_FLYWEIGHT_DEBUG
						std::cout << "[DEBUG] Flyweight Cache: Decrementing value ref count...\n\t";
						std::cout << "Value: " << std::to_string( *Iter->second->m_Ref.get() ) << "\n\t";
						std::cout << "Ref Count: " << Iter->second->m_RefCount << "\n\t";
						std::cout << "Type Count: " << m_Values.size() << "\n";
#endif

					}
				}
				else
				{
					// Bad Instance!
					std::cout << "[ERROR] Flyweight Cache: Failed to properly free value.. bad instance found... removing from cache!\n";
					m_Values.erase( Iter );
				}
			}
		}

		static std::shared_ptr< Instance > CopyValue( const std::shared_ptr< Instance >& In )
		{
			if( In )
				In->m_RefCount++;

			return In;
		}

#ifdef HYPERION_FLYWEIGHT_DEBUG
		static void DebugCache()
		{
			std::cout << "------------ FLYWEIGHT CACHE DEBUG BEGIN ------------\n";
			std::cout << "Value Count: " << m_Values.size() << "\n";
			std::cout << "\nValues:\n\t";

			for( auto It = m_Values.begin(); It != m_Values.end(); It++ )
			{
				std::cout << "----- Value ------\n\t";
				std::cout << std::to_string( *It->second->m_Ref.get() ) << "\n\t";
				std::cout << "Ref Count: " << It->second->m_RefCount << "\n\n\t";
			}

			std::cout << "\n------------ FLYWEIGHT CACHE DEBUG END ---------------\n";
		}
#endif

	};

	template< typename _Ty >
	std::unordered_map< _Ty, std::shared_ptr< typename _Impl_DefaultFlyweightCache< _Ty >::Instance > > _Impl_DefaultFlyweightCache< _Ty >::m_Values;

	template< typename _Ty, typename _Ch = _Impl_DefaultFlyweightCache< _Ty > >
	class Flyweight
	{

	private:

		std::shared_ptr< typename _Ch::Instance > m_Ref;

	public:

		Flyweight( const _Ty& In )
			: m_Ref( _Ch::NewValue( In ) )
		{
		}

		~Flyweight()
		{
			// Decrement ref counted instance
			if( m_Ref )
			{
				_Ch::FreeValue( m_Ref );
				m_Ref.reset();
			}
		}

		// Accessor Functions
		inline std::shared_ptr< const _Ty > GetShared() const	{ return m_Ref ? m_Ref->Get() : nullptr; }
		inline std::weak_ptr< const _Ty > GetWeak()	const		{ return std::weak_ptr< const _Ty >( GetShared() ); }
		inline bool IsValid() const								{ return m_Ref ? true : false; }
		inline const _Ty& GetRef() const						{ return *GetShared(); }
		inline _Ty GetCopy() const								{ return GetRef(); }

		// Operator Overloads
		inline operator bool() const			{ return IsValid(); }
		inline const _Ty& operator *() const	{ return GetRef(); }
		inline const _Ty* operator->() const	{ return GetShared().get(); }

		// Copy Constructor
		Flyweight( const Flyweight& Other )
			: m_Ref( _Ch::CopyValue( Other.m_Ref ) )
		{
		}

		// Assignment Operator
		void operator=( const Flyweight& Other )
		{
			if( m_Ref != Other.m_Ref )
			{
				if( m_Ref )
				{
					_Ch::FreeValue( m_Ref );
					m_Ref.reset();
				}

				if( Other.m_Ref )
				{
					m_Ref = _Ch::CopyValue( Other.m_Ref );
				}
			}
		}

		// Move Constructor
		Flyweight( Flyweight&& Other ) noexcept
			: m_Ref( nullptr )
		{
			m_Ref.swap( Other.m_Ref );
		}

		// Move Assignment
		Flyweight& operator=( Flyweight&& Other )
		{
			if( this != &Other )
			{
				// If we already have a value, we need to free it
				if( m_Ref )
				{
					_Ch::FreeValue( m_Ref );
					m_Ref.reset();
				}

				// Now, take the pointer from the other flyweight
				m_Ref.swap( Other.m_Ref );
			}

			return *this;
		}


	};

}