/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Simulation/Game/Entities/PointLight.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Simulation/Game/Components/LightComponent.h"
#include "Hyperion/Simulation/Game/Entity.h"


namespace Hyperion
{

	class PointLight : public Entity
	{

	protected:

		HypPtr< LightComponent > m_Light;

		void OnCreate() override;
		void OnDestroy() override;

		void OnSpawn( const HypPtr< World >& inWorld ) override;
		void OnDespawn( const HypPtr< World >& inWorld ) override;

	public:

		void SetColor( const Color3F& inColor );
		Color3F GetColor() const;

		void SetBrightness( float inBrightness );
		float GetBrightness() const;

		void SetRadius( float inRadius );
		float GetRadius() const;

	};

}