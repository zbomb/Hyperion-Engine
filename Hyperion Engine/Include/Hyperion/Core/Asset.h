/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Asset.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"

#include <condition_variable>

namespace Hyperion
{
	enum class AssetLocation
	{
		FileSystem = 0,
		Virtual = 1
	};

	class AssetPath
	{

		String m_Path;
		AssetLocation m_Location;

	public:

		AssetPath()
			: m_Path(), m_Location( AssetLocation::FileSystem )
		{}

		AssetPath( const String& inPath, AssetLocation inLocation )
			: m_Path( inPath ), m_Location( inLocation )
		{}

		AssetPath( const AssetPath& inOther )
			: m_Path( inOther.m_Path ), m_Location( inOther.m_Location )
		{}

		AssetPath( AssetPath&& inOther )
			: m_Path( std::move( inOther.m_Path ) ), m_Location( std::move( inOther.m_Location ) )
		{}

		AssetPath& operator=( const AssetPath& inOther )
		{
			m_Path = inOther.m_Path;
			m_Location = inOther.m_Location;

			return *this;
		}

		AssetPath& operator=( AssetPath&& inOther )
		{
			m_Path = std::move( inOther.m_Path );
			m_Location = std::move( inOther.m_Location );

			return *this;
		}

		inline bool IsValid() const { return !m_Path.IsWhitespaceOrEmpty(); }
		inline String GetPath() const { return m_Path; }
		inline AssetLocation GetLocation() const { return m_Location; }

		void Reset()
		{
			m_Path.Clear();
			m_Location = AssetLocation::FileSystem;
		}

	};

	class Asset
	{

	protected:

		AssetPath m_Path;

	public:

		virtual String GetAssetType() const = 0;
		inline AssetPath GetAssetPath() const { return m_Path; }

		friend class AssetLoader;
		friend class AssetManager;
	};

	class GenericAsset : public Asset
	{

	public:

		GenericAsset()
		{}

		GenericAsset( std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
			: m_Data( Begin, End )
		{}

		virtual String GetAssetType() const
		{
			return "GenericAsset";
		}

		std::vector< byte > m_Data;
	};

	struct AssetInstance
	{
		std::shared_ptr< Asset > m_Ref;
		std::atomic< int > m_RefCount;
		bool m_Cached;
		AssetLocation m_Location;

		std::condition_variable_any m_Wait;
		bool m_Loaded;
	};

	struct GroupInstance
	{
		std::unordered_map< String, std::shared_ptr< AssetInstance > > m_Assets;
		std::atomic< bool > m_Locked;
		std::atomic< uint32 > m_RefAssetCount;
	};

	class AssetRefBase
	{

	protected:

		std::shared_ptr< AssetInstance > m_Inst;

		void _IncRefCount();
		void _DecRefCount( const std::shared_ptr< AssetInstance >& Target );
		inline void _DecRefCount() { _DecRefCount( m_Inst ); }

		AssetRefBase()
			: m_Inst( nullptr )
		{}

		AssetRefBase( const std::shared_ptr< AssetInstance >& inInst )
			: m_Inst( inInst )
		{}

		AssetRefBase( std::shared_ptr< AssetInstance >&& inInst )
			: m_Inst( std::move( inInst ) )
		{}
	};

	template< typename _Ty >
	class AssetRef : public AssetRefBase
	{

	private:

		std::shared_ptr< _Ty > m_Ptr;
		AssetPath m_Path;

	public:

		/*
			Default constructor
		*/
		AssetRef()
			: AssetRef( nullptr )
		{
		}

		/*
			Convert from nullptr
		*/
		AssetRef( nullptr_t )
			: AssetRefBase(), m_Ptr( nullptr ), m_Identifier( nullptr )
		{}

		/*
			Construct from raw elements, handles ref counting properly
		*/
		AssetRef( const std::shared_ptr< _Ty >& inRef, const std::shared_ptr< AssetInstance >& inInst, const AssetPath& inPath )
			: m_Path( inPath )
		{
			// Ensure both pointers are valid
			if( inRef && inInst )
			{
				// Assign our pointers
				m_Ptr			= inRef;
				m_Inst			= inInst;

				_IncRefCount();
			}
		}

		/*
			Copy constructor, increments ref counter
		*/
		AssetRef( const AssetRef& Other )
			: AssetRef( Other.m_Ptr, Other.m_Inst, Other.m_Path )
		{
		}

		/*
			Move constructor, doesnt increment ref counter
		*/
		AssetRef( AssetRef&& Other ) noexcept
			: AssetRefBase( std::move( Other.m_Inst ) ), m_Ptr( std::move( Other.m_Ptr ) ), m_Path( std::move( Other.m_Path ) )
		{
		}

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
			m_Path.Reset();
		}

		/*
			Assign, changes the referenced asset, similar to assignment operator
		*/
		void Assign( const std::shared_ptr< _Ty >& inRef, const std::shared_ptr< AssetInstance >& inInst, const AssetPath& inPath )
		{
			// Copy the current instance pointer
			std::shared_ptr< AssetInstance > instCopy( m_Inst );

			m_Path = inPath;

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

			m_Path = Other.m_Path;

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

		AssetRef& operator=( AssetRef&& Other )
		{
			std::shared_ptr< AssetInstance > instCopy( m_Inst );

			m_Path = std::move( Other.m_Path );

			if( Other.m_Ptr && Other.m_Inst )
			{
				m_Ptr = std::move( Other.m_Ptr );
				m_Inst = std::move( Other.m_Inst );
			}
			else
			{
				m_Ptr.reset();
				m_Inst.reset();
			}

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
				return m_Inst == Other.m_Inst;
			}
		}

		bool operator!=( const AssetRef& Other ) const
		{
			return !( *this == Other );
		}

		/*
			Pointer accessor
		*/
		_Ty* operator->() const noexcept
		{
			if( m_Ptr && m_Inst )
			{
				return m_Ptr.get();
			}

			return nullptr;
		}

		/*
			Reference accessor
		*/
		_Ty& operator*() const noexcept
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

		inline AssetPath GetPath() const
		{
			return m_Path;
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