/*==================================================================================================
	Hyperion Engine
	Tests/HSMTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Assets/StaticModelAsset.h"


namespace Hyperion
{
	namespace Tests
	{
		void RunHSMTests()
		{
			Console::WriteLine( "\n================================================================================================" );
			Console::WriteLine( "--> Running HSM test...\n" );

			auto mdl = AssetManager::Get< StaticModelAsset >( "models/test_model.hsm" );
			if( mdl )
			{
				Console::WriteLine( "-----> Loaded static model!" );
			}
			else
			{
				Console::WriteLine( "------> Failed to load static model!" );
			}

			Console::WriteLine( "--> HSM test complete!" );
			Console::WriteLine( "================================================================================================\n" );

		}
	}
}