/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resources/RTexture.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RGraphicsResource.h"


namespace Hyperion
{
	/*
	*	Texture Mip Data Structure
	*/
	struct TextureMipData
	{
		void* Data;
		uint32 RowSize;
		uint32 LayerSize;

		TextureMipData()
			: Data( nullptr ), RowSize( 0 ), LayerSize( 0 )
		{}
	};

	/*
	*	Texture Parameters Structure
	*/
	struct TextureParameters
	{
		uint32 Width;
		uint32 Height;
		uint32 Depth;
		uint32 MipCount;
		bool bDynamic;
		bool bCPURead;
		uint32 ResourceAccess;
		TextureFormat Format;
		bool bAutogenMips;
		std::vector< TextureMipData > Data;
		uint32 AssetIdentifier;

		TextureParameters()
			: Width( 0 ), Height( 0 ), Depth( 0 ), bDynamic( false ), bCPURead( false ), ResourceAccess( RENDERER_RESOURCE_ACCESS_NONE ),
			Format( TextureFormat::NONE ), bAutogenMips( false ), AssetIdentifier( ASSET_INVALID ), MipCount( 1 )
		{}
	};

	/*
	*	Texture Base Interface
	*/
	class RTextureBase : public RGraphicsResource
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
		virtual TextureFormat GetFormat() const = 0;
		virtual uint32 GetAssetIdentifier() const = 0;

		virtual bool IsValid() const = 0;
		virtual void Shutdown() = 0;
	};

	/*
	*	Texture1D Interface
	*/
	class RTexture1D : public RTextureBase
	{

	public:

		virtual ~RTexture1D() {}

		inline uint8 GetDimensionCount() const final	{ return 1; }
		inline uint32 GetHeight() const final			{ return 1; }
		inline uint32 GetDepth() const final			{ return 1; }

		virtual void Swap( RTexture1D& Other ) = 0;

	};

	/*
	*	Texture2D Interface
	*/
	class RTexture2D : public RTextureBase
	{

	public:

		virtual ~RTexture2D() {}

		inline uint8 GetDimensionCount() const final	{ return 2; }
		inline uint32 GetDepth() const final			{ return 1; }

		virtual void Swap( RTexture2D& Other ) = 0;

	};

	/*
	*	Texture3D Interface
	*/
	class RTexture3D : public RTextureBase
	{

	public:

		virtual ~RTexture3D() {}

		inline uint8 GetDimensionCount() const final { return 3; }

		virtual void Swap( RTexture3D& Other ) = 0;

	};

}