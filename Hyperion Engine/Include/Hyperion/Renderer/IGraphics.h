/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/IGraphics.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/DataTypes.h"


namespace Hyperion
{

	class IGraphics
	{

	public:

		virtual ~IGraphics()
		{}

		virtual bool SetResolution( const ScreenResolution& inResolution ) = 0;
		virtual void SetVSync( bool bVSync ) = 0;

		virtual bool Initialize( const IRenderOutput& ) = 0;
		virtual void Shutdown() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void EnableAlphaBlending() = 0;
		virtual void DisableAlphaBlending() = 0;
		virtual bool IsAlphaBlendingEnabled() = 0;

		virtual void EnableZBuffer() = 0;
		virtual void DisableZBuffer() = 0;
		virtual bool IsZBufferEnabled() = 0;

		virtual std::vector< ScreenResolution > GetAvailableResolutions() = 0;

	};

}