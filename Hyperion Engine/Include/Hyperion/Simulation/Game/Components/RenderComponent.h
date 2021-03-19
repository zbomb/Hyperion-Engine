/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Simulation/Game/Components/RenderComponent.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Simulation/Game/Component.h"
#include "Hyperion/Renderer/Proxy/ProxyBase.h"


namespace Hyperion
{
	enum class RenderComponentState
	{
		Clean,
		Dirty,
		Stale
	};

	class RenderComponent : public Component
	{

	private:

		RenderComponentState m_RenderState;

	protected:

		virtual bool PerformProxyCreation() = 0;
		virtual bool PerformProxyUpdate() = 0;

		void MarkDirty();
		void MarkStale();

		virtual void AddToRenderer() = 0;
		virtual void RemoveFromRenderer() = 0;

		virtual void OnSpawn( const HypPtr< World >& inWorld ) override;
		virtual void OnDespawn( const HypPtr< World >& inWorld ) override;

	public:

		inline RenderComponentState GetRenderState() const { return m_RenderState; }

		virtual void SetPosition( const Vector3D& inPosition ) override;
		virtual void SetRotation( const Angle3D& inRotation ) override;
		virtual void SetQuaternion( const Quaternion& inQuat ) override;
		virtual void SetScale( const Vector3D& inScale ) override;
		virtual void SetTransform( const Transform& inTransform ) override;

		virtual void Translate( const Vector3D& inPos ) override;
		virtual void Rotate( const Quaternion& inQuat ) override;
		virtual void Rotate( const Angle3D& inEuler ) override;

		friend class GameInstance; // So it can reach in and mark this component as clean
	};

}