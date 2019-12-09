/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/RenderFactory.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <iostream>
#include <memory>
#include "Hyperion/Renderer/Renderer.h"


namespace Hyperion
{
	enum class RendererType
	{
		DirectX11
	};

	class IRenderFactory
	{
	public:


		static std::shared_ptr< Renderer > CreateRenderer( RendererType inType );

	};

}