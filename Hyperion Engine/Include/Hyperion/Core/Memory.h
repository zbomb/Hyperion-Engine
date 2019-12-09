/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Memory.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include <memory>

namespace Hyperion
{

	template< typename T >
	class Nullable
	{

	private:

		std::unique_ptr< T > obj;

	public:

		Nullable()
			: obj( std::make_unique< T >() )
		{}

		Nullable( nullptr_t )
			: obj( nullptr )
		{}

		Nullable( const T& In )
			: obj( In )
		{}

		Nullable( T&& In )
			: obj( std::move( In ) )
		{}

		Nullable( const Nullable& Other )
			: obj( Other ? Other.Get() : nullptr )
		{}

		~Nullable()
		{
			obj.reset();
		}

		operator bool() const
		{
			return obj ? true : false;
		}

		T& Get()
		{
			return *obj.get();
		}

		Nullable& operator=( const Nullable& Other )
		{
			obj.reset();

			if( Other )
			{
				obj = std::make_unique< T >( Other.Get() );
			}

			return *this;
		}

		Nullable& operator=( const T& In )
		{
			obj.reset();
			obj = std::make_unique< T >( In );
			return *this;
		}

		Nullable& operator=( nullptr_t )
		{
			obj.reset();
			return *this;
		}

		T& operator->()
		{
			return Get();
		}

		T& operator*()
		{
			return Get();
		}

		T* operator&()
		{
			return obj.get();
		}

		bool IsNull() const
		{
			return obj ? true : false;
		}
	};

	template< typename R >
	class Nullable< R& >
	{

	private:

		R* ref;
		bool valid;

	public:

		Nullable()
			: valid( false ), ref( nullptr )
		{}

		Nullable( nullptr_t )
			: valid( false ), ref( nullptr )
		{}

		Nullable( R& In )
			: ref( std::addressof( In ) ), valid( true )
		{}

		Nullable( R&& In ) = delete;

		Nullable( const Nullable& Other )
			: ref( Other.ref ), valid( Other.valid )
		{}

		~Nullable()
		{
			ref		= nullptr;
			valid	= false;
		}

		operator bool() const
		{
			return valid && ref;
		}

		R& Get()
		{
			return *ref;
		}

		Nullable& operator=( const Nullable& Other )
		{
			ref		= Other.ref;
			valid	= Other.valid;

			return *this;
		}

		Nullable& operator=( nullptr_t )
		{
			valid	= false;
			ref		= nullptr;

			return *this;
		}

		Nullable& operator=( R& In )
		{
			ref		= std::addressof( In );
			valid	= true;

			return *this;
		}

		R& operator->()
		{
			return *ref;
		}

		R& operator*()
		{
			return *ref;
		}

		R* operator&()
		{
			return ref;
		}

		bool IsNull() const
		{
			return !valid || !ref;
		}

	};

}