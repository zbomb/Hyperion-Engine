/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11Texture.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RTexture.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	UINT TranslateHyperionBindFlags( uint32 inFlags );
	uint32 TranslateDXBindFlags( UINT inDXFlags );
	DXGI_FORMAT TextureFormatToDXGIFormat( TextureFormat In );
	TextureFormat DXGIFormatToTextureFormat( DXGI_FORMAT In );


	class DirectX11Texture1D : public RTexture1D
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11Texture1D > m_Texture;
		Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > m_ResourceView;

		DirectX11Texture1D()
			: m_Texture( nullptr ), m_ResourceView( nullptr )
		{}

		DirectX11Texture1D( Microsoft::WRL::ComPtr< ID3D11Texture1D > inRaw, const Microsoft::WRL::ComPtr< ID3D11ShaderResourceView >& inView = nullptr )
			: m_Texture( inRaw ), m_ResourceView( inView )
		{}

	public:

		~DirectX11Texture1D()
		{
			Shutdown();
		}

		void Shutdown() override
		{
			m_ResourceView.Reset();
			m_Texture.Reset();
		}

		bool IsValid() const override
		{
			if( !m_Texture ) { return false; }
			if( !m_ResourceView )
			{
				// Check if this is a shader binded texture
				return !HYPERION_HAS_FLAG( GetBindTargets(), RENDERER_TEXTURE_BIND_FLAG_SHADER );
			}

			return true;
		}

		ID3D11Texture1D** GetAddress()
		{
			return m_Texture.GetAddressOf();
		}

		ID3D11ShaderResourceView** GetViewAddress()
		{
			return m_ResourceView.GetAddressOf();
		}

		ID3D11Texture1D* Get()
		{
			return m_Texture.Get();
		}

		ID3D11ShaderResourceView* GetView()
		{
			return m_ResourceView.Get();
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

		uint32 GetBindTargets() const final
		{
			if( !m_Texture ) { return RENDERER_TEXTURE_BIND_FLAG_NONE; }

			D3D11_TEXTURE1D_DESC desc;
			m_Texture->GetDesc( &desc );

			return TranslateDXBindFlags( desc.BindFlags );
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
			DirectX11Texture1D* casted = dynamic_cast< DirectX11Texture1D* >( &Other );
			HYPERION_VERIFY( casted != nullptr, "Attempt to swap textures from different apis?" );

			auto tmp = m_Texture;
			auto tmp_view = m_ResourceView;
			m_Texture = casted->m_Texture;
			m_ResourceView = casted->m_ResourceView;
			casted->m_Texture = tmp;
			casted->m_ResourceView = tmp_view;
		}

		friend class DirectX11Graphics;
	};


	class DirectX11Texture2D : public RTexture2D
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11Texture2D > m_Texture;
		Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > m_ResourceView;

		DirectX11Texture2D()
			: m_Texture( nullptr ), m_ResourceView( nullptr )
		{}

		DirectX11Texture2D( const Microsoft::WRL::ComPtr< ID3D11Texture2D >& inRawTexture, const Microsoft::WRL::ComPtr< ID3D11ShaderResourceView >& inView = nullptr )
			: m_Texture( inRawTexture ), m_ResourceView( inView )
		{}

	public:

		~DirectX11Texture2D()
		{
			Shutdown();
		}

		void Shutdown() override
		{
			m_ResourceView.Reset();
			m_Texture.Reset();
		}

		bool IsValid() const override
		{
			if( !m_Texture ) { return false; }
			if( !m_ResourceView )
			{
				// Check if this is a shader binded texture
				return !HYPERION_HAS_FLAG( GetBindTargets(), RENDERER_TEXTURE_BIND_FLAG_SHADER );
			}

			return true;
		}

		ID3D11Texture2D** GetAddress()
		{
			return m_Texture.GetAddressOf();
		}

		ID3D11ShaderResourceView** GetViewAddress()
		{
			return m_ResourceView.GetAddressOf();
		}

		ID3D11Texture2D* Get()
		{
			return m_Texture.Get();
		}

		ID3D11ShaderResourceView* GetView()
		{
			return m_ResourceView.Get();
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

		uint32 GetBindTargets() const final
		{
			if( !m_Texture ) { return RENDERER_TEXTURE_BIND_FLAG_NONE; }

			D3D11_TEXTURE2D_DESC desc;
			m_Texture->GetDesc( &desc );

			return TranslateDXBindFlags( desc.BindFlags );
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
			auto casted = dynamic_cast<DirectX11Texture2D*>( &Other );
			HYPERION_VERIFY( casted != nullptr, "Attempt to swap textures from different apis!" );

			auto tmp = m_Texture;
			m_Texture = casted->m_Texture;
			casted->m_Texture = tmp;

			auto res_tmp = m_ResourceView;
			m_ResourceView = casted->m_ResourceView;
			casted->m_ResourceView = res_tmp;
		}

		friend class DirectX11Graphics;
	};


	class DirectX11Texture3D : public RTexture3D
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11Texture3D > m_Texture;
		Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > m_ResourceView;


		DirectX11Texture3D()
			: m_Texture( nullptr ), m_ResourceView( nullptr )
		{}

		DirectX11Texture3D( const Microsoft::WRL::ComPtr< ID3D11Texture3D >& inRaw, const Microsoft::WRL::ComPtr< ID3D11ShaderResourceView >& inView = nullptr )
			: m_Texture( inRaw ), m_ResourceView( inView )
		{}

	public:

		~DirectX11Texture3D()
		{
			Shutdown();
		}

		void Shutdown() override
		{
			m_ResourceView.Reset();
			m_Texture.Reset();
		}

		bool IsValid() const override
		{
			if( !m_Texture ) { return false; }
			if( !m_ResourceView )
			{
				// Check if this is a shader binded texture
				return !HYPERION_HAS_FLAG( GetBindTargets(), RENDERER_TEXTURE_BIND_FLAG_SHADER );
			}

			return true;
		}

		ID3D11Texture3D** GetAddress()
		{
			return m_Texture.GetAddressOf();
		}

		ID3D11ShaderResourceView** GetViewAddress()
		{
			return m_ResourceView.GetAddressOf();
		}

		ID3D11Texture3D* Get()
		{
			return m_Texture.Get();
		}

		ID3D11ShaderResourceView* GetView()
		{
			return m_ResourceView.Get();
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

		uint32 GetBindTargets() const final
		{
			if( !m_Texture ) { return RENDERER_TEXTURE_BIND_FLAG_NONE; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return TranslateDXBindFlags( desc.BindFlags );
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
			auto casted = dynamic_cast<DirectX11Texture3D*>( &Other );
			HYPERION_VERIFY( casted != nullptr, "Attempt to swap textures from different apis!" );

			auto tmp = m_Texture;
			m_Texture = casted->m_Texture;
			casted->m_Texture = tmp;

			auto res_tmp = m_ResourceView;
			m_ResourceView = casted->m_ResourceView;
			casted->m_ResourceView = res_tmp;
		}

		friend class DirectX11Graphics;
	};
}