/*==================================================================================================
	Hyperion Engine
	Source/Core/RTTI.cpp
	© 2020, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/RTTI.h"
#include "Hyperion/Core/Object.h"


namespace Hyperion
{

	//std::map< size_t, std::shared_ptr< RTTI::TypeInfo > > g_TypeInfoList;

	std::shared_ptr< RTTI::TypeInfo > RTTI::m_ObjectTypeInfo( 
		std::make_shared< RTTI::TypeInfo >( typeid( Object ).hash_code(), String( HYPERION_OBJECT_TYPE_NAME ), nullptr, std::vector< size_t >() ) 
	);

	std::map< size_t, std::shared_ptr< RTTI::TypeInfo > >& getTypeMap()
	{
		static std::map< size_t, std::shared_ptr< RTTI::TypeInfo > > ret;
		return ret;
	}

	RTTI::RegistryEntry::RegistryEntry( size_t inIdentifier, const String& inName, size_t inParent, std::function< HypPtr< Object >() > inCreateFunc )
	{
		bool bResult = RTTI::RegisterType( inIdentifier, inName, inCreateFunc, inParent );
		HYPERION_VERIFY( bResult, "Failed to register a type!!" );
	}


	bool RTTI::TypeExists( size_t inIdentifier )
	{
		if( inIdentifier == typeid( Object ).hash_code() ) { return true; }

		auto& list = getTypeMap();
		return list.find( inIdentifier ) != list.end();

		//return g_TypeInfoList.find( inIdentifier ) != g_TypeInfoList.end();
	}


	bool RTTI::TypeExists( const String& inName )
	{
		if( inName.IsWhitespaceOrEmpty() ) { return false; }
		String lowerName = inName.ToLower(); // Names are stored as lowercase only

		static String objectName	= String::ToLower( HYPERION_OBJECT_TYPE_NAME );

		if( String::Equals( lowerName, objectName ) ) { return true; }
		auto& list = getTypeMap();

		for( auto It = list.begin(); It != list.end(); )
		{
			HYPERION_VERIFY( It->second, "[RTTI] Found null entry in type list!" );

			if( String::Equals( lowerName, It->second->Name ) ) { return true; }
			It++;
		}

		return false;
	}


	std::shared_ptr< RTTI::TypeInfo > RTTI::GetTypeInfo( size_t inIdentifier )
	{
		// Ensure we have initialized
		Initialize();

		// Check if were attempting to get the 'Object' type
		static size_t objectTypeId = typeid( Object ).hash_code();
		if( inIdentifier == objectTypeId )
		{
			return m_ObjectTypeInfo;
		}
		auto& list = getTypeMap();

		auto entry = list.find( inIdentifier );
		if( entry == list.end() ) { return nullptr; }

		HYPERION_VERIFY( entry->second, "[RTTI] Found requested type entry.. but it was null!" );

		return entry->second;
	}


	std::shared_ptr< RTTI::TypeInfo > RTTI::GetTypeInfo( const String& inName )
	{
		// Ensure we have initialized
		Initialize();

		if( inName.IsWhitespaceOrEmpty() ) { return nullptr; }

		String lowerName			= inName.ToLower();
		static String objectName	= String::ToLower( HYPERION_OBJECT_TYPE_NAME );

		if( String::Equals( lowerName, objectName ) )
		{
			return m_ObjectTypeInfo;
		}

		auto& list = getTypeMap();

		for( auto It = list.begin(); It != list.end(); It++ )
		{
			HYPERION_VERIFY( It->second, "[RTTI] Found a null entry in the type info list!" );

			if( String::Equals( lowerName, It->second->Name ) )
			{
				return It->second;
			}
		}

		return nullptr;
	}


	bool RTTI::RegisterType( size_t inIdentifier, const String& inName, std::function< HypPtr< Object >() > inCreateFunc, size_t inParent )
	{
		// Check the identifier
		if( inIdentifier == typeid( Object ).hash_code() )
		{
			Console::WriteLine( "[ERROR] RTTI: Failed to register type.. the identifier was the one assigned to the base object class" );
			return false;
		}

		// Check the name
		if( inName.IsWhitespaceOrEmpty() )
		{
			Console::WriteLine( "[ERROR] RTTI: Failed to register type.. an invalid name was specified" );
			return false;
		}

		auto& list = getTypeMap();

		// Check if an entry already exists with this identifier
		if( list.find( inIdentifier ) != list.end() )
		{
			Console::WriteLine( "[ERROR] RTTI: Failed to register type (", inName, ") because a type with the same identifier is already registered!" );
			return false;
		}

		// For now, were only going to store the immediate parent, once all types are registered (since they arent garunteed, and there
		// is no solid method to force them to load in order) so, we figure out each types parent/children lists after the fact
		// BUT, this has to run before any code attempts to use the type system, so we either have to...
		//  A: Use some hacky type code to run the 'type system init' before the RTTI table is accessed
		//  B: Run the init function inside of engine init, right at the begining

		std::vector< size_t > parentList;
		
		if( inParent == typeid( Object ).hash_code() )
		{
			m_ObjectTypeInfo->ChildrenList.push_back( inIdentifier );
		}
		else
		{
			parentList.push_back( inParent );
		}

		// Full parent list will be initialized when 'RTTI::Initialize' is called
		// This should happen at the very begining of engine initialization
		// But just incase a dev attempts to use the type system before this, it will run then

		list.emplace( inIdentifier, std::make_shared< TypeInfo >( inIdentifier, inName, inCreateFunc, parentList ) );
		return true;
	}


	void RTTI::Initialize()
	{
		// Ensure this is only run once
		static bool bHasInit = false;
		if( bHasInit ) { return; }
		bHasInit = true;

		Console::WriteLine( "[RTTI] Initializing type system..." );

		// Go through the list of types, and we need to build parent and children lists
		auto& list = getTypeMap();

		// First, lets place all of the types into a seperate map, so we can remove them when we finish processing it
		std::map< size_t, std::shared_ptr< TypeInfo > > typeListCopy( list.begin(), list.end() );
		std::map< uint32, std::vector< std::shared_ptr< TypeInfo > > > depthInfo;
		uint32 treeDepth = 0;

		while( typeListCopy.size() > 0 )
		{
			if( treeDepth == 0 )
			{
				// Find types whos parent is Object, i.e. ParentChain is empty
				// and remove them from the copy of the list
				for( auto It = typeListCopy.begin(); It != typeListCopy.end(); )
				{
					HYPERION_VERIFY( It->second, "[RTTI] Failed init.. type was null" );

					if( It->second->ParentChain.size() == 0 )
					{
						// Add to top level of depth info, and erase from flat list
						depthInfo[ treeDepth ].push_back( It->second );
						It = typeListCopy.erase( It );
					}
					else
					{
						It++;
					}
				}
			}
			else
			{
				// Look for types whos parent is in the last level of tree depth
				auto& lastDepthList = depthInfo.at( treeDepth - 1 );

				for( auto It = typeListCopy.begin(); It != typeListCopy.end(); )
				{
					HYPERION_VERIFY( It->second, "[RTTI] Failed init.. type was null" );
					HYPERION_VERIFY( It->second->ParentChain.size() == 1, "[RTTI] Failed init.. parent chain invalid?" );

					auto parentId = It->second->ParentChain.front();

					// Loop through the list of types registered in the previous 'depth'
					// Look for our parent type..
					bool bFound = false;

					for( auto tIt = lastDepthList.begin(); tIt != lastDepthList.end(); tIt++ )
					{
						if( parentId == ( *tIt )->Identifier )
						{
							// We found our parent!
							// We need to add ourself as a child, and finish building our parent chain 
							// by copying in our parents chain, and prepending the parents ID
							( *tIt )->ChildrenList.push_back( It->first );
							It->second->ParentChain.insert( It->second->ParentChain.end(), ( *tIt )->ParentChain.begin(), ( *tIt )->ParentChain.end() );

							bFound = true;
							break;
						}
					}

					// If we found a parent, remove from the flat list, and add to the tree structure
					if( bFound )
					{
						depthInfo[ treeDepth ].push_back( It->second );
						It = typeListCopy.erase( It );
					}
					else
					{
						It++;
					}
				}
			}

			treeDepth++;
		}

		Console::WriteLine( "[RTTI] Finished initialization! ", list.size(), " types loaded..." );
	}
}
