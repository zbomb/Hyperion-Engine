/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11RenderTarget.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"
#include "Hyperion/Renderer/Resources/RRenderTarget.h"


namespace Hyperion
{

	class DirectX11RenderTarget : public RRenderTarget
	{

	protected:

		Microsoft::WRL::ComPtr< ID3D11RenderTargetView > m_RenderTarget;


		DirectX11RenderTarget( const std::shared_ptr< RTexture2D >& inTexture, const Microsoft::WRL::ComPtr< ID3D11Device >& inDevice )
			: RRenderTarget( inTexture ), m_RenderTarget( nullptr )
		{
			HYPERION_VERIFY( inDevice, "[DX11] Device was null!" );

			if( m_TargetTexture && m_TargetTexture->IsValid() )
			{
				auto* texPtr = dynamic_cast< DirectX11Texture2D* >( inTexture.get() );
				HYPERION_VERIFY( texPtr != nullptr || texPtr->Get() == nullptr, "[DX11] Failed to cast texture2d to api type" );

				// Create the D3D11 render target
				D3D11_RENDER_TARGET_VIEW_DESC targetDesc;
				ZeroMemory( &targetDesc, sizeof( targetDesc ) );

				D3D11_TEXTURE2D_DESC texDesc;
				texPtr->Get()->GetDesc( &texDesc );

				targetDesc.Format				= texDesc.Format;
				targetDesc.ViewDimension		= D3D11_RTV_DIMENSION_TEXTURE2D;
				targetDesc.Texture2D.MipSlice	= 0;

				if( FAILED( inDevice->CreateRenderTargetView( texPtr->Get(), &targetDesc, m_RenderTarget.GetAddressOf() ) ) || m_RenderTarget.Get() == NULL )
				{
					Console::WriteLine( "[Warning] DX11: Failed to create texture render target!" );
				}
			}
			else
			{
				Console::WriteLine( "[Warning] DX11: Creating null render target!" );
			}
		}

	public:

		~DirectX11RenderTarget()
		{
			Shutdown();
		}

		bool IsValid() const final
		{
			return m_TargetTexture && m_TargetTexture->IsValid() && m_RenderTarget;
		}

		void Shutdown() final
		{
			m_RenderTarget.Reset();
			RRenderTarget::Shutdown();
		}

		ID3D11RenderTargetView* Get()
		{
			return m_RenderTarget ? m_RenderTarget.Get() : nullptr;
		}

		ID3D11RenderTargetView** GetAddress()
		{
			return m_RenderTarget ? m_RenderTarget.GetAddressOf() : nullptr;
		}

		friend class DirectX11Graphics;
	};

}