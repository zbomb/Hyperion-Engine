/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Core/GameInstance.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Object.h"
#include "Hyperion/Core/Simulation.h" // TODO: Move this file to Hyperion/Simulation/Simulation.h
#include "Hyperion/Renderer/Renderer.h"


namespace Hyperion
{

	/*
	*	Game Instance Class
	*/
	class GameInstance : public Object
	{

	private:

		HypPtr< Thread > m_CoreThread;
		std::atomic< bool > m_bIsGameRunning;

		HypPtr< Simulation > m_Simulation;
		HypPtr< Renderer > m_Renderer;

		void CoreMain( const std::atomic< bool >& bThreadState );

	public:

		GameInstance();
		~GameInstance();

		Engine::InitializeResult BeginGame( const Engine::InitializeParameters& inParams );
		void EndGame();

	};

	/*	-------------------------------------- OLD ------------------------------------------
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
		Transform m_LastTickCameraTransform;
		float m_LastTickCameraFOV;

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

		bool AddEntityToActiveWorld( const HypPtr< Entity >& inEnt, const Transform& inTransform = Transform() );
		bool RemoveEntityFromActiveWorld( const HypPtr< Entity >& inEnt );

		/*
			Hooks
		
		virtual void OnStart();
		virtual void OnStop();

		/*
			Render Components
		
		bool RegisterRenderComponent( const HypPtr< PrimitiveComponent >& );
		bool RegisterRenderComponent( const HypPtr< LightComponent >& );

		bool RemoveRenderComponent( const HypPtr< PrimitiveComponent >& );
		bool RemoveRenderComponent( const HypPtr< LightComponent >& );

		void ProcessRenderUpdates();
		*/

}