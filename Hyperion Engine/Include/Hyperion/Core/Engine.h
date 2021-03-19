/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Engine.h
	� 2020, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Core/Object.h"
#include "Hyperion/Renderer/DataTypes.h" // For ScreenResolution structure


namespace Hyperion
{
	/*
	*	Forward Decl.
	*/
	class GameInstance;
	class Simulation;
	class Renderer;


	class Engine
	{

	public:

		/*
		*	Structures
		*/
		struct InitializeParameters
		{
			GraphicsAPI targetAPI;
			void* ptrOutputSurface;
			uint32 desiredResX;
			uint32 desiredResY;
			bool bStartFullscreen;
			bool bVSync;
			uint32 numTaskPoolThreads;
			uint32 startupFlags;
			std::string instanceType;
		};

		enum class InitializeResult
		{
			Success						= 0,
			AlreadyInitialized			= 1,
			InvalidOutputSurface		= 2,
			InvalidResolution			= 3,
			InvalidGraphicsAPI			= 4,
			InvalidNumTaskPoolThreads	= 5,
			ServicesFailed				= 6,
			InvalidInstanceType			= 7
		};

	private:

		bool m_bInitialized;

		HypPtr< GameInstance > m_GameInstance;
		const std::shared_ptr< GarbageCollector > m_GC;

	public:

		/*
		*	Static Functions
		*/
		static Engine& Get();

		/*
		*	Constructor/Destructor
		*/
		Engine();
		~Engine();

		/*
		*	Deleted Functions
		*/
		Engine( const Engine& ) = delete;
		void operator=( const Engine& ) = delete;

		/*
		*	Init & Shutdown
		*	Note: These functions should only be called from the main OS thread!
		*/
		InitializeResult Initialize( const InitializeParameters& inParams );
		void Shutdown();

		/*
		*	Getters
		*/
		std::shared_ptr< GarbageCollector > GetGC() const	{ return m_GC; }
		HypPtr< GameInstance > GetGameInstance() const		{ return m_GameInstance; }


	};


	//////////////////////////////////////////////// OLD //////////////////////////////////////////////////
	/*
	*	Forward Declarations
	
	class GameInstance;
	class Renderer;
	class InputManager;
	class Thread;


	class Engine
	{

	private:

		HypPtr< GameInstance > m_Game;
		HypPtr< InputManager > m_Input;

		std::shared_ptr< Renderer > m_Renderer;

		HypPtr< Thread > m_GameThread;
		HypPtr< Thread > m_RenderThread;

		bool m_bServicesRunning;

		std::chrono::time_point< std::chrono::high_resolution_clock > m_LastGameTick;
		std::chrono::time_point< std::chrono::high_resolution_clock > m_LastRenderTick;

		std::unique_ptr< RenderFenceWatcher > m_FenceWatcher;

		bool m_bGameInit;
		bool m_bRenderInit;

		std::mutex m_GameWaitMutex;
		std::mutex m_RenderWaitMutex;

		std::condition_variable m_GameWaitCondition;
		std::condition_variable m_RenderWaitCondition;

		static std::function< void( const String& ) > s_FatalErrorCallback;
		static std::atomic< bool > s_bFatalError;

		static std::atomic< bool > s_bSuspended;

		// Renderer Init Helpers
		void DoRenderThreadInit( void* pWindow, ScreenResolution inResolution, uint32 inFlags );
		void DoRenderThreadShutdown();
		void DoRenderThreadTick();

		void DoGameThreadInit();
		void DoGameThreadTick();
		void DoGameThreadShutdown();

		void ShutdownRenderer();
		void ShutdownGame();
		void ShutdownServices();

	public:

		Engine();
		~Engine();

		inline HypPtr< Thread > GetGameThread() const { return m_GameThread; }
		inline HypPtr< Thread > GetRenderThread() const { return m_RenderThread; }
		inline HypPtr< GameInstance > GetGameInstancePtr() const { return m_Game; }
		inline std::shared_ptr< Renderer > GetRendererPtr() const { return m_Renderer; }
		inline HypPtr< InputManager > GetInputManagerPtr() const { return m_Input; }

		bool InitializeServices( uint32 inFlags = FLAG_NONE );
		bool InitializeRenderer( void* pWindow, ScreenResolution inResolution, uint32 inFlags = FLAG_NONE );
		bool InitializeGame( uint32 inFlags = FLAG_NONE );

		static void FatalError( const String& inDescription );
		static void SetFatalErrorCallback( std::function< void( const String& ) > inCallback ) { s_FatalErrorCallback = inCallback; }

		void Stop();

		ScreenResolution GetStartupScreenResolution();
		void OnResolutionUpdated();
		void OnVSyncUpdated();

		ScreenResolution GetResolution() const;
		bool IsVSyncOn() const;

		void Shutdown();
		static void Suspend();
		static void Resume();

		void WaitForInitComplete();

		static std::shared_ptr< Engine > Get();
		inline static std::shared_ptr< Renderer > GetRenderer() { return Get()->GetRendererPtr(); }
		inline static HypPtr< GameInstance > GetGame() { return Get()->GetGameInstancePtr(); }
		inline static HypPtr< InputManager > GetInputManager() { return Get()->GetInputManagerPtr(); }

	};
	*/
}