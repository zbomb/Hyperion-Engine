/*==================================================================================================
	Hyperion Engine
	Source/Core/GameInstance.cpp
	� 2021, Zachary Berry
==================================================================================================*/


#include "Hyperion/Core/GameInstance.h"
#include "Hyperion/Core/ThreadManager.h"


namespace Hyperion
{

	GameInstance::GameInstance( const HypPtr< ISimulationSchedulerTask >& inSimulationScheduler, const HypPtr< IRenderSchedulerTask >& inRenderScheduler )
		: m_SimulationScheduler( inSimulationScheduler ), m_RenderScheduler( inRenderScheduler ), m_bIsGameRunning( false )
	{
		HYPERION_VERIFY( inSimulationScheduler.IsValid() && inRenderScheduler.IsValid(), "[GameInstance] Both of these tasks have to be valid!" );
	}


	GameInstance::~GameInstance()
	{

	}


	bool GameInstance::StartGame()
	{
		// Ensure we are not already running
		if( m_bIsGameRunning.load() || m_CoreThread.IsValid() )
		{
			Console::WriteLine( "[ERROR] GameInstance: Failed to start game.. the game loop is already running" );
			return false;
		}

		// Ensure our main tasks are valid
		if( !m_SimulationScheduler.IsValid() || !m_RenderScheduler.IsValid() )
		{
			Console::WriteLine( "[ERROR] GameInstance: Failed to start game.. the render and simulation scheduler tasks were not valid! Check override settings" );
			return false;
		}

		m_bIsGameRunning.store( true );

		// Create the game instance thread
		ThreadParameters params {};

		params.identifier = "core";
		params.mainFunc = std::bind( &GameInstance::CoreMain, this, std::placeholders::_1 );
		params.bIsTicked = false;
		params.bAutoStart = true;

		m_CoreThread = ThreadManager::CreateThread( params );
		if( !m_CoreThread.IsValid() )
		{
			Console::WriteLine( "[ERROR] GameInstance: Failed to start game.. couldnt create the 'core game thread'!" );
			return false;
		}

		return true;
	}


	void GameInstance::StopGame()
	{
		m_bIsGameRunning.store( false );

		if( m_CoreThread.IsValid() )
		{
			m_CoreThread->Stop();
			m_CoreThread.Clear();
		}
	}


	void GameInstance::CoreMain( const std::atomic< bool >& bThreadState )
	{
		// This is the main game loop thread, its responsible for scheduling the tasks for each of the main systems
		// If a game wants to add/remove systems, they will have to override the GameInstance class, and setup and run the game loop however they please
		
		// TODO: Game Init

		while( bThreadState.load() )
		{
			
		}

		// TODO: Game Shutdown
	}

	/* ------------------------------------ OLD --------------------------------------
	*	ConsoleVars
	
	ConsoleVar< float > g_CVar_FOV = ConsoleVar< float >(
		"r_fov", "Field of view multiplier", 0.5f, 0.f, 1.f
	);


	GameInstance::GameInstance()
		: m_ActiveWorld( nullptr ), m_LocalPlayer( nullptr )
	{
		
	}

	GameInstance::~GameInstance()
	{
	
	}

	void GameInstance::Initialize()
	{
		// TODO: Create a system to setup a game instance, and world changes
		// First, create the local player object
		m_LocalPlayer = CreateObject< LocalPlayer >();
		
		// Next, create the world and the needed entities
		auto newWorld	= CreateObject< World >();
		auto character	= CreateObject< Character >();
		auto player		= CreateObject< Player >( PLAYER_LOCAL );

		newWorld->AddEntity( player );
		newWorld->AddEntity( character );
		character->SetPosition( Vector3D( 0.f, 5.f, 0.f ) );

		m_LocalPlayer->SetPlayer( player );
		player->PossessCharacter( character );

		// Finally, lets set the acive world and spawn everything in
		if( !SetActiveWorld( newWorld ) || !newWorld->SpawnWorld() )
		{
			Console::WriteLine( "[ERROR] Game: Failed to setup and spawn the world!" );
		}

		OnStart();
	}


	void GameInstance::Shutdown()
	{
		// Call derived shutdown code
		OnStop();

		// Shut the active world down and despawn it
		ClearActiveWorld();
	}


	bool GameInstance::SetActiveWorld( const HypPtr< World >& inWorld )
	{
		// Validate the world
		if( !inWorld || !inWorld->IsValid() )
		{
			Console::WriteLine( "[ERROR] GameInstance: Attempt to set an invalid world as active!" );
			return false;
		}

		// If theres a current world, then clear it
		ClearActiveWorld();

		// Set new world and call hooks
		m_ActiveWorld = inWorld;
		m_ActiveWorld->m_bActive = true;
		m_ActiveWorld->OnSetActive();

		return true;
	}

	bool GameInstance::ClearActiveWorld()
	{
		// If we dont have a world, or its not valid, then its already been cleared (or never set)
		if( !m_ActiveWorld || !m_ActiveWorld->IsValid() )
		{
			return true;
		}

		if( m_ActiveWorld->IsActive() )
		{
			// When we despawn the world, it should remove all components from the active render component list
			// If anything isnt (for some reason) it will get removed on the next iteration
			if( !m_ActiveWorld->DespawnWorld() )
			{
				Console::WriteLine( "[ERROR] GameInstance: Failed to despawn the active world!" );
				return false;
			}
		}

		// Inform streaming manager of the world reset
		//AdaptiveAssetManagerWorldResetEvent Event;
		//Engine::GetRenderer()->GetAdaptiveAssetManager()->OnWorldReset( Event );
		Engine::GetRenderer()->GetStreamingManager()->Reset();

		m_ActiveWorld->m_bActive = false;
		m_ActiveWorld->OnSetDeactive();

		m_ActiveWorld.Clear();
		return true;
	}


	/*
		Hooks for derived classes
	
	void GameInstance::OnStart()
	{

	}

	void GameInstance::OnStop()
	{

	}


	bool GameInstance::DoRegisterRenderComponent( const HypPtr< RenderComponent >& inComp )
	{
		if( !inComp || !inComp->IsActive() )
		{
			Console::WriteLine( "[ERROR] GameInstance: Failed to register a render component, was null or not spawned!" );
			return false;
		}

		// Add to the list, and mark it as stale so it gets created next frame
		m_ActiveRenderComponents[ inComp->GetIdentifier() ] = inComp;
		inComp->MarkStale();

		return true;
	}

	bool GameInstance::RegisterRenderComponent( const HypPtr< PrimitiveComponent >& inComp )
	{
		if( DoRegisterRenderComponent( inComp ) )
		{
			return true;
		}

		return false;
	}

	bool GameInstance::RegisterRenderComponent( const HypPtr< LightComponent >& inComp )
	{
		if( DoRegisterRenderComponent( inComp ) )
		{
			return true;
		}

		return false;
	}


	bool GameInstance::DoRemoveRenderComponent( const HypPtr< RenderComponent >& inComp )
	{
		// Check 'IsValid' instead of 'IsActive' because the component could be despawned by this point
		if( !inComp || !inComp->IsValid() )
		{
			Console::WriteLine( "[ERROR] GameInstance: Failed to remove render component, it was null/invalid" );
			return false;
		}

		auto entry = m_ActiveRenderComponents.find( inComp->GetIdentifier() );
		if( entry == m_ActiveRenderComponents.end() )
		{
			Console::WriteLine( "[ERROR] GameInstance: Failed to remove render component.. it wasnt in the list!" );
			// Dont return false, because we still want to try and remove it from the renderer just incase
		}
		else
		{
			m_ActiveRenderComponents.erase( entry );
		}

		return true;
	}


	bool GameInstance::RemoveRenderComponent( const HypPtr< PrimitiveComponent >& inComp )
	{
		if( !DoRemoveRenderComponent( inComp ) )
		{
			return false;
		}

		// Add render command to actually remove this component
		Engine::GetRenderer()->AddCommand( std::make_unique< RemovePrimitiveProxyCommand >( inComp->GetIdentifier() ) );
		return true;
	}


	bool GameInstance::RemoveRenderComponent( const HypPtr< LightComponent >& inComp )
	{
		if( !DoRemoveRenderComponent( inComp ) )
		{
			return false;
		}

		// Add render command to actually remove this component
		Engine::GetRenderer()->AddCommand( std::make_unique< RemoveLightProxyCommand >( inComp->GetIdentifier() ) );
		return true;
	}


	void GameInstance::ProcessRenderUpdates()
	{
		// We need to accumulate updates for the streaming system as well
		//AdaptiveAssetManagerObjectUpdateEvent Event;

		// Loop through active primitives and process updates
		for( auto It = m_ActiveRenderComponents.begin(); It != m_ActiveRenderComponents.end(); )
		{
			// Ensure this entry is valid
			if( !It->second || !It->second->IsActive() )
			{
				// Ensure this is removed from the renderer
				Console::WriteLine( "[ERROR] GameInstance: Invalid component found in render component list!" );
				It = m_ActiveRenderComponents.erase( It );
				continue;
			}

			// Check its state
			auto state = It->second->GetRenderState();
			if( state == RenderComponentState::Stale )
			{
				// If were stale, we need to (re)create the proxy
				if( It->second->PerformProxyCreation() )
				{
					It->second->m_RenderState = RenderComponentState::Clean;
				}
				else
				{
					Console::WriteLine( "[ERROR] GameInstance: Failed to (re)create a stale render component!" );
				}
			}
			else if( state == RenderComponentState::Dirty )
			{
				if( It->second->PerformProxyUpdate() )
				{
					It->second->m_RenderState = RenderComponentState::Clean;

					// Create update entry for this component
					//AdaptiveAssetManagerObjectUpdateEntry Entry;

					//Entry.Identifier	= It->second->GetIdentifier();
					//Entry.Position		= It->second->GetPosition();
					//Entry.Radius		= 10.f; // TODO TODO TODO 

					//Event.Entries.push_back( Entry );
				}
				else
				{
					Console::WriteLine( "[ERROR] GameInstance: Failed to update a dirty proxy!" );
				}
			}

			It++;
		}

		// Next, we want to get the current view state
		HYPERION_VERIFY( m_LocalPlayer && m_LocalPlayer->IsValid(), "[GAME] LocalPlayer was null!" );
		
		// Update renderer view state if the camera info has changed since last tick
		Transform cameraTransform{};
		m_LocalPlayer->GetActiveCameraTransform( cameraTransform );

		// Calculate the field of view
		// 0.0 = pi/8, 0.5 = pi/4, 1 = 3pi/8
		float fovValue = ( Math::PIf * ( g_CVar_FOV.GetValue() + 0.5f ) ) / 4.f;

		if( cameraTransform != m_LastTickCameraTransform || fovValue != m_LastTickCameraFOV )
		{
			ViewState newView;
			newView.Position	= cameraTransform.Position;
			auto cameraEuler	= cameraTransform.Rotation.GetEulerAngles();
			newView.Rotation	= Quaternion( Angle3D( cameraEuler.Pitch, cameraEuler.Yaw, 0.f ) );
			newView.FOV			= fovValue;

			m_LastTickCameraFOV			= fovValue;
			m_LastTickCameraTransform	= cameraTransform;

			Engine::GetRenderer()->AddCommand(
				std::make_unique< UpdateViewStateCommand >( newView )
			);
		}
	
	}


	bool GameInstance::AddEntityToActiveWorld( const HypPtr< Entity >& inEnt, const Transform& inTransform )
	{
		if( m_ActiveWorld && m_ActiveWorld->IsValid() )
		{
			return m_ActiveWorld->AddEntityAt( inEnt, inTransform );
		}

		return false;
	}


	bool GameInstance::RemoveEntityFromActiveWorld( const HypPtr< Entity >& inEnt )
	{
		if( m_ActiveWorld && m_ActiveWorld->IsValid() )
		{
			return m_ActiveWorld->RemoveEntity( inEnt );
		}

		return false;
	}
	*/
}


/*
*	Register Type
*/
HYPERION_REGISTER_OBJECT_TYPE( GameInstance, Object );