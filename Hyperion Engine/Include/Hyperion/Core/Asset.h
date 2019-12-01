/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Asset.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"


namespace Hyperion
{

	class Asset
	{

	public:

		virtual String GetAssetName() const = 0;

	};

	class GenericAsset : public Asset
	{

	public:

		GenericAsset()
		{}

		GenericAsset( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
			: m_Data( Begin, End )
		{}

		virtual String GetAssetName() const
		{
			return "#asset_generic";
		}

		std::vector< byte > m_Data;
	};

	struct AssetInstance
	{
		std::shared_ptr< Asset > m_Ref;
		std::atomic< uint32 > m_RefCount;
		std::atomic< bool > m_Cached;
	};

	struct GroupInstance
	{
		std::unordered_map< String, std::shared_ptr< AssetInstance > > m_Assets;
		std::atomic< bool > m_Locked;
		std::atomic< uint32 > m_RefAssetCount;
	};

	template< typename _Ty >
	class AssetRef
	{

	private:

		std::shared_ptr< _Ty > m_Ptr;
		std::shared_ptr< AssetInstance > m_Inst;

		/*
			Increment ref counter
		*/
		void _IncRefCount()
		{
			if( m_Inst )
			{
				m_Inst->m_RefCount++;
			}
		}

		/*
			Decrement ref counter
		*/
		void _DecRefCount( const std::shared_ptr< AssetInstance >& Target )
		{
			if( Target )
			{
				Target->m_RefCount--;
				// TODO: Check if we need to free this asset
			}
		}

		inline void _DecRefCount()
		{
			_DecRefCount( m_Inst );
		}


	public:

		/*
			Default constructor
		*/
		AssetRef()
			: AssetRef( nullptr )
		{}

		/*
			Convert from nullptr
		*/
		AssetRef( nullptr_t )
			: m_Ptr( nullptr ), m_Inst( nullptr )
		{}

		/*
			Construct from raw elements, handles ref counting properly
		*/
		AssetRef( const std::shared_ptr< _Ty >& inRef, const std::shared_ptr< AssetInstance >& inInst )
		{
			// Ensure both pointers are valid
			if( inRef && inInst )
			{
				// Assign our pointers
				m_Ptr	= inRef;
				m_Inst	= inInst;

				_IncRefCount();
			}
		}

		/*
			Copy constructor, increments ref counter
		*/
		AssetRef( const AssetRef& Other )
			: AssetRef( Other.m_Ptr, Other.m_Inst )
		{}

		/*
			Move constructor, doesnt increment ref counter
		*/
		AssetRef( AssetRef&& Other )
			: m_Ptr( std::move( Other.m_Ptr ) ), m_Inst( std::move( Other.m_Inst ) )
		{}

		/*
			Destructor
		*/
		~AssetRef()
		{
			Clear();
		}

		/*
			Clear, decrements ref counter and sets contents to null
		*/
		void Clear()
		{
			m_Ptr.reset();
			_DecRefCount();
			m_Inst.reset();
		}

		/*
			Assign, changes the referenced asset, similar to assignment operator
		*/
		void Assign( const std::shared_ptr< _Ty >& inRef, const std::shared_ptr< AssetInstance >& inInst )
		{
			// Copy the current instance pointer
			std::shared_ptr< AssetInstance > instCopy( m_Inst );

			// Assign the new pointers 
			if( inRef && inInst )
			{
				m_Ptr = inRef;
				m_Inst = inInst;

				_IncRefCount();
			}
			else
			{
				m_Ptr.reset();
				m_Inst.reset();
			}

			// Decrement old ref counter
			if( instCopy )
			{
				_DecRefCount( instCopy );
			}
		}

		/*
			Assignment Operator
		*/
		AssetRef& operator=( const AssetRef& Other )
		{
			// First, copy the current instance pointer
			std::shared_ptr< AssetInstance > instCopy( m_Inst );

			// Next, copy the pointer and instance in from the other assetref
			if( Other.m_Ptr && Other.m_Inst )
			{
				m_Ptr	= Other.m_Ptr;
				m_Inst	= Other.m_Inst;

				_IncRefCount();
			}
			else
			{
				m_Ptr.reset();
				m_Inst.reset();
			}

			// Use the copy of the old instance pointer to decrement ref counter for old data
			if( instCopy )
			{
				_DecRefCount( instCopy );
			}

			return *this;
		}

		/*
			Nullptr Assignment Overload
		*/
		AssetRef& operator=( nullptr_t )
		{
			Clear();
		}

		/*
			IsValid, requires both components to be valid
		*/
		bool IsValid() const
		{
			return m_Ptr && m_Inst;
		}

		/*
			Equality Operators
		*/
		bool operator==( const AssetRef& Other ) const
		{
			bool lhsValid = IsValid();
			bool rhsValid = Other.IsValid();

			if( !lhsValid && !rhsValid ) return true;
			else if( lhsValid != rhsValid ) return false;
			else
			{
				return m_Ptr == Other.m_Ptr && m_Inst == Other.m_Inst;
			}
		}

		bool operator!=( const AssetRef& Other ) const
		{
			return !( *this == Other );
		}

		/*
			Pointer accessor
		*/
		_Ty* operator->()
		{
			if( m_Ptr && m_Inst )
			{
				return m_Ptr.get();
			}

			return nullptr; // Throws exception!
		}

		/*
			Reference accessor
		*/
		_Ty& operator*()
		{
			return *( operator->() );
		}

		/*
			Boolean conversion
		*/
		inline operator bool() const
		{
			return IsValid();
		}



	};

	
	/*
		AssetRef Casting
	*/
	template< typename _To, typename _From >
	AssetRef< _To > AssetCast( const AssetRef< _From >& Target )
	{
		// Check if source is valid, and attempt to get the base ptr from the instance data
		// Then use a dynamic cast to get the desired type, if we cant, return null
		auto ptr = Target.IsValid() ? Target.m_Inst->m_Ptr : nullptr;
		auto casted_ptr = ptr ? std::dynamic_pointer_cast< _To >( ptr ) : nullptr;
		if( !ptr )
		{
			return AssetRef< _To >( nullptr );
		}

		// Construct result, constructor will perform ref counting
		return AssetRef< _To >( casted_ptr, Target.m_Inst );
	}

}