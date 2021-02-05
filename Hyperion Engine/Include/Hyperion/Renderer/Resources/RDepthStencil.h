/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resources/RDepthStencil.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{

	class RDepthStencil
	{

	public:

		virtual ~RDepthStencil() {}

		virtual bool IsValid() const = 0;
		virtual void Shutdown() = 0;

	};

}