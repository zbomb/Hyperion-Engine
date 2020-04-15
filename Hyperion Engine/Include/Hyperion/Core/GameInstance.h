/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Core/GameInstance.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Object.h"
#include "Hyperion/Core/Types/Transform.h"


namespace Hyperion
{
	class Entity;
	class World;
	class RenderComponent;
	class PrimitiveComponent;
	class LightComponent;
	class CameraComponent;
	class LocalPlayer;


	class GameInstance : public Object
	{

	private:

		HypPtr< World > m_ActiveWorld;
		std::map< uint32, HypPtr< RenderComponent > > m_ActiveRenderComponents; // Maybe should move into world..?

		HypPtr< LocalPlayer > m_LocalPlayer;
		uint32 m_LastTickScreenHeight;

	protected:

		bool ClearActiveWorld();
		bool SetActiveWorld( const HypPtr< World >& inWorld );

		bool DoRegisterRenderComponent( const HypPtr< RenderComponent >& inComp );
		bool DoRemoveRenderComponent( const HypPtr< RenderComponent >& inComp );

	public:

		GameInstance();
		GameInstance( const GameInstance& ) = delete;
		GameInstance( GameInstance&& ) = delete;

		~GameInstance();

		virtual void Initialize() final;
		virtual void Shutdown() final;

		inline HypPtr< World > GetWorld() const { return m_ActiveWorld; }
		inline HypPtr< LocalPlayer > GetLocalPlayer() const { return m_LocalPlayer; }

		bool AddEntityToActiveWorld( const HypPtr< Entity >& inEnt, const Transform3D& inTransform = Transform3D() );
		bool RemoveEntityFromActiveWorld( const HypPtr< Entity >& inEnt );

		/*
			Hooks
		*/
		virtual void OnStart();
		virtual void OnStop();

		/*
			Render Components
		*/
		bool RegisterRenderComponent( const HypPtr< PrimitiveComponent >& );
		bool RegisterRenderComponent( const HypPtr< LightComponent >& );

		bool RemoveRenderComponent( const HypPtr< PrimitiveComponent >& );
		bool RemoveRenderComponent( const HypPtr< LightComponent >& );

		void ProcessRenderUpdates();


	};

}