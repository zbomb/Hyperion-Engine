/*==================================================================================================
	Hyperion Engine
	Tests/FileTests.hpp
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/File.h"
#include "Hyperion/Core/Platform.h"

#include <iostream>


namespace Hyperion
{
namespace Tests
{
	
	void RunFileTests()
	{
		std::cout << "\n---------------------------------------------------------------------------------------------\n[TEST] Running file test...\n";

		std::cout << "----> Testing platform services for win32!\n";
		Platform::GetExecutableName();

		std::cout << std::endl;
		std::cout << "----> File Test Complete!\n";
		std::cout << "---------------------------------------------------------------------------------------------\n";
	}

}
}