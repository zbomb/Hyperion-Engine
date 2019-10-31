/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11Factory.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/RenderFactory.h"


namespace Hyperion
{

	class DirectX11Factory : public IRenderFactory
	{

		/*
			IRenderFactory Implementation
		*/
		std::shared_ptr< Renderer > CreateRenderer();

	};

}