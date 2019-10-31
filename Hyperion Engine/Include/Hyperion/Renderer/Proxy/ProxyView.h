/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Proxy/ProxyView.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Framework/ViewState.h"


namespace Hyperion
{
	/*
		Hyperion::ProxyView
		* Represents an active camera view within the game
	*/
	class ProxyView
	{

	private:

		ViewState m_State;

	public:

		inline Vector3D GetPosition() const		{ return m_State.Position; }
		inline Angle3D GetRotation() const		{ return m_State.Rotation; }
		inline float GetFOV() const				{ return m_State.FOV; }
		inline float GetAspectRatio() const		{ return m_State.AspectRatio; }

		friend class ProxyScene;

	};

}