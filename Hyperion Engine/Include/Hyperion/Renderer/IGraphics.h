/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/IGraphics.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/DataTypes.h"


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class IBuffer;
	struct BufferParameters;

	class ITexture1D;
	class ITexture2D;
	class ITexture3D;

	struct Texture1DParameters;
	struct Texture2DParameters;
	struct Texture3DParameters;

	class IRenderTarget;


	class IGraphics
	{

	public:

		virtual ~IGraphics()
		{}

		virtual bool SetResolution( const ScreenResolution& inResolution ) = 0;
		virtual void SetVSync( bool bVSync ) = 0;

		virtual bool Initialize( const IRenderOutput& ) = 0;
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

		virtual std::shared_ptr< IRenderTarget > GetRenderTarget() = 0;
		virtual std::shared_ptr< ITexture2D > GetBackBuffer() = 0;

		virtual std::vector< ScreenResolution > GetAvailableResolutions() = 0;

		virtual bool AllowAsyncTextureCreation() const = 0;

		virtual std::shared_ptr< IBuffer > CreateBuffer( const BufferParameters& ) = 0;

		// Texture Creation
		virtual std::shared_ptr< ITexture1D > CreateTexture1D( const Texture1DParameters& ) = 0;
		virtual std::shared_ptr< ITexture2D > CreateTexture2D( const Texture2DParameters& ) = 0;
		virtual std::shared_ptr< ITexture3D > CreateTexture3D( const Texture3DParameters& ) = 0;

		// Texture Copying
		virtual bool CopyTexture2D( std::shared_ptr< ITexture2D >& Source, std::shared_ptr< ITexture2D >& Dest ) = 0;
		virtual bool CopyLODTexture2D( std::shared_ptr< ITexture2D >& Source, std::shared_ptr< ITexture2D >& Dest,
									   uint32 SourceX, uint32 SourceY, uint32 Width, uint32 Height, uint32 DestX, uint32 DestY, uint8 SourceLevel, uint8 TargetLevel ) = 0;

		// Render Target Creation
		virtual std::shared_ptr< IRenderTarget > CreateRenderTarget( std::shared_ptr< ITexture2D > inTarget ) = 0;

		friend class TextureCache;
	};

}