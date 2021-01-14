/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/DirectX11Texture.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/DirectX11Texture.h"


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

	uint32 TranslateDXBindFlags( UINT inDXFlags )
	{
		uint32 output = 0;

		if( HYPERION_HAS_FLAG( inDXFlags, D3D11_BIND_SHADER_RESOURCE ) )
		{
			output |= (uint32) TextureBindTarget::Shader;
		}

		if( HYPERION_HAS_FLAG( inDXFlags, D3D11_BIND_DEPTH_STENCIL ) )
		{
			output |= (uint32) TextureBindTarget::DepthStencil;
		}

		if( HYPERION_HAS_FLAG( inDXFlags, D3D11_BIND_RENDER_TARGET ) )
		{
			output |= (uint32) TextureBindTarget::Render;
		}

		return output;
	}


	UINT TranslateHyperionBindFlags( uint32 inFlags )
	{
		uint32 output = 0;

		if( HYPERION_HAS_FLAG( inFlags, TextureBindTarget::DepthStencil ) )
		{
			output |= D3D11_BIND_DEPTH_STENCIL;
		}

		if( HYPERION_HAS_FLAG( inFlags, TextureBindTarget::Render ) )
		{
			output |= D3D11_BIND_RENDER_TARGET;
		}

		if( HYPERION_HAS_FLAG( inFlags, TextureBindTarget::Shader ) )
		{
			output |= D3D11_BIND_SHADER_RESOURCE;
		}

		return output;
	}
}