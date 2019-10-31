/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Renderer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"
#include "Hyperion/Core/Library/Mutex.hpp"


namespace Hyperion
{
	class ProxyScene;

	class Renderer : public Object
	{

		/*
			Thread Functions
		*/
	public:

		virtual void InitThread();
		virtual void TickThread();
		virtual void ShutdownThread();

		Renderer();
		~Renderer();

		/*
			Proxy Scene
		*/
	private:

		std::shared_ptr< ProxyScene > m_Scene;

	public:

		std::weak_ptr< ProxyScene > GetScne() { return std::weak_ptr< ProxyScene >( m_Scene ); }

		/*
			Render Marshal
		*/
	private:

		Barrier m_MainPassBarrier;
		Barrier m_UpdateBarrier;

		void PerformRenderPass();
		void ApplyProxyUpdates();


		friend class RenderMarshal;

	};

}