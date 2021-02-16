/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Proxy/ProxyLight.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Renderer/Proxy/ProxyBase.h"
#include "Hyperion/Library/Color.h"


namespace Hyperion
{

	class ProxyLight : public ProxyBase
	{

	protected:

		LightType m_Type;
		Color3F m_Color;
		float m_Brightness;
		float m_Radius;

	public:

		ProxyLight() = delete;
		ProxyLight( uint32 inIdentifier, LightType inType )
			: ProxyBase( inIdentifier ), m_Type( inType ), m_Color( 1.f, 1.f, 1.f ), m_Brightness( 0.5f )
		{}

		virtual ~ProxyLight() {}

		inline LightType GetType() const { return m_Type; }
		inline Color3F GetColor() const { return m_Color; }
		inline float GetBrightness() const { return m_Brightness; }
		inline float GetRadius() const { return m_Radius; }

		inline void SetColor( const Color3F& inColor ) { m_Color = inColor; }
		inline void SetBrightness( float inBrightness ) { m_Brightness = inBrightness; }
		inline void SetRadius( float inRadius ) { m_Radius = inRadius; }

	};

}