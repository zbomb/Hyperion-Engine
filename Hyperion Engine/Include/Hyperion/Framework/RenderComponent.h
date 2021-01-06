/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Framework/RenderComponent.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Framework/Component.h"
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

		friend class GameInstance; // So it can reach in and mark this component as clean
	};

}