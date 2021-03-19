/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Managers/ThreadManager.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Object.h"


namespace Hyperion
{
	enum class ThreadTickBehavior
	{
		// ClockSynced
		// On thread start, we create an OS clock and we only tick the thread on clock ticks, kind of like VSync
		// Once the tick function returns, we call it again on the next tick signal from the OS clock
		ClockSynced = 0,

		// RateLimited
		// We time the earliest possible next tick based on when the last tick was called
		// So basically, at the start of a tick, we take the current time and add the target tick time, then, if we arent at or past that time when the tick function returns
		// then we pause the current thread until we get there. If the current tick function runs past this time, the next tick is called immediatley after the current one returns
		RateLimited = 1
	};

	struct ThreadParameters
	{
		std::string identifier;;
		std::function< void() > initFunc;
		std::function< void( const std::atomic< bool >& ) > mainFunc;
		std::function< void() > shutdownFunc;
		bool bIsTicked;
		bool bAutoStart;
		double ticksPerSecond;
		ThreadTickBehavior tickType;
	};


	class Thread : public Object
	{

	protected:

		std::unique_ptr< std::thread > m_Handle;
		std::atomic< bool > m_State;
		ThreadParameters m_Params;

		Thread() = delete;

	public:

		Thread( const ThreadParameters& inParams )
			: m_Params( inParams )
		{}

		bool Start();
		bool Stop();

		const ThreadParameters& GetParameters() const { return m_Params; }
		std::string GetIdentifier() const { return m_Params.identifier; }
		bool IsRunning() const { return m_State; }

		std::thread::id GetSystemIdentifier() const
		{
			if( m_Handle )
			{
				return m_Handle->get_id();
			}

			return std::thread::id();
		}

		friend void _tickedThreadBody( Thread& );
		friend void _normalThreadBody( Thread& );
	};


	class ThreadManager
	{

	private:

		static std::map< std::string, HypPtr< Thread > > m_Threads;
		static std::mutex m_ThreadMutex;
		static std::atomic< bool > m_bIsShutdown;
		static std::atomic< uint32 > m_ThreadCount;
		static bool m_bInit;
		static bool m_bClosed;
		static float m_MinResolution;

	public:

		ThreadManager() = delete;

		static bool Initialize();
		static bool Shutdown();

		static HypPtr< Thread > CreateThread( ThreadParameters& inParams );
		static HypPtr< Thread > GetThread( const std::string& );
		static bool DestroyThread( const std::string& );
		static uint32 GetThreadCount();

	};

}