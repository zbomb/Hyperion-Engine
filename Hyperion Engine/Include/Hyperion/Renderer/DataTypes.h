/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DataTypes.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"

// For HWND
#if HYPERION_OS_WIN32
#include <Windows.h>
#endif

#include <functional>
#include <mutex>


namespace Hyperion
{

	/*
		Forward Declarations
	*/
	class Renderer;

	constexpr float SCREEN_NEAR		= 0.1f;
	constexpr float SCREEN_FAR		= 1000.f;

	struct ScreenResolution
	{
		uint32 Width;
		uint32 Height;
		bool FullScreen;
	};


	struct RenderSettings
	{
		ScreenResolution resolution;
		bool bVSync;
	};


	class IRenderOutput
	{

	public:

#if HYPERION_OS_WIN32

		IRenderOutput()
			: ptr( nullptr )
		{}

		~IRenderOutput()
		{
			ptr = nullptr;
		}

		HWND ptr;

		IRenderOutput( const IRenderOutput& inOther )
			: ptr( inOther.ptr )
		{}

		operator bool() const
		{
			return ptr != nullptr;
		}
#endif

	};


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

		RenderFenceWatcher( RenderFenceWatcher&& other ) noexcept
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











}