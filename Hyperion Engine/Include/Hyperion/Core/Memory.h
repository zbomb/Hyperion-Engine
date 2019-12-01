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
			return obj.get();
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

		Nullable& operator=( nullptr_t )
		{
			obj.reset();
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

}