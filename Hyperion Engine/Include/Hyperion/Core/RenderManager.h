/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Managers/RenderManager.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Renderer.h"


#if HYPERION_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class Thread;
	struct RawImageData;


	class RenderManager
	{

	private:

		static std::shared_ptr< Thread > m_Thread;
		static std::shared_ptr< Renderer > m_Instance;

		static IRenderOutput m_OutputWindow;

		static void Init();
		static void Tick();
		static void Shutdown();

		static std::shared_ptr< IGraphics > CreateAPI( const String& inStr );
		

	public:

		RenderManager() = delete;

		static bool Start( const IRenderOutput& inOutput, uint32 inFlags = 0 );
		static bool Stop();

		static std::thread::id GetThreadId();

		static bool IsRunning();
		inline static const IRenderOutput& GetOutputTarget() { return m_OutputWindow; }

		static void AddCommand( std::unique_ptr< RenderCommandBase >&& inCommand );

		static void OnResolutionUpdated();
		static void OnVSyncUpdated();

		static ScreenResolution ReadResolution( const String& inStr, uint32 inFullscreen, bool bPrintError = true );

		static std::shared_ptr< ITexture2D > Load2DTexture( const std::shared_ptr< RawImageData >& inData );

	};

}