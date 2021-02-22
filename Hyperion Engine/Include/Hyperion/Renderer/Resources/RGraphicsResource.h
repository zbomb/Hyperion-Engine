/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resources/RGraphicsResource.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{



	class RGraphicsResource
	{

	public:

		virtual ~RGraphicsResource() {}

		virtual bool IsComputeTarget() const = 0;
		virtual bool IsRenderTarget() const = 0;
		virtual bool IsShaderResource() const = 0;

	};

}