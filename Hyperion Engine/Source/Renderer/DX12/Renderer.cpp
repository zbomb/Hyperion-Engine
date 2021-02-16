/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Renderer.cpp
	© 2021, Zachary Berry
==================================================================================================*/

/*
*	Header Includes
*/
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/Proxy/ProxyScene.h"
#include "Hyperion/Renderer/Proxy/ProxyPrimitive.h"
#include "Hyperion/Renderer/Proxy/ProxyLight.h"
#include "Hyperion/Core/ThreadManager.h"


namespace Hyperion
{
	/*-----------------------------------------------------------------------------------
		Screen Resolution Function Definitions
	-----------------------------------------------------------------------------------*/
	bool ScreenResolution::LoadFromString( const String& inStr )
	{
		// We want to break it out into two components, seperated by a comma
		String trimmedStr = inStr.TrimBoth();
		std::vector< String > explodedStr = trimmedStr.Explode( ',' );

		if( explodedStr.size() != 2 )
		{
			return false;
		}

		uint32 strWidth = 0;
		uint32 strHeight = 0;

		if( !explodedStr[ 0 ].ToUInt( strWidth ) ||
			!explodedStr[ 1 ].ToUInt( strHeight ) )
		{
			return false;
		}

		if( strWidth > 420 && strHeight > 360 )
		{
			Width = strWidth;
			Height = strHeight;

			return true;
		}
		else
		{
			return false;
		}
	}


	/*-----------------------------------------------------------------------------------
		Renderer Function Definitions
	-----------------------------------------------------------------------------------*/
	Renderer::Renderer()
	{
		// Create our proxy scene
		m_Scene = std::make_shared< ProxyScene >();
	}


	Renderer::~Renderer()
	{
		m_Scene.reset();
	}


	bool Renderer::Initialize( void* inWindow, const ScreenResolution& inResolution, bool bVSync )
	{
		HYPERION_VERIFY( m_Scene != nullptr, "[Renderer] Scene is null" );

		m_Scene->Initialize();
		return true;
	}


	void Renderer::Shutdown()
	{
		HYPERION_VERIFY( m_Scene != nullptr, "[Renderer] Scene is null" );

		m_Scene->Shutdown();
	}


	void Renderer::Tick()
	{
		// First, we need to actually update the proxy scene, through our renderer commands
		UpdateScene();

		// Call derived function to actually render the scene
		Frame();
	}


	void Renderer::UpdateScene()
	{
				// Execute all immediate commands
		// Were going to run them until the list is empty
		auto nextImmediateCommand = m_ImmediateCommands.PopValue();
		while( nextImmediateCommand.first )
		{
			// Execute
			if( nextImmediateCommand.second )
			{
				nextImmediateCommand.second->Execute( *this );
			}

			// Pop next command
			nextImmediateCommand = m_ImmediateCommands.PopValue();
		}

		// For now, were just going to run the next frame of commands
		auto nextCommand = m_Commands.PopValue();
		while( nextCommand.first )
		{
			// Execute this command
			if( nextCommand.second )
			{
				nextCommand.second->Execute( *this );
			}

			// Check if this is the end of the frame, by the EOF flag
			if( nextCommand.second->HasFlag( RENDERER_COMMAND_FLAG_END_OF_FRAME ) )
			{
				break;
			}

			// Pop the next command in the list
			nextCommand = m_Commands.PopValue();
		}

		// Were going to loop through our primitives and cache the batches for the renderer
		for( auto it = m_Scene->PrimitivesBegin(); it != m_Scene->PrimitivesEnd(); it++ )
		{
			if( it->second )
			{
				// First, lets update the world matrix if needed, along with the OBB
				if( it->second->m_bMatrixDirty )
				{
					//m_API->CalculateWorldMatrix( it->second->GetTransform(), it->second->m_WorldMatrix );
					//m_API->TransformAABB( it->second->GetTransform(), it->second->GetAABB(), it->second->m_OrientedBounds );
					//it->second->m_bMatrixDirty = false;
				}

				if( it->second->m_bCacheDirty )
				{
					//it->second->CacheBatches();
				}

			}
		}
	}

	void Renderer::ShutdownProxy( const std::shared_ptr< ProxyBase >& inBase )
	{
		if( inBase )
		{
			// Call begin shutdown directly on render thread
			inBase->BeginShutdown();

			// Create task on pool to finalize shutdown of the proxy so we dont block the render thread
			ThreadManager::CreateTask< void >(
				[ inBase ] ()
				{
					if( inBase )
					{
						inBase->Shutdown();
					}

				} );
		}
	}


	bool Renderer::AddPrimitive( const std::shared_ptr< ProxyPrimitive >& inPrimitive )
	{
		HYPERION_VERIFY( m_Scene != nullptr, "[Renderer] Scene was null" );

		if( !inPrimitive )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to add primitive, it was null" );
			return false;
		}

		// Check if this primitive is already in the scene
		auto ptr = m_Scene->RemovePrimitive( inPrimitive->GetIdentifier() );
		if( ptr != nullptr )
		{
			ShutdownProxy( ptr );
		}

		// Add primitive to the scene
		if( m_Scene->AddPrimitive( inPrimitive ) )
		{
			inPrimitive->RenderInit();
			return true;
		}
		else
		{
			return false;
		}

	}


	bool Renderer::AddLight( const std::shared_ptr< ProxyLight >& inLight )
	{
		HYPERION_VERIFY( m_Scene != nullptr, "[Renderer] Scene was null" );

		if( !inLight )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to add light, it was null" );
			return false;
		}

		// Check if this light is already in the scene
		auto ptr = m_Scene->RemoveLight( inLight->GetIdentifier() );
		if( ptr != nullptr )
		{
			ShutdownProxy( ptr );
		}

		// Add light to scene
		if( m_Scene->AddLight( inLight ) )
		{
			inLight->RenderInit();
			return true;
		}
		else
		{
			return false;
		}
	}


	bool Renderer::RemovePrimitive( uint32 inIdentifier )
	{
		HYPERION_VERIFY( m_Scene != nullptr, "[Renderer] Scene was null" );

		// Remove primitive from the scene and shut it down
		auto ptr = m_Scene->RemovePrimitive( inIdentifier );
		if( ptr != nullptr )
		{
			ShutdownProxy( ptr );
			return true;
		}

		return false;
	}


	bool Renderer::RemoveLight( uint32 inIdentifier )
	{
		HYPERION_VERIFY( m_Scene != nullptr, "[Renderer] Scene was null" );

		// Remove light from the scene and shut it down
		auto ptr = m_Scene->RemoveLight( inIdentifier );
		if( ptr != nullptr )
		{
			ShutdownProxy( ptr );
			return true;
		}

		return false;
	}


	/*
	*	Renderer::AddImmediateCommand
	*/
	void Renderer::AddImmediateCommand( std::unique_ptr< RenderCommandBase >&& inCommand )
	{
		if( !m_bAllowCommands )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to push immediate command, the queue was closed" );
			return;
		}

		m_ImmediateCommands.Push( std::move( inCommand ) );
	}

	/*
	*	Renderer::AddCommand
	*/
	void Renderer::AddCommand( std::unique_ptr< RenderCommandBase >&& inCommand )
	{
		if( !m_bAllowCommands )
		{
			Console::WriteLine( "[Warning] Renderer: Failed to push command, the queue was closed" );
			return;
		}

		m_Commands.Push( std::move( inCommand ) );
	}




}