/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resource/DepthStencil.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{
	/*
	*	Forward Declarations
	*/
	class RTexture2D;


	class RDepthStencil
	{

	public:

		virtual ~RDepthStencil()
		{

		}

		virtual bool IsValid() const = 0;
		virtual void Shutdown() = 0;
		
	};

}