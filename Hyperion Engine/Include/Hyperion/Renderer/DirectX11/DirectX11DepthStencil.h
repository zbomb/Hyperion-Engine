/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11DepthStencil.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resource/DepthStencil.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"
#include "Hyperion/Renderer/DirectX11/DirectX11Texture.h"


namespace Hyperion
{

	class DirectX11DepthStencil : public RDepthStencil
	{

	protected:

		Microsoft::WRL::ComPtr< ID3D11DepthStencilView > m_View;
		Microsoft::WRL::ComPtr< ID3D11Texture2D > m_Texture;

		uint32 m_Width, m_Height;

	public:


		DirectX11DepthStencil( ID3D11Device* inDevice, uint32 inWidth, uint32 inHeight )
		{
			Resize( inDevice, inWidth, inHeight );
		}


		~DirectX11DepthStencil()
		{
			Shutdown();
		}


		void Shutdown() final
		{
			m_View.Reset();
			m_Texture.Reset();

			m_Width		= 0;
			m_Height	= 0;
		}


		bool IsValid() const final
		{
			return m_View && m_Texture;
		}


		inline ID3D11Texture2D* GetTexture() const { return m_Texture.Get(); }
		inline ID3D11DepthStencilView* GetStencilView() const { return m_View.Get(); }

		inline uint32 GetWidth() const { return m_Width; }
		inline uint32 GetHeight() const { return m_Height; }


		bool Resize( ID3D11Device* inDevice, uint32 inWidth, uint32 inHeight )
		{
			HYPERION_VERIFY( inDevice != nullptr, "[DX11] Device is null" );
			HYPERION_VERIFY( inWidth > 0 && inHeight > 0, "[DX11] Depth stencil dimensions invalid" );

			Shutdown();
			m_Width = inWidth;
			m_Height = inHeight;

			// Create the texture
			D3D11_TEXTURE2D_DESC texDesc;
			ZeroMemory( &texDesc, sizeof( texDesc ) );

			texDesc.Width				= inWidth;
			texDesc.Height				= inHeight;
			texDesc.MipLevels			= 1;
			texDesc.ArraySize			= 1;
			texDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
			texDesc.SampleDesc.Count	= 1;
			texDesc.SampleDesc.Quality	= 0;
			texDesc.Usage				= D3D11_USAGE_DEFAULT;
			texDesc.BindFlags			= D3D11_BIND_DEPTH_STENCIL;
			texDesc.CPUAccessFlags		= 0;
			texDesc.MiscFlags			= 0;

			if( FAILED( inDevice->CreateTexture2D( &texDesc, NULL, m_Texture.GetAddressOf() ) ) || !m_Texture )
			{
				Console::WriteLine( "[Warning] DX11: Failed to create depth stencil texture!" );
				Shutdown();

				return false;
			}
			else
			{
				// Create the stencil view
				D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
				ZeroMemory( &viewDesc, sizeof( viewDesc ) );

				viewDesc.Format					= DXGI_FORMAT_D24_UNORM_S8_UINT;
				viewDesc.ViewDimension			= D3D11_DSV_DIMENSION_TEXTURE2D;
				viewDesc.Texture2D.MipSlice		= 0;

				if( FAILED( inDevice->CreateDepthStencilView( m_Texture.Get(), &viewDesc, m_View.GetAddressOf() ) ) || !m_View )
				{
					Console::WriteLine( "[Warning] DX11: Failed to create depth stencil view!" );
					Shutdown();

					return false;
				}

				return true;
			}
		}
	};

}