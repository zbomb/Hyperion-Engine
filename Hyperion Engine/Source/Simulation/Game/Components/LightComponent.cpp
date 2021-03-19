/*==================================================================================================
	Hyperion Engine
	Source/Simulation/Game/Components/LightComponent.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Simulation/Game/Components/LightComponent.h"
#include "Hyperion/Renderer/Proxy/ProxyLight.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Core/GameInstance.h"
#include "Hyperion/Renderer/Renderer.h"


namespace Hyperion
{

	LightComponent::LightComponent()
		: m_Type( LightType::Point ), m_Color( 1.f, 1.f, 1.f )
	{

	}


	void LightComponent::SetLightType( LightType inType )
	{
		m_Type = inType;

		// Changing the light type requires a full re-creation of the proxy
		// This is because, the light proxy is initialized differently based on the type
		MarkStale();
	}


	void LightComponent::SetColor( const Color3F& inColor )
	{
		m_Color = inColor;
		MarkDirty();
	}


	void LightComponent::SetBrightness( float inBrightness )
	{
		m_Brightness = inBrightness;
		MarkDirty();
	}


	void LightComponent::SetRadius( float inRadius )
	{
		m_Radius = inRadius;
		MarkDirty();
	}


	void LightComponent::AddToRenderer()
	{
		Engine::GetGame()->RegisterRenderComponent( AquirePointer< LightComponent >() );
	}


	void LightComponent::RemoveFromRenderer()
	{
		Engine::GetGame()->RemoveRenderComponent( AquirePointer< LightComponent >() );
	}


	bool LightComponent::PerformProxyCreation()
	{
		// Create the new proxy
		auto newProxy = std::make_shared< ProxyLight >( GetIdentifier(), m_Type );
		newProxy->SetColor( m_Color );
		newProxy->SetBrightness( m_Brightness );
		newProxy->SetRadius( m_Radius );
		newProxy->m_Transform = GetWorldTransform();

		// Initialize proxy
		newProxy->GameInit();

		// Add to renderer
		Engine::GetRenderer()->AddCommand( std::make_unique< AddLightProxyCommand >( newProxy ) );
		m_Proxy = newProxy;

		return true;
	}


	bool LightComponent::PerformProxyUpdate()
	{

		auto ptr = m_Proxy.lock();
		if( !ptr ) { return false; }

		auto t = GetWorldTransform();

		HYPERION_RENDER_COMMAND(
			[=] ( Renderer& r )
			{
				ptr->m_Transform = t;
				ptr->SetColor( m_Color );
				ptr->SetBrightness( m_Brightness );
				ptr->SetRadius( m_Radius );
			}
		);

		return true;
	}

}


/*
*	Register Type
*/
HYPERION_REGISTER_OBJECT_TYPE( LightComponent, RenderComponent );