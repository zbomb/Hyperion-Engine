/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11RenderTarget.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"
#include "Hyperion/Renderer/Types/IRenderTarget.h"


namespace Hyperion
{

	class DirectX11RenderTarget : public IRenderTarget
	{

	protected:

		ID3D11RenderTargetView* m_RenderTarget;

		DirectX11RenderTarget( std::shared_ptr< ITexture2D > inTexture )
			: IRenderTarget( inTexture ), m_RenderTarget( nullptr )
		{
		}


	public:

		~DirectX11RenderTarget()
		{
			Shutdown();
		}

		bool IsValid() const final
		{
			return m_TargetTexture != nullptr && m_RenderTarget != nullptr;
		}

		void Shutdown() final
		{
			m_TargetTexture.reset();
			if( m_RenderTarget )
			{
				m_RenderTarget->Release();
				m_RenderTarget = nullptr;
			}
		}

		ID3D11RenderTargetView* Get()
		{
			return IsValid() ? m_RenderTarget : nullptr;
		}

		ID3D11RenderTargetView** GetAddress()
		{
			return &m_RenderTarget;
		}

		friend class DirectX11Graphics;
	};

}