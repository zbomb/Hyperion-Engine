/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/DirectX11Factory.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/DirectX11Factory.h"
#include "Hyperion/Renderer/DirectX11/DirectX11Renderer.h"
#include "Hyperion/Core/Engine.h"


namespace Hyperion
{

	std::shared_ptr< Renderer > DirectX11Factory::CreateRenderer()
	{
		auto& Eng = Engine::GetInstance();
		return Eng.CreateObject< DirectX11Renderer >();
	}

}