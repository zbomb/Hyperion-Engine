/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Engine.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

// Hyperion 
#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Object.h"
#include "Hyperion/Core/Threading.h"
#include "Hyperion/Core/InputManager.h"
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Core/String.h"

// STL
#include <type_traits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#ifdef HYPERION_DEBUG_OBJECT
#include <iostream>
#endif


namespace Hyperion
{
	/*
		Typedefs
	*/
	//typedef std::map< ObjectID, std::weak_ptr< Object > > ObjectCache;

	class Entity;
	class World;
	class InputManager;


	enum class EngineStatus
	{
		NotStarted,
		Init,
		Running,
		Shutdown
	};

	struct GameInfo
	{
		String DisplayName;
		String FolderName;
		String AuthorName;
		String AuthorContact;
		uint8 MajorVersion;
		uint8 MinorVersion;
		uint8 BuildNumber;
	};

	/*
		Engine Declaration
	*/
	class Engine
	{

		/*
			Singleton Access Pattern
		*/
	private:

		static std::unique_ptr< Engine > m_Instance;

	public:

		template< typename _EngTy >
		static Engine& CreateInstance()
		{
			if( m_Instance )
			{
				throw std::runtime_error( "Attempt to create engine instance, when one already exists!" );
			}

			m_Instance = std::make_unique< _EngTy >();
			return *m_Instance;
		}

		static Engine& GetInstance()
		{
			if( !m_Instance )
			{
				throw std::runtime_error( "Attempt to get engine singleton before initialization!" );
			}

			return *m_Instance;
		}

	public:

		Engine();
		~Engine();

		Engine( Engine const& ) = delete;
		void operator=( Engine const& ) = delete;

		/*
			Engine Info
		*/
		virtual String GetGameName() const = 0;
		virtual String GetDocumentsFolderName() const = 0;
		virtual String GetAuthorName() const = 0;
		virtual String GetVersionName() const = 0;

		virtual void OnInitialize() {}
		virtual void OnShutdown() {}

		virtual void RegisterAssetLoaders();

		/*
			Thread Manager
		*/
	private:

		HypPtr< ThreadManager > m_ThreadManager;

	public:

		inline const HypPtr< ThreadManager >& GetThreadManager() const { return m_ThreadManager; }

		/*
			Initialization System
		*/
	private:

		EngineStatus m_Status;
		std::chrono::time_point< std::chrono::high_resolution_clock > m_lastTick;

		InputManager m_InputManager;

		void InitEngine();
		void TickEngine();
		void ShutdownEngine();

	public:

		inline EngineStatus GetStatus() const { return m_Status; }

		bool Startup();
		bool Shutdown();

		inline InputManager& GetInputManager() { return m_InputManager; }

		/*=======================================================================================================
				Engine Initialization
		=======================================================================================================*/
	private:

		HypPtr< World > m_ActiveWorld;;

	public:

		inline const HypPtr< World >& GetWorld() const { return m_ActiveWorld; }

		bool SetActiveWorld( const HypPtr< World >& inWorld );
		bool ClearActiveWorld();

		/*=======================================================================================================
				Render System
		=======================================================================================================*/
	private:

		std::unique_ptr< RenderFenceWatcher > m_FenceWatcher;
		IRenderOutput m_RenderOutput;

	public:

		bool SetRenderOutput( const IRenderOutput& Output )
		{
			if( !Output )
			{
				std::cout << "[ERROR] Engine: Attempt to set invalid render output!\n";
				return false;
			}

			m_RenderOutput = Output;
			return true;
		}

		const IRenderOutput& GetRenderOutput()
		{
			return m_RenderOutput;
		}

	};

}