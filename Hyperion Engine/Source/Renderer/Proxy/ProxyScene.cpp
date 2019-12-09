/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Proxy/ProxyScene.cpp
	© 2019, Zachary Berry
==================================================================================================*/

// Hyperion Includes
#include "Hyperion/Renderer/Proxy/ProxyScene.h"

// STD Includes
#include <iostream>

namespace Hyperion
{


	/*
		ProxyScene::Initialize
		* Called after the renderer is created
		* Called from main render thread
	*/
	void ProxyScene::Initialize()
	{
		std::cout << "[DEBUG] ProxyScene: Initializing...\n";
	}

	/*
		ProxyScene::Shutdown
		* Called when the renderer is supposed to shutdown
		* Called from main render thread
	*/
	void ProxyScene::Shutdown()
	{
		std::cout << "[DEBUG] ProxyScene: Shutdown...\n";
	}
}