
#pragma once

#include "Hyperion/Core/GameManager.h"
#include "Hyperion/Framework/TestEntity.h"
#include "Hyperion/Framework/StaticModelComponent.h"


void RunEngineTests()
{
	Hyperion::Console::WriteLine( "[TEST] Starting type system test..." );

	auto test_debug = Hyperion::CreateObject< Hyperion::StaticModelComponent >();
	auto test_type = test_debug->GetType();

	// Create an object
	auto ent = Hyperion::CreateObject< Hyperion::TestEntity >();
	auto entType = ent->GetType();

	Hyperion::Console::WriteLine( "\tType Name: ", entType->GetTypeName() );
	Hyperion::Console::WriteLine( "\tType ID: ", entType->GetTypeIdentifier() );
	
	// Get immediate parent
	auto parentType = entType->GetParent();
	auto parentList = entType->GetParentList();

	Hyperion::Console::WriteLine( "\tParent Type Name: ", parentType->GetTypeName() );
	Hyperion::Console::WriteLine( "\tParent Type ID: ", parentType->GetTypeIdentifier() );

	Hyperion::Console::WriteLine( "\n-------------------------- Parent Chain ---------------------------" );
	Hyperion::Console::WriteLine( "\tFull Parent Count: ", parentList.size() );
	for( auto It = parentList.begin(); It != parentList.end(); It++ )
	{
		Hyperion::Console::WriteLine( "\t\tParent in chain: ", ( *It )->GetTypeName() );
	}

	Hyperion::Console::WriteLine( "-----------------------------------------------------------------------\n" );

	// Test other functions....

	bool bSuccess = true;

	if( entType->IsDerivedFrom< Hyperion::Entity >() )
	{
		Hyperion::Console::WriteLine( "---> IsDerivedFrom<T> success." );
	}
	else { bSuccess = false; }

	if( entType->IsDerivedFrom( Hyperion::Type::Get< Hyperion::Entity >() ) )
	{
		Hyperion::Console::WriteLine( "---> IsDerivedFrom( Type ) success" );
	}
	else { bSuccess = false; }

	if( entType->IsDirectlyDerivedFrom< Hyperion::Entity >() )
	{
		Hyperion::Console::WriteLine( "---> IsDirectlyDerivedFrom<T> success." );
	}
	else { bSuccess = false; }

	if( entType->IsDirectlyDerivedFrom( Hyperion::Type::Get< Hyperion::Entity >() ) )
	{
		Hyperion::Console::WriteLine( "---> IsDirectlyDerivedFrom( Type ) success." );
	}
	else { bSuccess = false; }

	auto tt = Hyperion::Type::Get( "entity" );
	if( tt.IsValid() && tt->IsValid() )
	{
		Hyperion::Console::WriteLine( "---> Type::Get( String ) success." );
	}
	else { bSuccess = false; }

	if( tt->IsDirectParentTo< Hyperion::TestEntity >() )
	{
		Hyperion::Console::WriteLine( "---> IsDirectParentTo< T >() success." );
	}
	else { bSuccess = false; }

	if( tt->IsDirectParentTo( entType ) )
	{
		Hyperion::Console::WriteLine( "---> IsDirectParentTo( Type ) success." );
	}
	else { bSuccess = false; }

	if( !bSuccess )
	{
		Hyperion::Console::WriteLine( "---> [ERROR] Not all tests passed!!!\n" );
	}


	Hyperion::Console::WriteLine( "---> Getting object type..." );
	auto objType = Hyperion::Type::Get< Hyperion::Object >();

	if( objType ) { Hyperion::Console::WriteLine( "\t\t- Success!" ); }

	Hyperion::Console::WriteLine( "\n-------------------------- Children List --------------------------------" );

	auto children = objType->GetDirectChildren();
	Hyperion::Console::WriteLine( "\tDirect children count: ", children.size() );

	for( auto It = children.begin(); It != children.end(); It++ )
	{
		Hyperion::Console::WriteLine( "\t\tChild Name: ", ( *It )->GetTypeName() );
	}

	Hyperion::Console::WriteLine( "----------------------------------------------------------------------------\n" );

}

void RunMiscTests()
{


}