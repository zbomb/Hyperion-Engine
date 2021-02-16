/*==================================================================================================
	Hyperion Engine
	Source/Core/Object.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/Object.h"


namespace Hyperion
{

	// DEBUG
	//std::map< size_t, std::shared_ptr< RTTI::TypeInfo > > g_TypeInfoList;

	std::map< uint32, std::shared_ptr< _ObjectState > > __objCache;
	uint32 __objIdCounter( 0 );

	void TickObjects( double inDelta )
	{
		for( auto It = __objCache.begin(); It != __objCache.end(); It++ )
		{
			// Validate object before ticking
			if( It->second && It->second->valid && It->second->ptr )
			{
				It->second->ptr->PerformTick( inDelta );
			}
		}
	}


	void TickObjectsInput( InputManager& im, double delta )
	{
		for( auto it = __objCache.begin(); it != __objCache.end(); it++ )
		{
			if( it->second && it->second->valid && it->second->ptr )
			{
				it->second->ptr->PerformInput( im, delta );
			}
		}
	}


	HypPtr< Type > Object::GetType() const
	{
		// First, we want to find the 'actual' type of this object, and not the type of pointer being used to refer to it
		// To do this, we store the type id on creation in the objects state info
		HYPERION_VERIFY( m_ThisState != nullptr, "Object is missing state info!" );

		auto typeInfo = RTTI::GetTypeInfo( m_ThisState->rtti_id );
		HYPERION_VERIFY( typeInfo != nullptr, "Couldnt find type info for object!" );

		return CreateObject< Type >( typeInfo );
	}



	/*------------------------------------------------------------------------------------------------------
		Type Class Definition
	------------------------------------------------------------------------------------------------------*/

	bool Type::IsParentTo( const HypPtr< Type >& Other ) const
	{
		if( !Other ) { return false; }
		HYPERION_VERIFY( m_Info && Other->m_Info, "[RTTI] Type class had invalid info pointer" );

		// Check if we are a parent to other

		// Object is a parent to every registered 'type'
		if( m_Info->Identifier == typeid( Object ).hash_code() ) { return true; }

		for( auto It = Other->m_Info->ParentChain.begin(); It != Other->m_Info->ParentChain.end(); It++ )
		{
			if( *It == m_Info->Identifier ) { return true; }
		}

		return false;
	}


	bool Type::IsDirectParentTo( const HypPtr< Type >& Other ) const
	{
		if( !Other ) { return false; }
		HYPERION_VERIFY( m_Info && Other->m_Info, "[RTTI] Type class had invalid info pointer" );

		// Check if we are direct parent to other

		// If an inheritance list is empty, it means its parent class is 'Object', so we check that case here
		if( Other->m_Info->ParentChain.size() == 0 && m_Info->Identifier == typeid( Object ).hash_code() ) { return true; }
		else return( *Other->m_Info->ParentChain.begin() == m_Info->Identifier );
	}


	bool Type::IsDerivedFrom( const HypPtr< Type >& Other ) const
	{
		if( !Other ) { return false; }
		HYPERION_VERIFY( m_Info && Other->m_Info, "[RTTI] Type class had invalid info pointer" );

		if( m_Info->Identifier == typeid( Object ).hash_code() ) { return false; }

		// Check if other is a parent to this
		// if 'Other' is an 'Object', then we must be derived from it
		if( typeid( Object ).hash_code() == Other->m_Info->Identifier ) { return true; }

		for( auto It = m_Info->ParentChain.begin(); It != m_Info->ParentChain.end(); It++ )
		{
			if( *It == Other->m_Info->Identifier ) { return true; }
		}

		return false;
	}


	bool Type::IsDirectlyDerivedFrom( const HypPtr< Type >& Other ) const
	{
		if( !Other ) { return false; }
		HYPERION_VERIFY( m_Info && Other->m_Info, "[RTTI] Type class had invalid info pointer" );

		if( m_Info->Identifier == typeid( Object ).hash_code() ) { return false; }

		// Check if other is a direct parent to this
		// If our inheritance list is empty, it means were derived directly from 'Object'
		if( m_Info->ParentChain.size() == 0 && Other->m_Info->Identifier == typeid( Object ).hash_code() ) { return true; }
		else return( *m_Info->ParentChain.begin() == Other->m_Info->Identifier );
	}


	std::vector< HypPtr< Type > > Type::GetParentList() const
	{
		HYPERION_VERIFY( m_Info != nullptr, "[RTTI] Type object was missing info pointer!" );

		std::vector< HypPtr< Type > > result;
		result.reserve( m_Info->ParentChain.size() + 1 );

		// First, loop through and add parent types in order (closest parent -> furthest parent)
		for( auto It = m_Info->ParentChain.begin(); It != m_Info->ParentChain.end(); It++ )
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
		HYPERION_VERIFY( m_Info != nullptr, "[RTTI] Type object was missing info pointer!" );

		if( m_Info->ParentChain.size() == 0 )
		{
			return CreateObject< Type >( RTTI::GetObjectType() );
		}
		else
		{
			auto parent_info = RTTI::GetTypeInfo( m_Info->ParentChain.front() );
			HYPERION_VERIFY( parent_info != nullptr, "[RTTI] Couldnt find type entry for parent" );

			return CreateObject< Type >( parent_info );
		}
	}


	std::vector< HypPtr< Type > > Type::GetDirectChildren() const
	{
		HYPERION_VERIFY( m_Info != nullptr, "[RTTI] Type object was missing info pointer!" );

		std::vector< HypPtr< Type > > output;
		output.reserve( m_Info->ChildrenList.size() );

		for( auto It = m_Info->ChildrenList.begin(); It != m_Info->ChildrenList.end(); It++ )
		{
			auto typePtr = CreateObject< Type >( RTTI::GetTypeInfo( *It ) );
			HYPERION_VERIFY( typePtr && typePtr->m_Info, "[RTTI] Failed to get child type instance!" );

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
		HYPERION_VERIFY( m_Info != nullptr, "[RTTI] Type object was missing info pointer!" );

		std::vector< HypPtr< Type > > output;

		// Loop through children and build list
		for( auto It = m_Info->ChildrenList.begin(); It != m_Info->ChildrenList.end(); It++ )
		{
			ProcessChildTreeNode( RTTI::GetTypeInfo( *It ), output );
		}

		return output;
	}


	HypPtr< Object > Type::CreateInstance() const
	{
		HYPERION_VERIFY( m_Info, "[RTTI] Type class had invalid info pointer" );
		return m_Info->CreateFunc ? m_Info->CreateFunc() : nullptr;
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