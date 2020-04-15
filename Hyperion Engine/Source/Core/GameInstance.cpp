/*==================================================================================================
	Hyperion Engine
	Source/Core/GameInstance.cpp
	© 2019, Zachary Berry
==================================================================================================*/


#include "Hyperion/Core/GameInstance.h"
#include "Hyperion/Framework/World.h"
#include "Hyperion/Core/RenderManager.h"
#include "Hyperion/Framework/RenderComponent.h"
#include "Hyperion/Framework/PrimitiveComponent.h"
#include "Hyperion/Framework/LightComponent.h"
#include "Hyperion/Framework/CameraComponent.h"
#include "Hyperion/Framework/LocalPlayer.h"
#include "Hyperion/Framework/Player.h"


namespace Hyperion
{

	GameInstance::GameInstance()
		: m_ActiveWorld( nullptr ), m_LocalPlayer( nullptr ), m_LastTickScreenHeight( 0 )
	{
		
	}

	GameInstance::~GameInstance()
	{
	
	}

	void GameInstance::Initialize()
	{
		// We need to create the local player instance
		m_LocalPlayer = CreateObject< LocalPlayer >();

		// DEBUG
		auto newWorld = CreateObject< World >();
		if( SetActiveWorld( newWorld ) )
		{
			Console::WriteLine( "[DEBUG] GameInstance: Default world set!" );
		}
		else
		{
			Console::WriteLine( "[DEBUG] GameInstance: Default world couldnt be loaded!" );
		}

		// MORE DEBUG
		if( newWorld->SpawnWorld() )
		{
			Console::WriteLine( "[DEBUG] GameInstance: Spawned default world" );
		}
		else
		{
			Console::WriteLine( "[DEBUG] GameInstance: Default world couldnt be spawned!" );
		}

		// MORE MORE DEBUG
		// Eventually, we need a world spawning system, and a joining system
		auto newPlayer = CreateObject< Player >( 1 );
		if( !newWorld->AddEntity( newPlayer ) )
		{
			Console::WriteLine( "[DEBUG] GameInstance: Failed to add player to world!" );
		}
		else
		{
			Console::WriteLine( "[DEBUG] GameInstance: Added player to world" );
			m_LocalPlayer->SetPlayerEntity( newPlayer );
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
		AdaptiveAssetManagerWorldResetEvent Event;
		RenderManager::GetStreamingManager().OnWorldReset( Event );


		m_ActiveWorld->m_bActive = false;
		m_ActiveWorld->OnSetDeactive();

		m_ActiveWorld.Clear();
		return true;
	}


	/*
		Hooks for derived classes
	*/
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
			Console::WriteLine( "[DEBUG] GameInstance: Registered primitive component!" );
			return true;
		}

		return false;
	}

	bool GameInstance::RegisterRenderComponent( const HypPtr< LightComponent >& inComp )
	{
		if( DoRegisterRenderComponent( inComp ) )
		{
			Console::WriteLine( "[DEBUG] GameInstance: Registered light component" );
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
		RenderManager::AddCommand( std::make_unique< RemovePrimitiveProxyCommand >( inComp->GetIdentifier() ) );
		
		// DEBUG
		Console::WriteLine( "[DEBUG] GameInstance: Removed primitive component from list/renderer" );
		return true;
	}


	bool GameInstance::RemoveRenderComponent( const HypPtr< LightComponent >& inComp )
	{
		if( !DoRemoveRenderComponent( inComp ) )
		{
			return false;
		}

		// Add render command to actually remove this component
		RenderManager::AddCommand( std::make_unique< RemoveLightProxyCommand >( inComp->GetIdentifier() ) );

		// DEBUG
		Console::WriteLine( "[DEBUG] GameInstance: Removed light component from list/renderer" );
		return true;
	}


	void GameInstance::ProcessRenderUpdates()
	{
		// We need to accumulate updates for the streaming system as well
		AdaptiveAssetManagerObjectUpdateEvent Event;

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
				if( It->second->UpdateProxy() )
				{
					It->second->m_RenderState = RenderComponentState::Clean;

					// Create update entry for this component
					AdaptiveAssetManagerObjectUpdateEntry Entry;

					Entry.Identifier	= It->second->GetIdentifier();
					Entry.Position		= It->second->GetPosition();
					Entry.Radius		= 10.f; // TODO TODO TODO 

					Event.Entries.push_back( Entry );
				}
				else
				{
					Console::WriteLine( "[ERROR] GameInstance: Failed to update a dirty proxy!" );
				}
			}

			It++;
		}

		// Next, we want to get the current view state
		if( m_LocalPlayer && m_LocalPlayer->IsValid() )
		{
			ViewState vs;
			uint32 screenHeight = RenderManager::GetActiveResolution().Height;

			if( m_LocalPlayer->GetViewState( vs ) || screenHeight != m_LastTickScreenHeight )
			{
				// This means the view state has changed
				// First, lets inform the renderer of the updated view state
				RenderManager::AddCommand( std::make_unique< UpdateViewStateCommand >( vs ) );

				// Next, lets inform the streaming manager of the updated view
				AdaptiveAssetManagerCameraUpdateEvent CameraEvent;

				CameraEvent.CameraInfo.FOV	= vs.FOV;
				CameraEvent.CameraInfo.Position = vs.Position;
				CameraEvent.CameraInfo.ScreenHeight = screenHeight;

				m_LastTickScreenHeight = screenHeight;
				RenderManager::GetStreamingManager().OnCameraUpdate( CameraEvent );
			}
		}
		else
		{
			// DEBUG
			Console::WriteLine( "[ERROR] GameInstance: Failed to update renderer.. local player instance was null?" );
		}

		// Send event to streaming system to update the position/size of all objects
		RenderManager::GetStreamingManager().OnPrimitiveUpdate( Event );
	
	}


	bool GameInstance::AddEntityToActiveWorld( const HypPtr< Entity >& inEnt, const Transform3D& inTransform )
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

	/*
	bool GameInstance::SetActiveCamera( const HypPtr< CameraComponent >& inCamera )
	{
		if( !inCamera || !inCamera->IsValid() )
		{
			Console::WriteLine( "[ERROR] GameInstance: Attempt to set  invalid camera component as the active scene camera" );
			return false;
		}

		m_ActiveCamera = inCamera;
		uint32 cameraIdentifier = m_ActiveCamera->GetIdentifier();

		// Add render command to update the active camera in the proxy scene
		HYPERION_RENDER_COMMAND(
			[ cameraIdentifier ] ( Renderer& inRenderer )
			{
				auto scene = inRenderer.GetScene();
				if( scene )
				{
					scene->SetActiveCamera( cameraIdentifier );
				}
				else
				{
					Console::WriteLine( "[ERROR] Renderer: Failed to set active camera.. scene was null!" );
				}
			}
		);

		return true;
	}
	*/

}