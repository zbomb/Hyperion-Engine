/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/CameraComponent.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/RenderComponent.h"


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class ProxyCamera;


	class CameraComponent : public RenderComponent
	{

	protected:

		bool PerformProxyCreation() override;
		bool UpdateProxy() override;

		void AddToRenderer() override;
		void RemoveFromRenderer() override;

		float m_FOV;
		float m_AspectRatio;
		bool m_Active;

		std::shared_ptr< ProxyCamera > m_Proxy;

	public:
		inline bool IsActiveCamera() const { return m_Active; }
		inline float GetFOV() const { return m_FOV; }
		inline float GetAspectRatio() const { return m_AspectRatio; }

		void SetActiveCamera( bool bIn );
		void SetFOV( float fIn );
		void SetAspectRatio( float fIn );

		void GetViewState( ViewState& Output );

	};

}