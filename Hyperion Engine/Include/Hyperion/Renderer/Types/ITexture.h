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