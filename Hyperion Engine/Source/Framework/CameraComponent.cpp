/*==================================================================================================
	Hyperion Engine
	Source/Framework/CameraComponent.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Framework/CameraComponent.h"
#include "Hyperion/Core/GameManager.h"
#include "Hyperion/Renderer/Proxy/ProxyCamera.h"
#include "Hyperion/Core/RenderManager.h"
#include "Hyperion/Framework/World.h"


namespace Hyperion
{

	void CameraComponent::AddToRenderer()
	{
		GameManager::GetInstance()->RegisterRenderComponent( AquirePointer< CameraComponent >() );
	}


	void CameraComponent::RemoveFromRenderer()
	{
		GameManager::GetInstance()->RemoveRenderComponent( AquirePointer< CameraComponent >() );
	}

	bool CameraComponent::PerformProxyCreation()
	{
		// First, we need to create a new proxy
		auto newProxy = std::make_shared< ProxyCamera >();
		if( !newProxy )
		{
			return false;
		}

		m_Proxy = newProxy;

		// Initialize this proxy
		m_Proxy->GameInit();

		// Run renderer command to add this new proxy
		RenderManager::AddCommand( std::make_unique< AddCameraProxyCommand >( m_Proxy ) );
		return true;
	}

	bool CameraComponent::UpdateProxy()
	{
		// Ensure we have a cached proxy copy
		if( !m_Proxy ) return false;
		
		auto proxy = m_Proxy;
		float cFOV = m_FOV;
		float cAR = m_AspectRatio;
		bool cA = m_Active;

		// Enqueue the render command
		HYPERION_RENDER_COMMAND( [ = ] ( Renderer& r ) 
								 {
									if( proxy )
									{
										proxy->SetFOV( cFOV );
										proxy->SetAspectRatio( cAR );

										if( proxy->IsActive() != cA )
										{
											// Tell renderer to update active camera
											proxy->SetActive( cA );
											
											if( cA )
											{
												//r.GetScene()->SetActiveCamera( proxy );
											}
											else
											{
												//r.GetScene()->SetActiveCamera( nullptr );
											}
										}
										else
										{
											r.GetScene()->OnCameraUpdate();
										}
									}
								 } );
	}


	void CameraComponent::SetActiveCamera( bool bIn )
	{
		if( m_Active != bIn )
		{
			m_Active = bIn;
			MarkDirty();

			if( bIn )
			{
				GetWorld()->SetActiveCamera( AquirePointer< CameraComponent >() );
			}
			else
			{
				GetWorld()->OnCameraUpdated();
			}
		}
	}

	void CameraComponent::SetFOV( float fIn )
	{
		if( m_FOV != fIn )
		{
			m_FOV = fIn;
			MarkDirty();

			GetWorld()->OnCameraUpdated();
		}
	}

	void CameraComponent::SetAspectRatio( float fIn )
	{
		if( m_AspectRatio != fIn )
		{
			m_AspectRatio = fIn;
			MarkDirty();

			GetWorld()->OnCameraUpdated();
		}
	}


	void CameraComponent::GetViewState( ViewState& Out )
	{
		Out.Position = GetPosition();
		Out.Rotation = GetRotation();
		Out.FOV = m_FOV;
		Out.AspectRatio = m_AspectRatio;
	}

}