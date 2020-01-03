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
#include "Hyperion/Core/Types/ConcurrentQueue.h"

#ifdef HYPERION_OS_WIN32
#include <Windows.h>
#endif


namespace Hyperion
{
	class PrimitiveComponent;
	class ProxyScene;
	struct ProxyBase;
	struct ProxyPrimitive;
	struct ProxyLight;
	struct ProxyCamera;
	class Renderer;

	constexpr uint32 RENDERER_COMMAND_FLAG_NONE				= 0b00000000;
	constexpr uint32 RENDERER_COMMAND_FLAG_UPDATE			= 0b00000001;
	constexpr uint32 RENDERER_COMMAND_FLAG_END_OF_FRAME		= 0b00000010;

	struct RenderCommandBase
	{
		uint32 Flags;
		virtual void Execute( Renderer& ) = 0;

		void EnableFlag( uint32 inFlag )
		{
			Flags |= inFlag;
		}

		bool HasFlag( uint32 inFlag )
		{
			return( ( Flags & inFlag ) > 0 );
		}

		void DisableFlag( uint32 inFlag )
		{
			Flags &= ~inFlag;
		}

		void ClearFlags()
		{
			Flags = 0;
		}
	};

	struct RenderCommand : public RenderCommandBase
	{
		std::function< void( Renderer& ) > Func;

		RenderCommand() = delete;
		RenderCommand( std::function< void( Renderer& ) > inFunc, uint32 inFlags )
			: Func( inFunc )
		{
			Flags = inFlags;
		}

		void Execute( Renderer& inRenderer )
		{
			if( Func )
			{
				Func( inRenderer );
			}
		}
	};

	struct RenderFenceState
	{
		std::mutex m;
		std::condition_variable cv;
		uint32 count;

		RenderFenceState()
			: count( 0 )
		{}
	};

	struct RenderFenceCommand : public RenderCommandBase
	{

	private:

		std::shared_ptr< RenderFenceState > state;

	public:

		RenderFenceCommand() = delete;
		RenderFenceCommand( std::shared_ptr< RenderFenceState >& inState )
			: state( inState )
		{
			// Incremement state
			if( state )
			{
				{
					std::unique_lock< std::mutex > lock( state->m );
					state->count++;
				}

				state->cv.notify_all();
			}
		}

		void Execute( Renderer& inRenderer )
		{
			if( state )
			{
				{
					std::unique_lock< std::mutex > lock( state->m );
					
					if( state->count <= 1 ) state->count = 0;
					else state->count--;
				}

				state->cv.notify_all();
			}
		}

	};

	struct RenderFenceWatcher
	{
		
	private:

		std::shared_ptr< RenderFenceState > state;

	public:

		RenderFenceWatcher()
			: state( std::make_shared< RenderFenceState >() )
		{}

		RenderFenceWatcher( const RenderFenceWatcher& other )
			: state( other.state )
		{}

		RenderFenceWatcher( RenderFenceWatcher&& other )
			: state( std::move( other.state ) )
		{}

		uint32 GetCount()
		{
			if( state )
			{
				std::unique_lock< std::mutex > lock( state->m );
				return state->count;
			}

			return 0;
		}

		bool WaitForCount( uint32 targetCount, ComparisonType comp = ComparisonType::EQUAL )
		{
			if( state )
			{
				std::unique_lock< std::mutex > lock( state->m );
				state->cv.wait( lock, [ targetCount, comp, this ] { return !state || EvaluateComparison( state->count, targetCount, comp ); } );

				return true;
			}

			return false;
		}

		template< typename _Rep, typename _Per >
		bool WaitForCount( uint32 targetCount, std::chrono::duration< _Rep, _Per > timeout, ComparisonType comp = ComparisonType::EQUAL )
		{
			if( state )
			{
				std::unique_lock< std::mutex > lock( state->m );
				return state->cv.wait_for( lock, timeout, [ targetCount, comp, this ] { return !state || EvaluateComparison( state->count, targetCount, comp ); } );
			}

			return false;
		}

		bool WaitForUpdate()
		{
			if( state )
			{
				std::unique_lock< std::mutex > lock( state->m );
				auto original_count = state->count;
				state->cv.wait( lock, [ original_count, this ] { return !state || state->count != original_count; } );

				return true;
			}

			return false;
		}

		std::unique_ptr< RenderFenceCommand > CreateCommand()
		{
			if( state )
			{
				return std::make_unique< RenderFenceCommand >( state );
			}
			else
			{
				return nullptr;
			}
		}
	};

	struct ScreenResolution
	{
		uint32 Width;
		uint32 Height;
		bool FullScreen;
	};

	class Renderer
	{

	protected:

		virtual bool Init()			= 0;
		virtual void Frame()		= 0;
		virtual void Shutdown()		= 0;

		void Initialize();
		void Shutdown();
		void Tick();

		void UpdateProxyScene();

	public:

		Renderer();
		virtual ~Renderer();

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

		// These functions are used to register a primitive with the renderer.. should be called from a RenderCommand
		// Before calling.. call Engine_Init.. we will call Render_Init
		bool AddPrimitive( std::shared_ptr< ProxyPrimitive > inPrimitive );
		bool AddLight( std::shared_ptr< ProxyLight > inLight );
		bool AddCamera( std::shared_ptr< ProxyCamera > inCamera );

		bool RemovePrimitive( std::weak_ptr< ProxyPrimitive > inPrimitive, std::function< void() > inCallback = nullptr );
		bool RemoveLight( std::weak_ptr< ProxyLight > inLight, std::function< void() > inCallback = nullptr );
		bool RemoveCamera( std::weak_ptr< ProxyCamera > inCamera, std::function< void() > inCallback = nullptr );

		/*
			Command System
		*/
	private:

		ConcurrentQueue< std::unique_ptr< RenderCommandBase > > m_CommandQueue;


	public:

		void PushCommand( std::unique_ptr< RenderCommandBase >&& inCommand );

		friend class TRenderThread;

	};


	class TRenderThread
	{

	private:

		static std::shared_ptr< Thread > m_Thread;

	public:

		static void Init();
		static void Shutdown();
		static void Tick();

		static bool Start();
		static bool Stop();
		static bool IsRunning();

	};

}