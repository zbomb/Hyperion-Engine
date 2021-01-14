/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resource/Texture.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{

	enum class TextureBindTarget
	{
		None			= 0b0,
		Shader			= 0b1,
		Render			= 0b10,
		DepthStencil	= 0b100
	};


	struct TextureMipData
	{
		void* Data;
		uint32 RowSize;
		uint32 LayerSize;

		TextureMipData()
			: Data( nullptr ), RowSize( 0 ), LayerSize( 0 )
		{}
	};


	struct TextureParameters
	{
		uint32 Width;
		uint32 Height;
		uint32 Depth;
		bool bDynamic;
		bool bCPURead;
		uint32 BindTargets;
		TextureFormat Format;
		bool bAutogenMips;

		std::vector< TextureMipData > Data;

		TextureParameters()
			: Width( 0 ), Height( 1 ), Depth( 1 ), bDynamic( false ), bCPURead( false ),
			BindTargets( (uint32)TextureBindTarget::Shader ), Format( TextureFormat::NONE ), bAutogenMips( false )
		{}
	};



	class RTextureBase
	{

	public:

		virtual ~RTextureBase() {}

		virtual uint8 GetDimensionCount() const = 0;
		virtual uint8 GetMipCount() const = 0;
		virtual uint32 GetWidth() const = 0;
		virtual uint32 GetHeight() const = 0;
		virtual uint32 GetDepth() const = 0;
		virtual bool IsDynamic() const = 0;
		virtual bool CanCPURead() const = 0;
		virtual uint32 GetBindTargets() const = 0;
		virtual TextureFormat GetFormat() const = 0;

		virtual bool IsValid() const = 0;
		virtual void Shutdown() = 0;

		bool HasBindTarget( TextureBindTarget inTarget )
		{
			return( ( GetBindTargets() & (uint32)inTarget ) == (uint32)inTarget );
		}
	};


	class RTexture1D : public RTextureBase
	{

	public:

		virtual ~RTexture1D() {}

		inline uint8 GetDimensionCount() const final	{ return 1; }
		inline uint32 GetHeight() const final			{ return 1; }
		inline uint32 GetDepth() const final			{ return 1; }

		virtual void Swap( RTexture1D& Other ) = 0;

	};


	class RTexture2D : public RTextureBase
	{

	public:

		virtual ~RTexture2D() {}

		inline uint8 GetDimensionCount() const final	{ return 2; }
		inline uint32 GetDepth() const final			{ return 1; }

		virtual void Swap( RTexture2D& Other ) = 0;

	};


	class RTexture3D : public RTextureBase
	{

	public:

		virtual ~RTexture3D() {}

		inline uint8 GetDimensionCount() const final { return 3; }

		virtual void Swap( RTexture3D& Other ) = 0;
	
	};

}