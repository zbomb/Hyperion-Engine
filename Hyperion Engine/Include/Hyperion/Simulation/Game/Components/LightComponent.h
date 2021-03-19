/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Simulation/Game/Components/LightComponent.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Simulation/Game/Components/RenderComponent.h"
#include "Hyperion/Library/Color.h"


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class ProxyLight;


	class LightComponent : public RenderComponent
	{

	protected:

		std::weak_ptr< ProxyLight > m_Proxy;
		LightType m_Type;
		Color3F m_Color;
		float m_Brightness;
		float m_Radius;

		bool PerformProxyCreation() override;
		bool PerformProxyUpdate() override;

		void AddToRenderer() override;
		void RemoveFromRenderer() override;

	public:

		LightComponent();

		void SetLightType( LightType inType );
		inline LightType GetLightType() const { return m_Type; }

		void SetColor( const Color3F& inColor );
		inline Color3F GetColor() const { return m_Color; }

		void SetBrightness( float inBrightness );
		inline float GetBrightness() const { return m_Brightness; }

		void SetRadius( float inRadius );
		inline float GetRadius() const { return m_Radius; }

	};

}
