/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/RenderMarshal.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once
#include "Hyperion/Core/Object.h"
#include <condition_variable>


namespace Hyperion
{
	class Renderer;

	class RenderMarshal : public Object
	{

	public:

		void InitThread();
		void TickThread();
		void ShutdownThread();

	private:

		std::weak_ptr< Renderer > m_RendererRef;

		void PerformUpdatePass();
		void PerformCleanupPass();

		friend class Engine;

	};
}