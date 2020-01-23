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

		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	class DirectX11Texture1D : public ITexture1D
	{

	private:

		ID3D11Texture1D* m_Texture;
		Texture1DParameters m_Parameters;

		DirectX11Texture1D( const Texture1DParameters& inParams )
			: m_Texture( nullptr ), m_Parameters( inParams )
		{}

	public:

		DirectX11Texture1D() = delete;

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
			return m_Parameters.Width;
		}

		uint32 GetMipLevels() const final
		{
			return m_Parameters.MipLevels;
		}

		bool IsDynamicResource() const final
		{
			return m_Parameters.Dynamic;
		}

		bool CanCPURead() const final
		{
			return m_Parameters.CanCPURead;
		}

		TextureBindTarget GetBindTarget() const final
		{
			return m_Parameters.Target;
		}

		TextureFormat GetFormat() const final
		{
			return m_Parameters.Format;
		}

		friend class DirectX11Graphics;
	};


	class DirectX11Texture2D : public ITexture2D
	{

	private:

		ID3D11Texture2D* m_Texture;
		Texture2DParameters m_Parameters;

		DirectX11Texture2D( const Texture2DParameters& inParams )
			: m_Texture( nullptr ), m_Parameters( inParams )
		{}

	public:

		DirectX11Texture2D() = delete;

		~DirectX11Texture2D()
		{
			Shutdown();
		}

		void Shutdown() override
		{
			if( m_Texture )
			{
				Console::WriteLine( "[DEBUG] D3DTEXTURE2D SHUTDOWN" );
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
			return m_Parameters.Width;
		}

		uint32 GetHeight() const final
		{
			return m_Parameters.Height;
		}

		uint32 GetMipLevels() const final
		{
			return m_Parameters.MipLevels;
		}

		bool IsDynamicResource() const final
		{
			return m_Parameters.Dynamic;
		}

		bool CanCPURead() const final
		{
			return m_Parameters.CanCPURead;
		}

		TextureBindTarget GetBindTarget() const final
		{
			return m_Parameters.Target;
		}

		TextureFormat GetFormat() const final
		{
			return m_Parameters.Format;
		}

		friend class DirectX11Graphics;
	};


	class DirectX11Texture3D : public ITexture3D
	{

	private:

		ID3D11Texture3D* m_Texture;
		Texture3DParameters m_Parameters;

		DirectX11Texture3D( const Texture3DParameters& inParams )
			: m_Texture( nullptr ), m_Parameters( inParams )
		{}

	public:

		DirectX11Texture3D() = delete;

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
			return m_Parameters.Width;
		}

		uint32 GetHeight() const final
		{
			return m_Parameters.Height;
		}

		uint32 GetDepth() const final
		{
			return m_Parameters.Depth;
		}

		uint32 GetMipLevels() const final
		{
			return m_Parameters.MipLevels;
		}

		bool IsDynamicResource() const final
		{
			return m_Parameters.Dynamic;
		}

		bool CanCPURead() const final
		{
			return m_Parameters.CanCPURead;
		}

		TextureBindTarget GetBindTarget() const final
		{
			return m_Parameters.Target;
		}

		TextureFormat GetFormat() const final
		{
			return m_Parameters.Format;
		}

		friend class DirectX11Graphics;
	};
}