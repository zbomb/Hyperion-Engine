/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DX12/DX12Renderer.cpp
	© 2021, Zachary Berry
==================================================================================================*/

/*
*	Header Includes
*/
#include "Hyperion/Renderer/DX12/DX12Renderer.h"
#include "Hyperion/Renderer/DX12/DX12Core.h"



namespace Hyperion
{

	DX12Renderer::DX12Renderer()
	{
		m_Core = std::make_shared< DX12Core >();
	}


	DX12Renderer::~DX12Renderer()
	{
		Shutdown();

		m_Core.reset();
	}


	bool DX12Renderer::Initialize( void* inWindow, const ScreenResolution& inResolution, bool bVsync )
	{
		HYPERION_VERIFY( inWindow != nullptr, "[DX12] Window pointer was null" );
		HYPERION_VERIFY( m_Core != nullptr, "[DX12] API core was null" );

		// Cast to window handle...
		HWND handle = static_cast< HWND >( inWindow );
		if( !m_Core->Initialize( handle, inResolution ) )
		{
			return false;
		}

		if( !Renderer::Initialize( inWindow, inResolution, bVsync ) )
		{
			return false;
		}

		m_Resolution.store( inResolution );
		m_bVSync.store( bVsync );

		return true;
	}


	void DX12Renderer::Shutdown()
	{
		HYPERION_VERIFY( m_Core != nullptr, "[DX12] API core was null" );
		m_Core->Shutdown();

		Renderer::Shutdown();
	}


	void DX12Renderer::Frame()
	{
		HYPERION_VERIFY( m_Core != nullptr, "[DX12] Device is null" );

		auto renderTarget = m_Core->Get
	}


	bool DX12Renderer::SetResolution( const ScreenResolution& inResolution )
	{
		// First, we need to check if the selected resolution is even valid
		if( !m_Core->IsResolutionValid( inResolution ) )
		{
			return false;
		}

		if( IsRenderThread() )
		{
			if( m_Core->UpdateResolution( inResolution ) )
			{
				m_Resolution.store( inResolution );
				OnResolutionChanged();
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			AddImmediateCommand( std::make_unique< RenderCommand >(
				[this, inResolution] ( Renderer& r )
				{
					if( m_Core->UpdateResolution( inResolution ) )
					{
						OnResolutionChanged();
					}
				} )
			);

			m_Resolution.store( inResolution );
			return true;
		}
	}


	bool DX12Renderer::SetVSync( bool bVSync )
	{
		if( IsRenderThread() )
		{
			if( m_Core->UpdateVSync( bVSync ) )
			{
				m_bVSync.store( bVSync );
				OnVSyncChanged();
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			AddImmediateCommand( std::make_unique< RenderCommand >(
				[this, bVSync] ( Renderer& r )
				{
					if( m_Core->UpdateVSync( bVSync ) )
					{
						OnVSyncChanged();
					}
				} )
			);

			m_bVSync.store( bVSync );
			return true;
		}

		return true;
	}


	void DX12Renderer::OnResolutionChanged()
	{
		auto res = m_Resolution.load();

		Console::WriteLine( "[DX12] Resolution updated to ", res.Width, "x", res.Height );
	}


	void DX12Renderer::OnVSyncChanged()
	{
		auto b = m_bVSync.load();

		Console::WriteLine( "[DX12] VSync ", b ? "Enabled" : "Disabled" );
	}

}