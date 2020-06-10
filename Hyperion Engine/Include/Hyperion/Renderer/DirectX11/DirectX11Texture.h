/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11Texture.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once


#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Types/ITexture.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	TextureFormat DXGIFormatToTextureFormat( DXGI_FORMAT In )
	{
		switch( In )
		{
			/*
			 * 8-bit Types
			*/
			// Unsigned Normals (converted to floats in shaders) [0,1]
		case DXGI_FORMAT_R8_UNORM:
			return TextureFormat::R_8BIT_UNORM;
		case DXGI_FORMAT_R8G8_UNORM:
			return TextureFormat::RG_8BIT_UNORM;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			return TextureFormat::RGBA_8BIT_UNORM;
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			return TextureFormat::RGBA_8BIT_UNORM_SRGB;

				// Signed Normals (converted to floats in shaders) [-1,1]
		case DXGI_FORMAT_R8_SNORM:
			return TextureFormat::R_8BIT_SNORM;
		case DXGI_FORMAT_R8G8_SNORM:
			return TextureFormat::RG_8BIT_SNORM;
		case DXGI_FORMAT_R8G8B8A8_SNORM:
			return TextureFormat::RGBA_8BIT_SNORM;

				// Unsigned Integers (not converted to floats)
		case DXGI_FORMAT_R8_UINT:
			return TextureFormat::R_8BIT_UINT;
		case DXGI_FORMAT_R8G8_UINT:
			return TextureFormat::RG_8BIT_UINT;
		case DXGI_FORMAT_R8G8B8A8_UINT:
			return TextureFormat::RGBA_8BIT_UINT;

				// Signed Integers (not converted to floats)
		case DXGI_FORMAT_R8_SINT:
			return TextureFormat::R_8BIT_SINT;
		case DXGI_FORMAT_R8G8_SINT:
			return TextureFormat::RG_8BIT_SINT;
		case DXGI_FORMAT_R8G8B8A8_SINT:
			return TextureFormat::RGBA_8BIT_SINT;

				/*
				 * 16-bit Types
				*/
				// Unsigned normals (converted to floats in shaders) [0,1]
		case DXGI_FORMAT_R16_UNORM:
			return TextureFormat::R_16BIT_UNORM;
		case DXGI_FORMAT_R16G16_UNORM:
			return TextureFormat::RG_16BIT_UNORM;
		case DXGI_FORMAT_R16G16B16A16_UNORM:
			return TextureFormat::RGBA_16BIT_UNORM;

				// Signed normals (converted to floats in shaders) [-1,1]
		case DXGI_FORMAT_R16_SNORM:
			return TextureFormat::R_16BIT_SNORM;
		case DXGI_FORMAT_R16G16_SNORM:
			return TextureFormat::RG_16BIT_SNORM;
		case DXGI_FORMAT_R16G16B16A16_SNORM:
			return TextureFormat::RGBA_16BIT_SNORM;

				// Unisnged Integers (not converted to floats)
		case DXGI_FORMAT_R16_UINT:
			return TextureFormat::R_16BIT_UINT;
		case DXGI_FORMAT_R16G16_UINT:
			return TextureFormat::RG_16BIT_UINT;
		case DXGI_FORMAT_R16G16B16A16_UINT:
			return TextureFormat::RGBA_16BIT_UINT;

				// Signed Integers (not converted to floats)
		case DXGI_FORMAT_R16_SINT:
			return TextureFormat::R_16BIT_SINT;
		case DXGI_FORMAT_R16G16_SINT:
			return TextureFormat::RG_16BIT_SINT;
		case DXGI_FORMAT_R16G16B16A16_SINT:
			return TextureFormat::RGBA_16BIT_SINT;

				// Floats 
		case DXGI_FORMAT_R16_FLOAT:
			return TextureFormat::R_16BIT_FLOAT;
		case DXGI_FORMAT_R16G16_FLOAT:
			return TextureFormat::RG_16BIT_FLOAT;
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
			return TextureFormat::RGBA_16BIT_FLOAT;

				/*
				 * 32-bit Types
				*/

				// Unsigned integers (not converted to floats)
		case DXGI_FORMAT_R32_UINT:
			return TextureFormat::R_32BIT_UINT;
		case DXGI_FORMAT_R32G32_UINT:
			return TextureFormat::RG_32BIT_UINT;
		case DXGI_FORMAT_R32G32B32_UINT:
			return TextureFormat::RGB_32BIT_UINT;
		case DXGI_FORMAT_R32G32B32A32_UINT:
			return TextureFormat::RGBA_32BIT_UINT;

				// Signed integers (not converted to floats)
		case DXGI_FORMAT_R32_SINT:
			return TextureFormat::R_32BIT_SINT;
		case DXGI_FORMAT_R32G32_SINT:
			return  TextureFormat::RG_32BIT_SINT;
		case DXGI_FORMAT_R32G32B32_SINT:
			return  TextureFormat::RGB_32BIT_SINT;
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return TextureFormat::RGBA_32BIT_SINT;

				// Floats
		case DXGI_FORMAT_R32_FLOAT:
			return TextureFormat::R_32BIT_FLOAT;
		case DXGI_FORMAT_R32G32_FLOAT:
			return TextureFormat::RG_32BIT_FLOAT;
		case DXGI_FORMAT_R32G32B32_FLOAT:
			return TextureFormat::RGB_32BIT_FLOAT;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return TextureFormat::RGBA_32BIT_FLOAT;

			// Compressed Types
		case DXGI_FORMAT_BC1_UNORM:
			return TextureFormat::RGB_DXT_1;
		case DXGI_FORMAT_BC3_UNORM:
			return TextureFormat::RGBA_DXT_5;
		case DXGI_FORMAT_BC7_UNORM:
			return TextureFormat::RGBA_BC_7; // ISSUE: We have two BC7 formats, one with RGB and one with RGBA


		default:
			return TextureFormat::NONE;
		}
	}

	// Lookup table for Hyperion::TextureFormat -> DXGI_FORMAT_*
	DXGI_FORMAT TextureFormatToDXGIFormat( TextureFormat In )
	{
		switch( In )
		{
			/*
			 * 8-bit Types
			*/
			// Unsigned Normals (converted to floats in shaders) [0,1]
		case TextureFormat::R_8BIT_UNORM:
			return DXGI_FORMAT_R8_UNORM;
		case TextureFormat::RG_8BIT_UNORM:
			return DXGI_FORMAT_R8G8_UNORM;
		case TextureFormat::RGBA_8BIT_UNORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::RGBA_8BIT_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

				// Signed Normals (converted to floats in shaders) [-1,1]
		case TextureFormat::R_8BIT_SNORM:
			return DXGI_FORMAT_R8_SNORM;
		case TextureFormat::RG_8BIT_SNORM:
			return DXGI_FORMAT_R8G8_SNORM;
		case TextureFormat::RGBA_8BIT_SNORM:
			return DXGI_FORMAT_R8G8B8A8_SNORM;

				// Unsigned Integers (not converted to floats)
		case TextureFormat::R_8BIT_UINT:
			return DXGI_FORMAT_R8_UINT;
		case TextureFormat::RG_8BIT_UINT:
			return DXGI_FORMAT_R8G8_UINT;
		case TextureFormat::RGBA_8BIT_UINT:
			return DXGI_FORMAT_R8G8B8A8_UINT;

				// Signed Integers (not converted to floats)
		case TextureFormat::R_8BIT_SINT:
			return DXGI_FORMAT_R8_SINT;
		case TextureFormat::RG_8BIT_SINT:
			return DXGI_FORMAT_R8G8_SINT;
		case TextureFormat::RGBA_8BIT_SINT:
			return DXGI_FORMAT_R8G8B8A8_SINT;

				/*
				 * 16-bit Types
				*/
				// Unsigned normals (converted to floats in shaders) [0,1]
		case TextureFormat::R_16BIT_UNORM:
			return DXGI_FORMAT_R16_UNORM;
		case TextureFormat::RG_16BIT_UNORM:
			return DXGI_FORMAT_R16G16_UNORM;
		case TextureFormat::RGBA_16BIT_UNORM:
			return DXGI_FORMAT_R16G16B16A16_UNORM;

				// Signed normals (converted to floats in shaders) [-1,1]
		case TextureFormat::R_16BIT_SNORM:
			return DXGI_FORMAT_R16_SNORM;
		case TextureFormat::RG_16BIT_SNORM:
			return DXGI_FORMAT_R16G16_SNORM;
		case TextureFormat::RGBA_16BIT_SNORM:
			return DXGI_FORMAT_R16G16B16A16_SNORM;

				// Unisnged Integers (not converted to floats)
		case TextureFormat::R_16BIT_UINT:
			return DXGI_FORMAT_R16_UINT;
		case TextureFormat::RG_16BIT_UINT:
			return DXGI_FORMAT_R16G16_UINT;
		case TextureFormat::RGBA_16BIT_UINT:
			return DXGI_FORMAT_R16G16B16A16_UINT;

				// Signed Integers (not converted to floats)
		case TextureFormat::R_16BIT_SINT:
			return DXGI_FORMAT_R16_SINT;
		case TextureFormat::RG_16BIT_SINT:
			return DXGI_FORMAT_R16G16_SINT;
		case TextureFormat::RGBA_16BIT_SINT:
			return DXGI_FORMAT_R16G16B16A16_SINT;

				// Floats 
		case TextureFormat::R_16BIT_FLOAT:
			return DXGI_FORMAT_R16_FLOAT;
		case TextureFormat::RG_16BIT_FLOAT:
			return DXGI_FORMAT_R16G16_FLOAT;
		case TextureFormat::RGBA_16BIT_FLOAT:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;

				/*
				 * 32-bit Types
				*/

				// Unsigned integers (not converted to floats)
		case TextureFormat::R_32BIT_UINT:
			return DXGI_FORMAT_R32_UINT;
		case TextureFormat::RG_32BIT_UINT:
			return DXGI_FORMAT_R32G32_UINT;
		case TextureFormat::RGB_32BIT_UINT:
			return DXGI_FORMAT_R32G32B32_UINT;
		case TextureFormat::RGBA_32BIT_UINT:
			return DXGI_FORMAT_R32G32B32A32_UINT;

				// Signed integers (not converted to floats)
		case TextureFormat::R_32BIT_SINT:
			return DXGI_FORMAT_R32_SINT;
		case TextureFormat::RG_32BIT_SINT:
			return DXGI_FORMAT_R32G32_SINT;
		case TextureFormat::RGB_32BIT_SINT:
			return DXGI_FORMAT_R32G32B32_SINT;
		case TextureFormat::RGBA_32BIT_SINT:
			return DXGI_FORMAT_R32G32B32A32_SINT;;

				// Floats
		case TextureFormat::R_32BIT_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;
		case TextureFormat::RG_32BIT_FLOAT:
			return DXGI_FORMAT_R32G32_FLOAT;
		case TextureFormat::RGB_32BIT_FLOAT:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case TextureFormat::RGBA_32BIT_FLOAT:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

			// Compressed Types
		case TextureFormat::RGB_DXT_1:
			return DXGI_FORMAT_BC1_UNORM;
		case TextureFormat::RGB_BC_7:
			return DXGI_FORMAT_BC7_UNORM;
		case TextureFormat::RGBA_DXT_5:
			return DXGI_FORMAT_BC3_UNORM;
		case TextureFormat::RGBA_BC_7:
			return DXGI_FORMAT_BC7_UNORM;


		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	class DirectX11Texture1D : public ITexture1D
	{

	private:

		ID3D11Texture1D* m_Texture;

		DirectX11Texture1D()
			: m_Texture( nullptr )
		{}

		DirectX11Texture1D( ID3D11Texture1D* inRaw )
			: m_Texture( inRaw )
		{}

	public:

		~DirectX11Texture1D()
		{
			Shutdown();
		}

		void Shutdown() override
		{
			if( m_Texture )
			{
				m_Texture->Release();
				m_Texture = nullptr;
			}
		}

		bool IsValid() const override
		{
			return m_Texture != nullptr;
		}

		ID3D11Texture1D** GetAddress()
		{
			return &m_Texture;
		}

		ID3D11Texture1D* Get()
		{
			return m_Texture;
		}

		uint32 GetWidth() const final
		{
			if( m_Texture == nullptr ) { return 0; }

			D3D11_TEXTURE1D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Width;
		}

		uint32 GetMipLevels() const final
		{
			if( m_Texture == nullptr ) { return 0; }

			D3D11_TEXTURE1D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.MipLevels;
		}

		bool IsDynamicResource() const final
		{
			if( m_Texture == nullptr ) { return 0; }

			D3D11_TEXTURE1D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Usage == D3D11_USAGE_DYNAMIC;
		}

		bool CanCPURead() const final
		{
			if( m_Texture == nullptr ) { return false; }

			D3D11_TEXTURE1D_DESC desc;
			m_Texture->GetDesc( &desc );

			return( ( desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ ) != 0 );
		}

		TextureBindTarget GetBindTarget() const final
		{
			if( m_Texture == nullptr ) { return TextureBindTarget::Shader; }

			D3D11_TEXTURE1D_DESC desc;
			m_Texture->GetDesc( &desc );

			switch( desc.BindFlags )
			{
			case D3D11_BIND_SHADER_RESOURCE:
				return TextureBindTarget::Shader;
			case D3D11_BIND_DEPTH_STENCIL:
				return TextureBindTarget::DepthStencil;
			case D3D11_BIND_RENDER_TARGET:
				return TextureBindTarget::Render;
			default:
				HYPERION_VERIFY( true, "Invalid texture bind target!" );
				return TextureBindTarget::Shader;
			}
		}

		TextureFormat GetFormat() const final
		{
			if( m_Texture == nullptr ) { return TextureFormat::NONE; }

			D3D11_TEXTURE1D_DESC desc;
			m_Texture->GetDesc( &desc );

			return DXGIFormatToTextureFormat( desc.Format );
		}

		void Swap( ITexture1D& Other ) final
		{
			DirectX11Texture1D* casted = dynamic_cast< DirectX11Texture1D* >( &Other );
			HYPERION_VERIFY( casted != nullptr, "Attempt to swap textures from different apis?" );

			auto tmp = m_Texture;
			m_Texture = casted->m_Texture;
			casted->m_Texture = tmp;
		}

		friend class DirectX11Graphics;
	};


	class DirectX11Texture2D : public ITexture2D
	{

	private:

		ID3D11Texture2D* m_Texture;

		DirectX11Texture2D()
			: m_Texture( nullptr )
		{}

		DirectX11Texture2D( ID3D11Texture2D* inRawTexture )
			: m_Texture( inRawTexture )
		{}

	public:

		~DirectX11Texture2D()
		{
			Shutdown();
		}

		void Shutdown() override
		{
			if( m_Texture )
			{
				m_Texture->Release();
				m_Texture = nullptr;
			}
		}

		bool IsValid() const override
		{
			return m_Texture != nullptr;
		}

		ID3D11Texture2D** GetAddress()
		{
			return &m_Texture;
		}

		ID3D11Texture2D* Get()
		{
			return m_Texture;
		}

		uint32 GetWidth() const final
		{
			if( m_Texture == nullptr ) { return 0; }

			D3D11_TEXTURE2D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Width;
		}

		uint32 GetHeight() const final
		{
			if( m_Texture == nullptr ) { return 0; }

			D3D11_TEXTURE2D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Height;
		}

		uint32 GetMipLevels() const final
		{
			if( m_Texture == nullptr ) { return 0; }

			D3D11_TEXTURE2D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.MipLevels;
		}

		bool IsDynamicResource() const final
		{
			if( m_Texture == nullptr ) { return false; }

			D3D11_TEXTURE2D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Usage == D3D11_USAGE_DYNAMIC;
		}

		bool CanCPURead() const final
		{
			if( m_Texture == nullptr ) { return false; }

			D3D11_TEXTURE2D_DESC desc;
			m_Texture->GetDesc( &desc );

			return( ( desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ ) != 0 );
		}

		TextureBindTarget GetBindTarget() const final
		{
			if( m_Texture == nullptr ) { return TextureBindTarget::Shader; }

			D3D11_TEXTURE2D_DESC desc;
			m_Texture->GetDesc( &desc );

			switch( desc.BindFlags )
			{
			case D3D11_BIND_SHADER_RESOURCE:
				return TextureBindTarget::Shader;
			case D3D11_BIND_DEPTH_STENCIL:
				return TextureBindTarget::DepthStencil;
			case D3D11_BIND_RENDER_TARGET:
				return TextureBindTarget::Render;
			default:
				HYPERION_VERIFY( true, "Invalid texture bind target!" );
				return TextureBindTarget::Shader;
			}
		}

		TextureFormat GetFormat() const final
		{
			if( m_Texture == nullptr ) { return TextureFormat::NONE; }

			D3D11_TEXTURE2D_DESC desc;
			m_Texture->GetDesc( &desc );

			return DXGIFormatToTextureFormat( desc.Format );
		}

		void Swap( ITexture2D& Other )
		{
			auto casted = dynamic_cast<DirectX11Texture2D*>( &Other );
			HYPERION_VERIFY( casted != nullptr, "Attempt to swap textures from different apis!" );

			auto tmp = m_Texture;
			m_Texture = casted->m_Texture;
			casted->m_Texture = tmp;
		}

		friend class DirectX11Graphics;
	};


	class DirectX11Texture3D : public ITexture3D
	{

	private:

		ID3D11Texture3D* m_Texture;

		DirectX11Texture3D()
			: m_Texture( nullptr )
		{}

		DirectX11Texture3D( ID3D11Texture3D* inRaw )
			: m_Texture( inRaw )
		{}

	public:

		~DirectX11Texture3D()
		{
			Shutdown();
		}

		void Shutdown() override
		{
			if( m_Texture )
			{
				m_Texture->Release();
				m_Texture = nullptr;
			}
		}

		bool IsValid() const override
		{
			return m_Texture != nullptr;
		}

		ID3D11Texture3D** GetAddress()
		{
			return &m_Texture;
		}

		ID3D11Texture3D* Get()
		{
			return m_Texture;
		}

		uint32 GetWidth() const final
		{
			if( m_Texture == nullptr ) { return 0; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Width;
		}

		uint32 GetHeight() const final
		{
			if( m_Texture == nullptr ) { return 0; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Height;
		}

		uint32 GetDepth() const final
		{
			if( m_Texture == nullptr ) { return 0; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Depth;
		}

		uint32 GetMipLevels() const final
		{
			if( m_Texture == nullptr ) { return 0; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.MipLevels;
		}

		bool IsDynamicResource() const final
		{
			if( m_Texture == nullptr ) { return false; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return desc.Usage == D3D11_USAGE_DYNAMIC;
		}

		bool CanCPURead() const final
		{
			if( m_Texture == nullptr ) { return false; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return( ( desc.CPUAccessFlags & D3D11_CPU_ACCESS_READ ) != 0 );
		}

		TextureBindTarget GetBindTarget() const final
		{
			if( m_Texture == nullptr ) { return TextureBindTarget::Shader; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			switch( desc.BindFlags )
			{
			case D3D11_BIND_SHADER_RESOURCE:
				return TextureBindTarget::Shader;
			case D3D11_BIND_DEPTH_STENCIL:
				return TextureBindTarget::DepthStencil;
			case D3D11_BIND_RENDER_TARGET:
				return TextureBindTarget::Render;
			default:
				HYPERION_VERIFY( true, "Invalid texture bind target!" );
				return TextureBindTarget::Shader;
			}
		}

		TextureFormat GetFormat() const final
		{
			if( m_Texture == nullptr ) { return TextureFormat::NONE; }

			D3D11_TEXTURE3D_DESC desc;
			m_Texture->GetDesc( &desc );

			return DXGIFormatToTextureFormat( desc.Format );
		}

		void Swap( ITexture3D& Other )
		{
			auto casted = dynamic_cast<DirectX11Texture3D*>( &Other );
			HYPERION_VERIFY( casted != nullptr, "Attempt to swap textures from different apis!" );

			auto tmp = m_Texture;
			m_Texture = casted->m_Texture;
			casted->m_Texture = tmp;
		}

		friend class DirectX11Graphics;
	};
}