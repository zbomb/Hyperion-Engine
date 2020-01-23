/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Managers/GameManager.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/GameInstance.h"
#include "Hyperion/Core/ThreadManager.h"
#include "Hyperion/Core/InputManager.h"


namespace Hyperion
{
	struct RenderFenceWatcher;

	class InstanceFactoryBase
	{
	public:

		virtual HypPtr< GameInstance > Create() = 0;
	};

	template< typename T >
	class InstanceFactory : public InstanceFactoryBase
	{

	public:

		HypPtr< GameInstance > Create() final
		{
			return CreateObject< T >();
		}

	};

	class GameManager
	{

	private:

		static HypPtr< GameInstance > m_Instance;
		static std::shared_ptr< Thread > m_Thread;
		static std::shared_ptr< InstanceFactoryBase > m_Factory;
		static std::unique_ptr< RenderFenceWatcher > m_FenceWatcher;
		static std::chrono::time_point< std::chrono::high_resolution_clock > m_LastTick;
		static InputManager m_InputManager;

		static void Init();
		static void Tick();
		static void Shutdown();

	public:

		GameManager() = delete;

		static bool Start( uint32 inFlags = 0, std::shared_ptr< InstanceFactoryBase > inFactory = nullptr );
		static bool Stop();

		static std::thread::id GetThreadId();
		static inline InputManager& GetInputManager() { return m_InputManager; }
		static inline HypPtr< GameInstance > GetInstance() { return m_Instance; }

		/*
		template< typename GM >
		static bool LoadLevel( const String& levelName );
		*/

	};

}