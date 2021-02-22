/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11Texture.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RTexture.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"
#include "Hyperion/Renderer/DirectX11/RDX11Resource.h"


namespace Hyperion
{

	UINT TranslateHyperionBindFlags( uint32 inFlags );
	uint32 TranslateDXBindFlags( UINT inDXFlags );
	DXGI_FORMAT TextureFormatToDXGIFormat( TextureFormat In );
	TextureFormat DXGIFormatToTextureFormat( DXGI_FORMAT In );


	class DX11Texture1D : public RTexture1D, public RDX11Resource
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11Texture1D > m_Texture;
		Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > m_SRV;
		Microsoft::WRL::ComPtr< ID3D11UnorderedAccessView > m_UAV;
		Microsoft::WRL::ComPtr< ID3D11RenderTargetView > m_RTV;

		uint32 m_Asset;

		DX11Texture1D()
			: m_Texture( nullptr ), m_SRV( nullptr ), m_UAV( nullptr ), m_RTV( nullptr ), m_Asset( ASSET_INVALID )
		{}

	public:

		~DX11Texture1D()
		{
			Shutdown();
		}

		void Shutdown() override
		{
			m_RTV.Reset();
			m_UAV.Reset();
			m_SRV.Reset();
			m_Texture.Reset();
		}

		bool IsValid() const override
		{
			return m_Texture ? true : false;
		}


		ID3D11Texture1D** GetAddress()
		{
			return m_Texture.GetAddressOf();
		}

		ID3D11Texture1D* Get()
		{
			return m_Texture.Get();
		}

		uint32 GetWidth() const final
		{
			if( !m_Texture ) { return 0; }

			D3D11_TEXTURE1D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Width;
		}

		uint8 GetMipCount() const final
		{
			if( !m_Texture ) { return 0; }

			D3D11_TEXTURE1D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.MipLevels;
		}

		bool IsDynamic() const final
		{
			if( !m_Texture ) { return 0; }

			D3D11_TEXTURE1D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Usage == D3D11_USAGE_DYNAMIC;
		}

		bool CanCPURead() const final
		{
			if( !m_Texture ) { return false; }

			D3D11_TEXTURE1D_DESC desc;
			m_Texture->GetDesc( &desc );

			return( ( desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ ) != 0 );
		}

		TextureFormat GetFormat() const final
		{
			if( !m_Texture ) { return TextureFormat::NONE; }

			D3D11_TEXTURE1D_DESC desc;
			m_Texture->GetDesc( &desc );

			return DXGIFormatToTextureFormat( desc.Format );
		}

		void Swap( RTexture1D& Other ) final
		{
			DX11Texture1D* casted = dynamic_cast< DX11Texture1D* >( &Other );
			HYPERION_VERIFY( casted != nullptr, "Attempt to swap textures from different apis?" );

			auto tmp = m_Texture;
			auto tmp_srv = m_SRV;
			auto tmp_uav = m_UAV;
			auto tmp_rtv = m_RTV;
			m_Texture = casted->m_Texture;
			m_SRV = casted->m_SRV;
			m_UAV = casted->m_UAV;
			m_RTV = casted->m_UAV;
			casted->m_Texture = tmp;
			casted->m_SRV = tmp_srv;
			casted->m_UAV = tmp_uav;
			casted->m_RTV = tmp_rtv;
			auto id_temp = m_Asset;
			m_Asset = casted->m_Asset;
			casted->m_Asset = id_temp;
		}

		uint32 GetAssetIdentifier() const final { return m_Asset;; }

		/*
		*	RDX11Resource Implementation
		*/
		inline ID3D11ShaderResourceView* GetSRV() final { return m_SRV.Get(); }
		inline ID3D11UnorderedAccessView* GetUAV()	final { return m_UAV.Get(); }
		inline ID3D11RenderTargetView* GetRTV() final { return m_RTV.Get(); }
		inline ID3D11ShaderResourceView** GetSRVAddress() final { return m_SRV.GetAddressOf(); }
		inline ID3D11UnorderedAccessView** GetUAVAddress()	final { return m_UAV.GetAddressOf(); }
		inline ID3D11RenderTargetView** GetRTVAddress() final { return m_RTV.GetAddressOf(); }
		inline ID3D11Resource* GetResource() final { return m_Texture.Get(); }

		/*
		*	RGraphicsResource Implementation
		*/
		inline bool IsComputeTarget() const final { return m_UAV ? true : false; }
		inline bool IsRenderTarget() const final { return m_RTV ? true : false; }
		inline bool IsShaderResource() const final { return m_SRV ? true : false; }

		friend class DX11Graphics;
	};


	class DX11Texture2D : public RTexture2D, public RDX11Resource
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11Texture2D > m_Texture;
		Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > m_SRV;
		Microsoft::WRL::ComPtr< ID3D11UnorderedAccessView > m_UAV;
		Microsoft::WRL::ComPtr< ID3D11RenderTargetView > m_RTV;

		uint32 m_Asset;

		DX11Texture2D()
			: m_Texture( nullptr ), m_SRV( nullptr ), m_UAV( nullptr ), m_RTV( nullptr ), m_Asset( ASSET_INVALID )
		{}

	public:

		~DX11Texture2D()
		{
			Shutdown();
		}

		void Shutdown() override
		{
			m_RTV.Reset();
			m_UAV.Reset();
			m_SRV.Reset();
			m_Texture.Reset();
		}

		bool IsValid() const override
		{
			return m_Texture ? true : false;
		}

		ID3D11Texture2D** GetAddress()
		{
			return m_Texture.GetAddressOf();
		}

		ID3D11Texture2D* Get()
		{
			return m_Texture.Get();
		}

		uint32 GetWidth() const final
		{
			if( !m_Texture ) { return 0; }

			D3D11_TEXTURE2D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Width;
		}

		uint32 GetHeight() const final
		{
			if( !m_Texture ) { return 0; }

			D3D11_TEXTURE2D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Height;
		}

		uint8 GetMipCount() const final
		{
			if( !m_Texture ) { return 0; }

			D3D11_TEXTURE2D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.MipLevels;
		}

		bool IsDynamic() const final
		{
			if( !m_Texture ) { return false; }

			D3D11_TEXTURE2D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Usage == D3D11_USAGE_DYNAMIC;
		}

		bool CanCPURead() const final
		{
			if( !m_Texture ) { return false; }

			D3D11_TEXTURE2D_DESC desc;
			m_Texture->GetDesc( &desc );

			return( ( desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ ) != 0 );
		}

		TextureFormat GetFormat() const final
		{
			if( !m_Texture ) { return TextureFormat::NONE; }

			D3D11_TEXTURE2D_DESC desc;
			m_Texture->GetDesc( &desc );

			return DXGIFormatToTextureFormat( desc.Format );
		}

		void Swap( RTexture2D& Other )
		{
			auto casted = dynamic_cast<DX11Texture2D*>( &Other );
			HYPERION_VERIFY( casted != nullptr, "Attempt to swap textures from different apis!" );

			auto tmp = m_Texture;
			auto tmp_srv = m_SRV;
			auto tmp_uav = m_UAV;
			auto tmp_rtv = m_RTV;
			m_Texture = casted->m_Texture;
			m_SRV = casted->m_SRV;
			m_UAV = casted->m_UAV;
			m_RTV = casted->m_UAV;
			casted->m_Texture = tmp;
			casted->m_SRV = tmp_srv;
			casted->m_UAV = tmp_uav;
			casted->m_RTV = tmp_rtv;
			auto id_temp = m_Asset;
			m_Asset = casted->m_Asset;
			casted->m_Asset = id_temp;
		}

		uint32 GetAssetIdentifier() const final { return m_Asset; }

		/*
		*	RDX11Resource Implementation
		*/
		inline ID3D11ShaderResourceView* GetSRV() final { return m_SRV.Get(); }
		inline ID3D11UnorderedAccessView* GetUAV()	final { return m_UAV.Get(); }
		inline ID3D11RenderTargetView* GetRTV() final { return m_RTV.Get(); }
		inline ID3D11ShaderResourceView** GetSRVAddress() final { return m_SRV.GetAddressOf(); }
		inline ID3D11UnorderedAccessView** GetUAVAddress()	final { return m_UAV.GetAddressOf(); }
		inline ID3D11RenderTargetView** GetRTVAddress() final { return m_RTV.GetAddressOf(); }
		inline ID3D11Resource* GetResource() final { return m_Texture.Get(); }

		/*
		*	RGraphicsResource Implementation
		*/
		inline bool IsComputeTarget() const final { return m_UAV ? true : false; }
		inline bool IsRenderTarget() const final { return m_RTV ? true : false; }
		inline bool IsShaderResource() const final { return m_SRV ? true : false; }

		friend class DX11Graphics;
	};


	class DX11Texture3D : public RTexture3D, public RDX11Resource
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11Texture3D > m_Texture;
		Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > m_SRV;
		Microsoft::WRL::ComPtr< ID3D11UnorderedAccessView > m_UAV;
		Microsoft::WRL::ComPtr< ID3D11RenderTargetView > m_RTV;
		uint32 m_Asset;


		DX11Texture3D()
			: m_Texture( nullptr ), m_SRV( nullptr ), m_UAV( nullptr ), m_RTV( nullptr ), m_Asset( ASSET_INVALID )
		{}

	public:

		~DX11Texture3D()
		{
			Shutdown();
		}

		void Shutdown() override
		{
			m_RTV.Reset();
			m_UAV.Reset();
			m_SRV.Reset();
			m_Texture.Reset();
		}

		bool IsValid() const override
		{
			return m_Texture ? true : false;
		}

		ID3D11Texture3D** GetAddress()
		{
			return m_Texture.GetAddressOf();
		}

		ID3D11Texture3D* Get()
		{
			return m_Texture.Get();
		}

		uint32 GetWidth() const final
		{
			if( !m_Texture ) { return 0; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Width;
		}

		uint32 GetHeight() const final
		{
			if( !m_Texture ) { return 0; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Height;
		}

		uint32 GetDepth() const final
		{
			if( !m_Texture ) { return 0; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Depth;
		}

		uint8 GetMipCount() const final
		{
			if( !m_Texture ) { return 0; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.MipLevels;
		}

		bool IsDynamic() const final
		{
			if( !m_Texture ) { return false; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Usage == D3D11_USAGE_DYNAMIC;
		}

		bool CanCPURead() const final
		{
			if( !m_Texture ) { return false; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return( ( desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ ) != 0 );
		}

		TextureFormat GetFormat() const final
		{
			if( !m_Texture ) { return TextureFormat::NONE; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return DXGIFormatToTextureFormat( desc.Format );
		}

		void Swap( RTexture3D& Other )
		{
			auto casted = dynamic_cast<DX11Texture3D*>( &Other );
			HYPERION_VERIFY( casted != nullptr, "Attempt to swap textures from different apis!" );

			auto tmp = m_Texture;
			auto tmp_srv = m_SRV;
			auto tmp_uav = m_UAV;
			auto tmp_rtv = m_RTV;
			m_Texture = casted->m_Texture;
			m_SRV = casted->m_SRV;
			m_UAV = casted->m_UAV;
			m_RTV = casted->m_UAV;
			casted->m_Texture = tmp;
			casted->m_SRV = tmp_srv;
			casted->m_UAV = tmp_uav;
			casted->m_RTV = tmp_rtv;
			auto id_temp = m_Asset;
			m_Asset = casted->m_Asset;
			casted->m_Asset = id_temp;
		}

		uint32 GetAssetIdentifier() const final { return m_Asset; }

		/*
		*	RDX11Resource Implementation
		*/
		inline ID3D11ShaderResourceView* GetSRV() final { return m_SRV.Get(); }
		inline ID3D11UnorderedAccessView* GetUAV()	final { return m_UAV.Get(); }
		inline ID3D11RenderTargetView* GetRTV() final { return m_RTV.Get(); }
		inline ID3D11ShaderResourceView** GetSRVAddress() final { return m_SRV.GetAddressOf(); }
		inline ID3D11UnorderedAccessView** GetUAVAddress()	final { return m_UAV.GetAddressOf(); }
		inline ID3D11RenderTargetView** GetRTVAddress() final { return m_RTV.GetAddressOf(); }
		inline ID3D11Resource* GetResource() final { return m_Texture.Get(); }

		/*
		*	RGraphicsResource Implementation
		*/
		inline bool IsComputeTarget() const final { return m_UAV ? true : false; }
		inline bool IsRenderTarget() const final { return m_RTV ? true : false; }
		inline bool IsShaderResource() const final { return m_SRV ? true : false; }

		friend class DX11Graphics;
	};
}