/*==================================================================================================
	Hyperion Engine
	Source/Renderer/RenderFactory.cpp
	© 2019, Zachary Berry
==================================================================================================*/


#include "Hyperion/Renderer/RenderFactory.h"

// We dont want to build the directx renderer if its not supported on this platform
#ifdef HYPERION_SUPPORT_DIRECTX
#include "Hyperion/Renderer/DirectX11/DirectX11Renderer.h"
#endif

namespace Hyperion
{
	std::shared_ptr< Renderer > IRenderFactory::CreateRenderer( RendererType inType )
	{
		switch( inType )
		{
#ifdef HYPERION_SUPPORT_DIRECTX
		case RendererType::DirectX11:
#endif
		default:
#ifdef HYPERION_SUPPORT_DIRECTX
			return std::make_shared< DirectX11Renderer >();
#else
#endif
		}
	}

}