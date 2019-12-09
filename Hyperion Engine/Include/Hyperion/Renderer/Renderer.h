/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Renderer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Object.h"
#include "Hyperion/Core/Threading.h"
#include "Hyperion/Core/Library/Mutex.hpp"
#include "Hyperion/Core/String.h"

#ifdef HYPERION_OS_WIN32
#include <Windows.h>
#endif


namespace Hyperion
{
	class ProxyScene;


	struct ScreenResolution
	{
		uint32 Width;
		uint32 Height;
		bool FullScreen;
	};

	class Renderer : public Object
	{

	protected:

		bool m_isRunning;

		std::shared_ptr< Thread > m_renderThread;

		void PerformInit();
		void PerformTick();
		void PerformShutdown();

		virtual bool Init()			= 0;
		virtual void Frame()		= 0;
		virtual void Shutdown()		= 0;

		void UpdateProxyScene();

	public:

		Renderer();
		~Renderer();

		bool Start();
		void Stop();

		inline bool IsRunning() const { return m_isRunning && m_renderThread; }

		virtual bool SetScreenResolution( const ScreenResolution& ) = 0;
		virtual ScreenResolution GetScreenResolution() = 0;
		virtual std::vector< ScreenResolution > GetAvailableResolutions() = 0;

#ifdef HYPERION_OS_WIN32
		virtual bool SetRenderTarget( HWND ) = 0;
		virtual HWND GetRenderTarget() = 0;
#endif

		virtual void SetVSync( bool ) = 0;
		virtual bool GetVSyncEnabled() = 0;

		virtual String GetVideoCardDescription() = 0;
		virtual uint32 GetVideoCardMemory() = 0;

		/*
			Proxy Scene
		*/
	private:

		std::shared_ptr< ProxyScene > m_Scene;

	public:

		std::weak_ptr< ProxyScene > GetScene() { return std::weak_ptr< ProxyScene >( m_Scene ); }

	};

}