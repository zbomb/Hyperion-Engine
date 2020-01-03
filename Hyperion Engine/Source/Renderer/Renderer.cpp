/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Renderer.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/Proxy/ProxyScene.h"
#include "Hyperion/Renderer/Proxy/ProxyPrimitive.h"
#include "Hyperion/Renderer/Proxy/ProxyLight.h"
#include "Hyperion/Renderer/Proxy/ProxyCamera.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Core/Async.h"
#include <iostream>



namespace Hyperion
{


	/*
		Render Thread
	*/
	bool TRenderThread::Start()
	{
		// Check if were already running
		if(  m_Thread )
		{
			std::cout << "[ERROR] Render Thread: Can not start render thread when its already running!\n";
			return false;
		}

		auto threadManager = Engine::GetInstance().GetThreadManager();
		if( !threadManager )
		{
			std::cout << "[ERROR] Render Thread: Failed to start.. couldnt access thread manager!\n";
			return false;
		}

		// Ensure there are no other threads running with our identifier
		threadManager->DestroyThread( THREAD_RENDERER );

		// Setup the parameters for our thread
		TickedThreadParameters params;

		params.Identifier			= THREAD_RENDERER;
		params.AllowTasks			= true;
		params.Deviation			= 0.f;
		params.Frequency			= 60.f;
		params.MinimumTasksPerTick	= 3;
		params.MaximumTasksPerTick	= 0;
		params.StartAutomatically	= true;

		params.InitFunction			= std::bind( &TRenderThread::Init );
		params.ShutdownFunction		= std::bind( &TRenderThread::Shutdown );
		params.TickFunction			= std::bind( &TRenderThread::Tick );

		m_Thread = threadManager->CreateThread( params );
		if( !m_Thread )
		{
			std::cout << "[ERROR] Render Thread: Failed to start.. couldnt create the thread!\n";
			return false;
		}

		__gRenderThreadId = m_Thread->GetSystemIdentifier();
		return true;
	}

	bool TRenderThread::Stop()
	{
		if( !m_Thread )
		{
			std::cout << "[ERROR] Render Thread: Attempt to stop renderer.. but the render thread is not running!\n";
			return false;
		}

		// Stop and destroy the thread
		m_Thread->Stop();

		auto threadManager = Engine::GetInstance().GetThreadManager();
		if( !threadManager )
		{
			std::cout << "[ERROR] Render Thread: Failed to completley stop renderer.. couldnt access the thread manager to delete the stopped thread!\n";
			m_Thread.reset();

			return false;
		}

		threadManager->DestroyThread( THREAD_RENDERER );
		return true;
	}

	bool TRenderThread::IsRunning()
	{
		return m_Thread ? true : false;
	}

	void TRenderThread::Init()
	{
		std::cout << "[STATE] Render Thread Initializing...\n";
	}

	void TRenderThread::Shutdown()
	{
		std::cout << "[STATE] Render Thread Shutting Down...\n";
	}

	void TRenderThread::Tick()
	{

	}





	Renderer::Renderer()
		: m_isRunning( false )
	{
	}


	Renderer::~Renderer()
	{
		// Shutdown the scene
		if( m_Scene )
		{
			m_Scene->Shutdown();
			m_Scene.reset();
		}

		// Shutdown the thread
		if( m_renderThread )
		{
			m_renderThread->Stop();
		}

		auto& eng = Engine::GetInstance();
		auto tm = eng.GetThreadManager();

		if( tm )
			tm->DestroyThread( THREAD_RENDERER );

		m_isRunning = false;
	}


	bool Renderer::Start()
	{
		auto& eng = Engine::GetInstance();
		auto tm = eng.GetThreadManager();

		if( !tm )
		{
			std::cout << "[ERROR] Renderer: Couldnt initialize.. thread manager was null\n";
			return false;
		}

		// Check if were already running
		if( m_isRunning )
		{
			std::cout << "[ERROR] Renderer: Couldnt initialize.. already running\n";
			return false;
		}

		// If the render thread is still running.. stop it
		tm->DestroyThread( THREAD_RENDERER );

		// Create our new render thread
		TickedThreadParameters params;

		params.Identifier			= THREAD_RENDERER;
		params.AllowTasks			= true;
		params.Deviation			= 0.f;
		params.Frequency			= 60.f;
		params.MinimumTasksPerTick	= 3;
		params.MaximumTasksPerTick	= 0;
		params.StartAutomatically	= true;
		
		params.InitFunction			= std::bind( &Renderer::PerformInit, this );
		params.TickFunction			= std::bind( &Renderer::PerformTick, this );
		params.ShutdownFunction		= std::bind( &Renderer::PerformShutdown, this );

		m_renderThread = tm->CreateThread( params );
		if( !m_renderThread )
		{
			std::cout << "[ERROR] Renderer: Couldnt initialize.. failed to create render thread\n";
			return false;
		}

		m_isRunning			= true;
		__gRenderThreadId	= m_renderThread->GetSystemIdentifier();

		return true;
	}


	void Renderer::Stop()
	{
		if( !m_isRunning && !m_renderThread )
		{
			std::cout << "[WARNING] Renderer: Attempt to stop renderer.. but it wasnt running!\n";
			return;
		}

		m_isRunning = false;
		if( m_renderThread )
		{
			m_renderThread->Stop();
		}

		auto& eng = Engine::GetInstance();
		auto tm = eng.GetThreadManager();

		if( !tm )
		{
			std::cout << "[ERROR] Renderer: Failed to properly shutdown.. couldnt access thread manager to destroy thread!\n";
			return;
		}

		tm->DestroyThread( THREAD_RENDERER );

	}


	void Renderer::PerformInit()
	{
		std::cout << "[DEBUG] Renderer: Performing init...\n";

		// Reset the scene
		if( m_Scene )
		{
			m_Scene->Shutdown();
			m_Scene.reset();
		}

		m_Scene = std::make_shared< ProxyScene >();
		m_Scene->Initialize();

		// Initialize derived renderer
		if( !Init() )
		{
			std::cout << "[ERROR] Renderer: Failed to initialize!\n";
		}
	}

	void Renderer::PerformTick()
	{
		UpdateProxyScene();
		Frame();
	}

	void Renderer::PerformShutdown()
	{
		std::cout << "[DEBUG] Renderer: Performing shutdown...\n";

		if( m_Scene )
		{
			m_Scene->Shutdown();
			m_Scene.reset();
		}

		// Shutdown derived renderer
		Shutdown();
	}

	void Renderer::UpdateProxyScene()
	{
		// For now, were just going to run the next frame of commands
		auto nextCommand = m_CommandQueue.PopValue();
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
		}
	}


	void Renderer::PushCommand( std::unique_ptr< RenderCommandBase >&& inCommand )
	{
		// Move the command into the queue
		m_CommandQueue.Push( std::move( inCommand ) );
	}


	bool Renderer::AddPrimitive( std::shared_ptr< ProxyPrimitive > inPrimitive )
	{
		if( m_Scene && inPrimitive )
		{
			inPrimitive->Render_Init();
			return m_Scene->AddPrimitive( inPrimitive );
		}

		return false;
	}

	bool Renderer::AddLight( std::shared_ptr< ProxyLight > inLight )
	{
		if( m_Scene && inLight )
		{
			inLight->Render_Init();
			return m_Scene->AddLight( inLight );
		}

		return false;
	}

	bool Renderer::AddCamera( std::shared_ptr< ProxyCamera > inCamera )
	{
		if( m_Scene && inCamera )
		{
			inCamera->Render_Init();
			return m_Scene->AddCamera( inCamera );
		}

		return false;
	}

	bool Renderer::RemovePrimitive( std::weak_ptr< ProxyPrimitive > inPrimitive, std::function< void() > inCallback )
	{
		if( m_Scene && !inPrimitive.expired() )
		{
			// Try to remove this primitive from the map
			uint32 Identifier = 0;
			{
				auto ptr = inPrimitive.lock();
				Identifier = ptr ? ptr->GetIdentifier() : 0;
			}

			if( Identifier != 0 )
			{
				auto removedPtr = m_Scene->RemovePrimitive( Identifier );
				if( removedPtr )
				{
					removedPtr->BeginShutdown();
					auto t = Task::Create< void >( [removedPtr, inCallback] ()
						{
							removedPtr->Shutdown();
							if( inCallback )
							{
								inCallback();
							}
						} );

					return true;
				}
			}
		}

		std::cout << "[WARNING] ProxySystem: Failed to remove a primitive proxy from the scene!\n";
		return false;
	}

	bool Renderer::RemoveLight( std::weak_ptr< ProxyLight > inLight, std::function< void() > inCallback )
	{
		if( m_Scene && !inLight.expired() )
		{
			uint32 Identifier = 0;
			{
				auto ptr = inLight.lock();
				Identifier = ptr ? ptr->GetIdentifier() : 0;
			}

			if( Identifier != 0 )
			{
				auto removedPtr = m_Scene->RemoveLight( Identifier );
				if( removedPtr )
				{
					removedPtr->BeginShutdown();
					auto t = Task::Create< void >( [removedPtr, inCallback] ()
						{
							removedPtr->Shutdown();
							if( inCallback )
							{
								inCallback();
							}
						} );

					return true;
				}
			}
		}

		std::cout << "[WARNING] ProxySystem: Failed to remove a light proxy from the scene!\n";
		return false;
	}

	bool Renderer::RemoveCamera( std::weak_ptr< ProxyCamera > inCamera, std::function< void() > inCallback )
	{
		if( m_Scene && !inCamera.expired() )
		{
			uint32 Identifier = 0;
			{
				auto ptr = inCamera.lock();
				Identifier = ptr ? ptr->GetIdentifier() : 0;
			}

			if( Identifier != 0 )
			{
				auto removedPtr = m_Scene->RemoveCamera( Identifier );
				if( removedPtr )
				{
					removedPtr->BeginShutdown();
					auto t = Task::Create< void >( [removedPtr, inCallback] ()
						{
							removedPtr->Shutdown();
							if( inCallback )
							{
								inCallback();
							}
						} );

					return true;
				}
			}
		}

		std::cout << "[WARNING] ProxySystem: Failed to remove a camera proxy from the scene!\n";
		return false;
	}


}