/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Renderer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Object.h"
#include "Hyperion/Core/Threading.h"
#include "Hyperion/Core/String.h"
#include "Hyperion/Renderer/IGraphics.h"
#include "Hyperion/Renderer/DataTypes.h"
#include "Hyperion/Core/Types/ConcurrentQueue.h"


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class ProxyScene;


	/*
		Renderer Instance
	*/
	class Renderer
	{

	protected:

		ScreenResolution m_Resolution;
		bool m_bVSync;
		const IRenderOutput m_Output;

		std::shared_ptr< IGraphics > m_API;
		std::shared_ptr< ProxyScene > m_Scene;

		ConcurrentQueue< std::unique_ptr< RenderCommandBase > > m_Commands;

		bool Initialize();
		void Shutdown();
		void Frame();

		void UpdateScene();

	public:

		Renderer() = delete;
		Renderer( const IRenderOutput&, const RenderSettings&, const std::shared_ptr< IGraphics >& );
		~Renderer();

		Renderer( const Renderer& ) = delete;
		Renderer( Renderer&& ) = delete;

		Renderer& operator=( const Renderer& ) = delete;
		Renderer& operator=( Renderer&& ) = delete;

		friend class TRenderSystem;

	};


	/*
		Render System Interface
	*/
	class TRenderSystem
	{

	private:

		static std::shared_ptr< Thread > m_Thread;
		static std::shared_ptr< Renderer > m_Renderer;
		static RenderSettings m_CurrentSettings;
		static std::atomic< bool > m_bIsRunning;

		static bool ValidateSettings( const RenderSettings& inSettings );

	public:

		static void Init();
		static void Shutdown();
		static void Tick();

		template< typename _API >
		static bool Start( const IRenderOutput& Target, const RenderSettings& inSettings )
		{
			// Check if were already running
			if( m_Thread || m_Renderer )
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

			// Validate startup parameters
			if( !Target )
			{
				std::cout << "[ERROR] Render System: Attempt to start renderer with invalid output window!\n";
				return false;
			}

			if( !ValidateSettings( inSettings ) )
			{
				std::cout << "[ERROR] Render System: Attempt to start renderer with invalid settings\n";
				return false;
			}

			// Ensure there are no other threads running with our identifier
			threadManager->DestroyThread( THREAD_RENDERER );

			// Create our renderer
			m_CurrentSettings = inSettings;
			m_Renderer = std::make_shared< Renderer >( Target, inSettings, std::make_shared< _API >() );

			// Setup the parameters for our thread
			TickedThreadParameters params;

			params.Identifier			= THREAD_RENDERER;
			params.AllowTasks			= true;
			params.Deviation			= 0.f;
			params.Frequency			= 60.f;
			params.MinimumTasksPerTick	= 3;
			params.MaximumTasksPerTick	= 0;
			params.StartAutomatically	= true;

			params.InitFunction			= std::bind( &TRenderSystem::Init );
			params.ShutdownFunction		= std::bind( &TRenderSystem::Shutdown );
			params.TickFunction			= std::bind( &TRenderSystem::Tick );

			m_Thread = threadManager->CreateThread( params );
			if( !m_Thread )
			{
				std::cout << "[ERROR] Render Thread: Failed to start.. couldnt create the thread!\n";
				return false;
			}

			__gRenderThreadId = m_Thread->GetSystemIdentifier();
			return true;
		}

		static bool Stop();
		static bool IsRunning();

		static void AddCommand( std::unique_ptr< RenderCommandBase >&& inCommand );

		//static bool Pause();
		//static bool Resume();

		//static bool UpdateResolution( const ScreenResolution& );
		//static bool UpdateDetailSettings( const DetailSettings& );
		//static bool UpdateOutputDevice( const IRenderOutput& );

		static const RenderSettings& GetCurrentSettings();

	};

}