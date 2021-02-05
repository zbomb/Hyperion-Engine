/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/IGraphics.h
	� 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/DataTypes.h"
#include "Hyperion/Renderer/Resources/RTexture.h"
#include "Hyperion/Renderer/Resources/RBuffer.h"
#include "Hyperion/Renderer/Resources/RShader.h"
#include "Hyperion/Renderer/Resources/RMesh.h"
#include "Hyperion/Renderer/Resources/RDepthStencil.h" // TODO: Remove this?
#include "Hyperion/Library/Color.h"
#include "Hyperion/Renderer/LightBuffer.h"


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class IBuffer;
	struct BufferParameters;
	class RRenderTarget;
	struct AABB;


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

		virtual void SetCameraInfo( const ViewState& inView ) = 0;
		virtual bool CheckViewCull( const Transform& inTransform, const AABB& inBounds ) = 0;

		virtual void EnableAlphaBlending() = 0;
		virtual void DisableAlphaBlending() = 0;
		virtual bool IsAlphaBlendingEnabled() = 0;

		virtual void EnableZBuffer() = 0;
		virtual void DisableZBuffer() = 0;
		virtual bool IsZBufferEnabled() = 0;

		virtual std::shared_ptr< RRenderTarget > GetRenderTarget() = 0;
		virtual std::shared_ptr< RTexture2D > GetBackBuffer() = 0;
		virtual std::shared_ptr< RDepthStencil > GetDepthStencil() = 0;

		virtual std::vector< ScreenResolution > GetAvailableResolutions() = 0;

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
		virtual bool AllowAsyncTextureCreation() const = 0;


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
		virtual std::shared_ptr< RRenderTarget > CreateRenderTarget( const std::shared_ptr< RTexture2D >& inTarget ) = 0;
		virtual void ClearRenderTarget( const std::shared_ptr< RRenderTarget >& inTarget, const Color4F& inColor ) = 0;

		// Depth Buffer Creation/Resizing
		// TODO: Rename to 'DepthBuffer'?
		virtual std::shared_ptr< RDepthStencil > CreateDepthStencil( uint32 inWidth, uint32 inHeight ) = 0;
		virtual bool ResizeDepthStencil( const std::shared_ptr< RDepthStencil >& inStencil, uint32 inWidth, uint32 inHeight ) = 0;
		virtual void ClearDepthStencil( const std::shared_ptr< RDepthStencil >& inStenci, const Color4F& inColor ) = 0;

		virtual std::shared_ptr< RViewClusters > CreateViewClusters() = 0;
		virtual std::shared_ptr< RLightBuffer > CreateLightBuffer() = 0;
		
		// Rendering
		virtual void SetNoRenderTargetAndClusterWriteAccess( const std::shared_ptr< RViewClusters >& inClusters ) = 0;
		virtual void ClearClusterWriteAccess() = 0;
		virtual void SetGBufferRenderTarget( const std::shared_ptr< GBuffer >& inGBuffer, const std::shared_ptr< RDepthStencil >& inStencil ) = 0;

		virtual void SetRenderTarget( const std::shared_ptr< RRenderTarget >& inTarget, const std::shared_ptr< RDepthStencil >& inStencil ) = 0;
		virtual void DetachRenderTarget() = 0;

		virtual void RenderBatch( const std::shared_ptr< RBuffer >& inVertexBuffer, const std::shared_ptr< RBuffer >& inIndexBuffer, uint32 inIndexCount ) = 0;
		virtual void RenderScreenQuad() = 0;
		virtual void GetDebugFloorQuad( std::shared_ptr< RBuffer >& outVertexBuffer, std::shared_ptr< RBuffer >& outIndexBuffer ) = 0;

		// Matricies
		virtual void CalculateViewMatrix( const ViewState& inView, Matrix& outViewMatrix ) = 0;
		virtual void CalculateProjectionMatrix( const ScreenResolution& inResolution, float inFOV, Matrix& outProjMatrix ) = 0;
		virtual void CalculateWorldMatrix( const Transform& inTransform, Matrix& outWorldMatrix ) = 0;
		virtual void CalculateOrthoMatrix( const ScreenResolution& inResolution, Matrix& outOrthoMatrix ) = 0;
		virtual void CalculateScreenViewMatrix( Matrix& outMatrix ) = 0;

		// Shader Creation
		virtual std::shared_ptr< RVertexShader > CreateVertexShader( VertexShaderType inType ) = 0;
		virtual std::shared_ptr< RGeometryShader > CreateGeometryShader( GeometryShaderType inType ) = 0;
		virtual std::shared_ptr< RPixelShader > CreatePixelShader( PixelShaderType inType ) = 0;
		virtual std::shared_ptr< RComputeShader > CreateComputeShader( ComputeShaderType inType ) = 0;

		friend class TextureCache;
	};

}