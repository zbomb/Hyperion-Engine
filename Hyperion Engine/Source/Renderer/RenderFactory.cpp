/*==================================================================================================
	Hyperion Engine
	Source/Renderer/RenderFactory.cpp
	© 2019, Zachary Berry
==================================================================================================*/


#include "Hyperion/Renderer/RenderFactory.h"
#include "Hyperion/Renderer/DirectX11/DirectX11Factory.h"


namespace Hyperion
{
	// Default Renderer Type
	RendererType IRenderFactory::m_RendererType( RendererType::DirectX11 );
	bool IRenderFactory::m_RendererSet( false );
	std::unique_ptr< IRenderFactory > IRenderFactory::m_RenderFactory( nullptr );

	IRenderFactory& IRenderFactory::GetInstance()
	{
		// Check if we need to create the renderer object
		if( !m_RenderFactory )
		{
			switch( m_RendererType )
			{
			case RendererType::DirectX11:
			default:
				m_RenderFactory = std::make_unique< DirectX11Factory >();
				break;
			}
		}

		// Return reference to this object
		static IRenderFactory& CachedFactory = static_cast< IRenderFactory& >( *m_RenderFactory );
		return CachedFactory;
	}

}