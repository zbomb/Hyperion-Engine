/*==================================================================================================
	Hyperion Engine
	Source/Simulation/Game/Entities/PointLight.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Simulation/Game/Entities/PointLight.h"



namespace Hyperion
{

	void PointLight::OnCreate()
	{
		// Create our light component
		m_Light = CreateObject< LightComponent >();
		m_Light->SetLightType( LightType::Point );
		m_Light->SetBrightness( 0.5f );
		m_Light->SetColor( Color3F( 1.f, 1.f, 1.f ) );
		m_Light->SetRadius( 200.f );

		AddComponent( m_Light, "light", nullptr );
	}


	void PointLight::OnDestroy()
	{

	}


	void PointLight::OnSpawn( const HypPtr< World >& inWorld )
	{

	}


	void PointLight::OnDespawn( const HypPtr< World >& inWorld )
	{

	}

	void PointLight::SetColor( const Color3F& inColor )
	{
		if( m_Light ) m_Light->SetColor( inColor );
	}

	Color3F PointLight::GetColor() const
	{
		return m_Light ? m_Light->GetColor() : Color3F( 1.f, 1.f, 1.f );
	}

	void PointLight::SetBrightness( float inBrightness )
	{
		if( m_Light ) m_Light->SetBrightness( inBrightness );
	}

	float PointLight::GetBrightness() const
	{
		return m_Light ? m_Light->GetBrightness() : 0.f;
	}

	void PointLight::SetRadius( float inRadius )
	{
		if( m_Light ) m_Light->SetRadius( inRadius );
	}

	float PointLight::GetRadius() const
	{
		return m_Light ? m_Light->GetBrightness() : 0.f;
	}
}

HYPERION_REGISTER_OBJECT_TYPE( PointLight, Entity );
