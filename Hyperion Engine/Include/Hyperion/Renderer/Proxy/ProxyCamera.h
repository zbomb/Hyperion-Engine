/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Proxy/ProxyCamera.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/Proxy/ProxyBase.h"
#include "Hyperion/Library/Math.h"


namespace Hyperion
{

	class ProxyCamera : public ProxyBase
	{

	protected:

		bool m_Active;
		float m_FOV, m_AspectRatio;

	public:

		ProxyCamera() = delete;
		ProxyCamera( uint32 inIdentifier )
			: ProxyBase( inIdentifier ), m_Active( false )
		{}

		inline bool IsActive() const { return m_Active; }
		inline float GetFOV() const { return m_FOV; }
		inline float GetAspectRatio() const { return m_AspectRatio; }

		void SetActive( bool bIn )
		{
			m_Active = bIn;
		}

		void SetFOV( float inFOV )
		{
			m_FOV = inFOV;
		}

		void SetAspectRatio( float inAspectRatio )
		{
			m_AspectRatio = inAspectRatio;
		}

	};

}