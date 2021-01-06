/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/IGraphics.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/DataTypes.h"
#include "Hyperion/Renderer/Resource/Texture.h"
#include "Hyperion/Renderer/Resource/Buffer.h"


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class IBuffer;
	struct BufferParameters;

	class RRenderTarget;


	class IGraphics
	{

	public:

		virtual ~IGraphics()
		{}

		virtual bool SetResolution( const ScreenResolution& inResolution ) = 0;
		virtual void SetVSync( bool bVSync ) = 0;

		virtual bool Initialize( void* pWindow ) = 0;
		virtual void Shutdown() = 0;

		virtual bool IsRunning() const = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void EnableAlphaBlending() = 0;
		virtual void DisableAlphaBlending() = 0;
		virtual bool IsAlphaBlendingEnabled() = 0;

		virtual void EnableZBuffer() = 0;
		virtual void DisableZBuffer() = 0;
		virtual bool IsZBufferEnabled() = 0;

		virtual std::shared_ptr< RRenderTarget > GetRenderTarget() = 0;
		virtual std::shared_ptr< RTexture2D > GetBackBuffer() = 0;

		virtual std::vector< ScreenResolution > GetAvailableResolutions() = 0;

		virtual bool AllowAsyncTextureCreation() const = 0;

		// Buffer Creation
		virtual std::shared_ptr< RBuffer > CreateBuffer( const BufferParameters& ) = 0;
		virtual std::shared_ptr< RBuffer > CreateBuffer( BufferType ty = BufferType::Vertex ) = 0;

		// Texture Creation
		virtual std::shared_ptr< RTexture1D > CreateTexture1D( const TextureParameters& ) = 0;
		virtual std::shared_ptr< RTexture2D > CreateTexture2D( const TextureParameters& ) = 0;
		virtual std::shared_ptr< RTexture3D > CreateTexture3D( const TextureParameters& ) = 0;
		virtual std::shared_ptr< RTexture1D > CreateTexture1D() = 0;
		virtual std::shared_ptr< RTexture2D > CreateTexture2D() = 0;
		virtual std::shared_ptr< RTexture3D > CreateTexture3D() = 0;

		// Texture Copying
		virtual bool CopyTexture1D( std::shared_ptr< RTexture1D >& inSource, std::shared_ptr< RTexture1D >& inDest ) = 0;
		virtual bool CopyTexture2D( std::shared_ptr< RTexture2D >& inSource, std::shared_ptr< RTexture2D >& inDest ) = 0;
		virtual bool CopyTexture3D( std::shared_ptr< RTexture3D >& inSource, std::shared_ptr< RTexture3D >& inDest ) = 0;

		virtual bool CopyTexture1DRegion( std::shared_ptr< RTexture1D >& inSource, std::shared_ptr< RTexture1D >& inDest,
										 uint32 sourceX, uint32 inWidth, uint32 destX, uint8 sourceMip, uint8 targetMip ) = 0;
		virtual bool CopyTexture2DRegion( std::shared_ptr< RTexture2D >& inSource, std::shared_ptr< RTexture2D >& inDest, uint32 sourceX, uint32 sourceY,
									   uint32 inWidth, uint32 inHeight, uint32 destX, uint32 destY, uint8 sourceMip, uint8 targetMip ) = 0;
		virtual bool CopyTexture3DRegion( std::shared_ptr< RTexture3D >& inSource, std::shared_ptr< RTexture3D >& inDest, uint32 sourceX, uint32 sourceY, uint32 sourceZ,
									   uint32 inWidth, uint32 inHeight, uint32 inDepth, uint32 destX, uint32 destY, uint32 destZ, uint8 sourceMip, uint8 targetMip ) = 0;

		virtual bool CopyTexture1DMip( std::shared_ptr< RTexture1D >& inSource, std::shared_ptr< RTexture1D >& inDest, uint8 sourceMip, uint8 destMip ) = 0;
		virtual bool CopyTexture2DMip( std::shared_ptr< RTexture2D >& inSource, std::shared_ptr< RTexture2D >& inDest, uint8 sourceMip, uint8 destMip ) = 0;
		virtual bool CopyTexture3DMip( std::shared_ptr< RTexture3D >& inSource, std::shared_ptr< RTexture3D >& inDest, uint8 sourceMip, uint8 destMip ) = 0;

		// Render Target Creation
		virtual std::shared_ptr< RRenderTarget > CreateRenderTarget( std::shared_ptr< RTexture2D > inTarget ) = 0;

		friend class TextureCache;
	};

}