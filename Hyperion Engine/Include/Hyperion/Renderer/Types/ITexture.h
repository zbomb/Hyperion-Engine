/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Types/ITexture.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Types/Resource.h"
#include "Hyperion/Renderer/DataTypes.h"


namespace Hyperion
{


	enum class TextureBindTarget
	{
		Shader = 1,
		Output = 2,
		DepthStencil = 3
	};


	enum class TextureFormat
	{
		NONE = 0,

		/*
		 * 8-bit Types
		*/
		// Unsigned Normals (converted to floats in shaders) [0,1]
		R_8BIT_UNORM = 1,
		RG_8BIT_UNORM = 2,
		RGBA_8BIT_UNORM = 4,
		RGBA_8BIT_UNORM_SRGB = 5,

		// Signed Normals (converted to floats in shaders) [-1,1]
		R_8BIT_SNORM = 6,
		RG_8BIT_SNORM = 7,
		RGBA_8BIT_SNORM = 9,

		// Unsigned Integers (not converted to floats)
		R_8BIT_UINT = 10,
		RG_8BIT_UINT = 11,
		RGBA_8BIT_UINT = 13,

		// Signed Integers (not converted to floats)
		R_8BIT_SINT = 14,
		RG_8BIT_SINT = 15,
		RGBA_8BIT_SINT = 17,

		/*
		 * 16-bit Types
		*/
		// Unsigned normals (converted to floats in shaders) [0,1]
		R_16BIT_UNORM = 18,
		RG_16BIT_UNORM = 19,
		RGBA_16BIT_UNORM = 21,

		// Signed normals (converted to floats in shaders) [-1,1]
		R_16BIT_SNORM = 23,
		RG_16BIT_SNORM = 24,
		RGBA_16BIT_SNORM = 26,

		// Unisnged Integers (not converted to floats)
		R_16BIT_UINT = 27,
		RG_16BIT_UINT = 28,
		RGBA_16BIT_UINT = 30,

		// Signed Integers (not converted to floats)
		R_16BIT_SINT = 31,
		RG_16BIT_SINT = 32,
		RGBA_16BIT_SINT = 34,

		// Floats 
		R_16BIT_FLOAT = 35,
		RG_16BIT_FLOAT = 36,
		RGBA_16BIT_FLOAT = 38,

		/*
		 * 32-bit Types
		*/

		// Unsigned integers (not converted to floats)
		R_32BIT_UINT = 48,
		RG_32BIT_UINT = 49,
		RGB_32BIT_UINT = 50,
		RGBA_32BIT_UINT = 51,

		// Signed integers (not converted to floats)
		R_32BIT_SINT = 52,
		RG_32BIT_SINT = 53,
		RGB_32BIT_SINT = 54,
		RGBA_32BIT_SINT = 55,

		// Floats
		R_32BIT_FLOAT = 56,
		RG_32BIT_FLOAT = 57,
		RGB_32BIT_FLOAT = 58,
		RGBA_32BIT_FLOAT = 59,

		/*
			Compressed Types
		*/
		RGB_DXT_1 = 60,
		RGBA_DXT_5 = 61,
		RGB_BC_7 = 62,
		RGBA_BC_7 = 63
	};


	struct RawImageData
	{

		uint32 Width;
		uint32 Height;

		std::vector< byte > Data;

	};

	
	struct Texture1DParameters
	{
		Texture1DParameters()
			: Width( 0 ), MipLevels( 0 ), Dynamic( false ), CanCPURead( false ),
			Target( TextureBindTarget::Shader ), Format( TextureFormat::RGBA_8BIT_UNORM ), Data( nullptr )
		{}

		uint32 Width;
		uint32 MipLevels;
		bool Dynamic;
		bool CanCPURead;
		TextureBindTarget Target;
		TextureFormat Format;

		const void* Data;
	};

	struct Texture2DMipData
	{
		const void* Data;
		uint32 RowDataSize;

		Texture2DMipData()
			: Data( nullptr ), RowDataSize( 0 )
		{}
	};

	struct Texture2DParameters
	{
		Texture2DParameters()
			: Width( 0 ), Height( 0 ), MipLevels( 0 ), Dynamic( false ), CanCPURead( false ),
			Target( TextureBindTarget::Shader ), Format( TextureFormat::RGBA_8BIT_UNORM )

		{}

		uint32 Width;
		uint32 Height;
		uint32 MipLevels;
		bool Dynamic;
		bool CanCPURead;
		TextureBindTarget Target;
		TextureFormat Format;

		std::vector< Texture2DMipData > Data;
	};

	struct Texture3DParameters
	{
		Texture3DParameters()
			: Width( 0 ), Height( 0 ), Depth( 0 ), MipLevels( 0 ), Dynamic( false ), CanCPURead( false ),
			Target( TextureBindTarget::Shader ), Format( TextureFormat::RGBA_8BIT_UNORM ), Data( nullptr ),
			RowDataSize( 0 ), LayerDataSize( 0 )
		{}

		uint32 Width;
		uint32 Height;
		uint32 Depth;
		uint32 MipLevels;
		bool Dynamic;
		bool CanCPURead;
		TextureBindTarget Target;
		TextureFormat Format;

		const void* Data;
		uint32 RowDataSize;
		uint32 LayerDataSize;
	};


	class ITextureBase : public IAPIResourceBase
	{

	public:

		virtual uint8 GetDimensions() const = 0;
		virtual uint32 GetWidth() const = 0;
		virtual uint32 GetHeight() const = 0;
		virtual uint32 GetDepth() const = 0;
		virtual uint32 GetMipLevels() const = 0;
		virtual bool IsDynamicResource() const = 0;
		virtual bool CanCPURead() const = 0;
		virtual TextureBindTarget GetBindTarget() const = 0;
		virtual TextureFormat GetFormat() const = 0;

	};


	class ITexture1D : public ITextureBase
	{

	public:

		virtual ~ITexture1D() {};
		inline uint8 GetDimensions() const final { return 1; }
		inline uint32 GetHeight() const final { return 0; }
		inline uint32 GetDepth() const final { return 0; }

		virtual void Swap( ITexture1D& ) = 0;

	};

	class ITexture2D : public ITextureBase
	{

	public:

		virtual ~ITexture2D() {};
		inline uint8 GetDimensions() const final { return 2; }
		inline uint32 GetDepth() const final { return 0; }

		virtual void Swap( ITexture2D& ) = 0;

	};

	class ITexture3D : public ITextureBase
	{

	public:

		virtual ~ITexture3D() {};
		inline uint8 GetDimensions() const final { return 3; }

		virtual void Swap( ITexture3D& ) = 0;

	};

}