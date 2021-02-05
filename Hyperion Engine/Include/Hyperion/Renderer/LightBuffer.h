/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/LightBuffer.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Proxy/ProxyLight.h"


namespace Hyperion
{

	class RLightBuffer
	{

	public:

		virtual ~RLightBuffer() {}

		virtual void Shutdown() = 0;
		virtual bool IsValid() const = 0;

		virtual bool UploadLights( const std::vector< std::shared_ptr< ProxyLight > >& inLights ) = 0;
		virtual uint32 GetLightCount() const = 0;

	};

}