/*==================================================================================================
	Hyperion Engine
	Tests/AssetTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <iostream>

#include "Hyperion/Core/AssetManager.h"
#include "Hyperion/Assets/TestAsset.h"


namespace Hyperion
{
namespace Tests
{

	void RunAssetTests()
	{
		std::cout << "--------------------------------------------------------------------------------------------------\n---> Running asset tests!\n\n";

		auto asset = AssetManager::LoadSync< TestAsset >( "test_asset.htxt" );
		if( asset.IsValid() )
		{
			std::cout << "---> Test asset valid!\n";
			std::cout << "-----> String Value: " << asset->m_Data << "\n";
		}
		else
		{
			std::cout << "----> Test asset invalid!\n";
		}

		std::cout << "---> Asset tests complete!\n";
		std::cout << "--------------------------------------------------------------------------------------------------\n\n";
	}
}
}