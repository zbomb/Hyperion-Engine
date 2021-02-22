/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/IGraphics.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/DataTypes.h"
#include "Hyperion/Renderer/Resources/RTexture.h"
#include "Hyperion/Renderer/Resources/RBuffer.h"
#include "Hyperion/Renderer/Resources/RShader.h"
#include "Hyperion/Renderer/Resources/RMesh.h"
#include "Hyperion/Library/Color.h"


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class IBuffer;
	struct BufferParameters;
	class RRenderTarget;
	struct AABB;
	class BatchCollector;

	/*
	*	Enums
	*/
	enum class AlphaBlendingState
	{
		Disabled	= 0,
		Enabled		= 1
	};

	enum class DepthStencilState
	{
		DepthAndStencilEnabled		= 0,
		DepthDisabledStencilEnabled	= 1
	};


	class IGraphics
	{

	public:

		/*
		*	Structures that are used to encapsulate parameters for API calls
		*/
		struct InitializationParameters
		{
			void* ptrOSWindow;
			ScreenResolution startupResolution;
			bool bEnableVSync;
			uint32 depthStencilWidth;
			uint32 depthStencilHeight;
		};


		struct ResourceRangeParameters
		{
			uint32 sourceX;
			uint32 sourceY;
			uint32 sourceZ;
			uint32 sourceMip;

			uint32 destX;
			uint32 destY;
			uint32 destZ;
			uint32 destMip;

			uint32 rangeWidth;
			uint32 rangeHeight;
			uint32 rangeDepth;
		};


	public:

		virtual ~IGraphics()
		{}

		/*
		*	Init and Shutdown
		*/
		virtual bool Initialize( const InitializationParameters& inParameters ) = 0;
		virtual void Shutdown() = 0;

		/*	
		*	Resolution Managment
		*/
		virtual bool UpdateResolution( const ScreenResolution& inResolution ) = 0;
		virtual ScreenResolution GetResolution() = 0;
		virtual void GetAvailableResolutions( std::vector< ScreenResolution >& outResolutions ) = 0;
		virtual float GetDisplayRefreshRate() = 0;

		/*
		*	Depth Stencil Managment
		*/
		virtual std::pair< uint32, uint32 > GetDepthStencilResolution() = 0;
		virtual bool SetDepthStencilResolution( uint32 inWidth, uint32 inHeight ) = 0;
		virtual bool ClearDepthStencil( float inDepth = 1.f, uint8 inStencil = 0 ) = 0;

		/*
		*	VSync Managment
		*/
		virtual bool GetVSyncEnabled() = 0;
		virtual bool SetVSyncEnabled( bool inEnabled ) = 0;

		/*
		*	Depth Stencil and Alpha Blending Managment
		*/
		virtual void SetAlphaBlendingState( AlphaBlendingState inState ) = 0;
		virtual AlphaBlendingState GetAlphaBlendingState() = 0;
		virtual void SetDepthStencilState( DepthStencilState inState ) = 0;
		virtual DepthStencilState GetDepthStencilState() = 0;

		/*
		*	Render Target Managment
		*/
		virtual std::shared_ptr< RTexture2D > GetBackBuffer() = 0;
		virtual bool ClearRenderTarget( const std::shared_ptr< RTexture2D >& inTexture, float r = 0.f, float g = 0.f, float b = 0.f, float a = 0.f ) = 0;
		virtual bool AttachRenderTarget( const std::shared_ptr< RTexture2D >& inTexture, bool bUseDepthStencil = true ) = 0;
		virtual bool AttachRenderTargets( const std::vector< std::shared_ptr< RTexture2D > >& inTargets, bool bUseDepthStencil = true ) = 0;
		virtual void DetachRenderTargets() = 0;

		/*
		*	Debugging
		*/
		virtual void GetFloorMesh( std::shared_ptr< RBuffer >& outVertexBuffer, std::shared_ptr< RBuffer >& outIndexBuffer, std::vector< Matrix >& outMatricies ) = 0;

		/*
		*	Rendering API
		*/
		virtual void RenderScreenQuad() = 0;
		virtual void AttachMesh( const std::shared_ptr< RBuffer >& inVertexBuffer, const std::shared_ptr< RBuffer >& inIndexBuffer, uint32 inIndexCount ) = 0;
		virtual void DetachMesh() = 0;
		virtual void Render( uint32 inInstanceCount ) = 0;

		virtual void DisplayFrame() = 0;

		/*
		*	General Resource Managment
		*/
		virtual bool IsAsyncResourceCreationAllowed() const = 0;

		/*
		*	Texture Managment
		*/
		virtual std::shared_ptr< RTexture1D > CreateTexture1D( const TextureParameters& inParams ) = 0;
		virtual std::shared_ptr< RTexture2D > CreateTexture2D( const TextureParameters& inParams ) = 0;
		virtual std::shared_ptr< RTexture3D > CreateTexture3D( const TextureParameters& inParams ) = 0;

		virtual std::shared_ptr< RTexture1D > CreateTexture1D() = 0;
		virtual std::shared_ptr< RTexture2D > CreateTexture2D() = 0;
		virtual std::shared_ptr< RTexture3D > CreateTexture3D() = 0;

		/*
		*	Buffer Managment
		*/
		virtual std::shared_ptr< RBuffer > CreateBuffer( const BufferParameters& inParams ) = 0;
		virtual std::shared_ptr< RBuffer > CreateBuffer( BufferType inType ) = 0;

		/*
		*	Resource Copying
		*/
		virtual bool CopyResource( const std::shared_ptr< RGraphicsResource >& inSource, const std::shared_ptr< RGraphicsResource >& inTarget ) = 0;
		virtual bool CopyResourceRange( const std::shared_ptr< RGraphicsResource >& inSource, const std::shared_ptr< RGraphicsResource >& inTarget, const ResourceRangeParameters& inParams ) = 0;

		/*
		*	Shader Creation
		*/
		virtual std::shared_ptr< RVertexShader > CreateVertexShader( VertexShaderType inType ) = 0;
		virtual std::shared_ptr< RGeometryShader > CreateGeometryShader( GeometryShaderType inType ) = 0;
		virtual std::shared_ptr< RPixelShader > CreatePixelShader( PixelShaderType inType ) = 0;
		virtual std::shared_ptr< RComputeShader > CreateComputeShader( ComputeShaderType inType ) = 0;
		virtual std::shared_ptr< RPostProcessShader > CreatePostProcessShader( PostProcessShaderType inType ) = 0;

	};

}