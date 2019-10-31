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
		/*
			Static Accessor Methods
		*/
	private:

		static bool m_RendererSet;
		static RendererType m_RendererType;
		static std::unique_ptr< IRenderFactory > m_RenderFactory;

	public:

		static void SetActiveRenderer( RendererType inType )
		{
			if( m_RendererSet )
			{
				std::cout << "[ERROR] IRenderFactory: Can only set target renderer type once!\n";
			}
			else
			{
				m_RendererType	= inType;
				m_RendererSet	= true;
			}
		}

		static IRenderFactory& GetInstance();

		/*
			RenderFactory Interface
		*/
	public:

		virtual std::shared_ptr< Renderer > CreateRenderer() = 0;


	};

}