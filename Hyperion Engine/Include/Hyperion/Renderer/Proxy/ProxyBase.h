/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Proxy/ProxyBase.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

// Hyperion Includes
#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Types/Transform.h"


namespace Hyperion
{

	class ProxyBase
	{
		
	protected:

		uint32 m_Identifier;

	public:

		ProxyBase() = delete;

		ProxyBase( uint32 inIdentifier )
			: m_Identifier( inIdentifier )
		{}

		Transform3D m_Transform;

		virtual void GameInit() = 0;
		virtual void RenderInit() = 0;

		virtual void BeginShutdown() = 0;
		virtual void Shutdown() = 0;
		inline uint32 GetIdentifier() const { return m_Identifier; }

		inline Transform3D GetTransform() const { return m_Transform; }
	};

}