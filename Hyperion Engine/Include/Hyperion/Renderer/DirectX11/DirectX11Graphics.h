/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DirectX11Graphics.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"

#ifdef HYPERION_SUPPORT_DIRECTX

#include "Hyperion/Renderer/IGraphics.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"
#include "Hyperion/Core/String.h"
#include "Hyperion/Renderer/Resources/RMesh.h"
#include "Hyperion/Renderer/DirectX11/DirectX11Frustum.h"


namespace Hyperion
{

	// Forward Declarations
	class DirectX11RenderTarget;
	class DirectX11Texture2D;
	class DirectX11DepthStencil;
	


	class DirectX11Graphics : public IGraphics
	{

		template< typename _Ty >
		using ComPtr = Microsoft::WRL::ComPtr< _Ty >;

	private:

		/*
			Data Members
		*/
		HWND m_Output;
		ScreenResolution m_Resolution;
		bool m_bVSync;
		bool m_bRunning;
		bool m_bDepthEnabled;

		uint32 m_GraphicsMemory;
		String m_GraphicsDevice;

		DirectX11Frustum m_ViewFrustum;

		/*
			Resource Pointers
		*/
		ComPtr< ID3D11Debug > m_Debug;
		ComPtr< IDXGISwapChain > m_SwapChain;
		ComPtr< ID3D11Device > m_Device;
		ComPtr< ID3D11DeviceContext > m_DeviceContext;
		ComPtr< ID3D11RasterizerState > m_RasterizerState;
		ComPtr< ID3D11DepthStencilState > m_DepthStencilState;
		ComPtr< ID3D11DepthStencilState > m_DepthDisabledState;
		ComPtr< ID3D11BlendState > m_BlendState;
		ComPtr< ID3D11BlendState > m_BlendDisabledState;

		std::shared_ptr< DirectX11RenderTarget > m_RenderTarget;
		std::shared_ptr< DirectX11Texture2D > m_BackBuffer;
		std::shared_ptr< DirectX11DepthStencil > m_DepthStencil;

		ComPtr< ID3D11Buffer > m_ScreenVertexList;
		ComPtr< ID3D11Buffer > m_ScreenIndexList;

		// DEBUG 
		// Floor, just so we have something to cast lights and shadows against
		std::shared_ptr< RBuffer > m_FloorVertexBuffer;
		std::shared_ptr< RBuffer > m_FloorIndexBuffer;

		/*
			Matricies
		*/
		DirectX::XMMATRIX m_WorldMatrix, m_ProjectionMatrix, m_OrthoMatrix, m_ScreenViewMatrix, m_ViewMatrix;

		/*
		*	Camera Info
		*/
		DirectX::XMFLOAT3 m_CameraPosition;
		DirectX::XMVECTOR m_CameraRotation;
		float m_FOV;

		/*
			Helper Functions
		*/
		bool InitializeResources( HWND Target, ScreenResolution& Resolution );
		void ShutdownResources();
		void GenerateMatricies( const ScreenResolution& inRes, float inFOV, float inNear, float inFar );
		void GenerateScreenGeometry( uint32 inWidth, uint32 inHeight );

		void GenerateFloorGeometry();

	public:

		DirectX11Graphics();
		~DirectX11Graphics();

		bool SetResolution( const ScreenResolution& inResolution ) override;
		void SetVSync( bool bVSync ) override;

		bool Initialize( void* pWindow ) override;
		void Shutdown() override;

		bool IsRunning() const override;

		void BeginFrame() override;
		void EndFrame() override;

		void SetCameraInfo( const ViewState& inView ) override;
		bool CheckViewCull( const Transform& inTransform, const AABB& inBounds ) override;

		void EnableAlphaBlending() override;
		void DisableAlphaBlending() override;
		bool IsAlphaBlendingEnabled() override;

		void EnableZBuffer() override;
		void DisableZBuffer() override;
		bool IsZBufferEnabled() override;

		std::shared_ptr< RRenderTarget > GetRenderTarget() override;
		std::shared_ptr< RTexture2D > GetBackBuffer() override;
		std::shared_ptr< RDepthStencil > GetDepthStencil() override;

		std::vector< ScreenResolution > GetAvailableResolutions() override;

		inline bool AllowAsyncTextureCreation() const override { return true; }

		std::shared_ptr< RBuffer > CreateBuffer( const BufferParameters& inParams ) override;
		std::shared_ptr< RBuffer > CreateBuffer( BufferType ty = BufferType::Vertex ) override;
		
		// Texture Creation
		std::shared_ptr< RTexture1D > CreateTexture1D( const TextureParameters& ) final;
		std::shared_ptr< RTexture2D > CreateTexture2D( const TextureParameters& ) final;
		std::shared_ptr< RTexture3D > CreateTexture3D( const TextureParameters& ) final;
		std::shared_ptr< RTexture1D > CreateTexture1D() final;
		std::shared_ptr< RTexture2D > CreateTexture2D() final;
		std::shared_ptr< RTexture3D > CreateTexture3D() final;

		// Texture Copying
		bool CopyTexture1D( std::shared_ptr< RTexture1D >& inSource, std::shared_ptr< RTexture1D >& inDest ) final;
		bool CopyTexture2D( std::shared_ptr< RTexture2D >& inSource, std::shared_ptr< RTexture2D >& inDest ) final;
		bool CopyTexture3D( std::shared_ptr< RTexture3D >& inSource, std::shared_ptr< RTexture3D >& inDest ) final;

		bool CopyTexture1DRegion( std::shared_ptr< RTexture1D >& inSource, std::shared_ptr< RTexture1D >& inDest,
										  uint32 sourceX, uint32 inWidth, uint32 destX, uint8 sourceMip, uint8 targetMip ) final;
		bool CopyTexture2DRegion( std::shared_ptr< RTexture2D >& inSource, std::shared_ptr< RTexture2D >& inDest, uint32 sourceX, uint32 sourceY,
										  uint32 inWidth, uint32 inHeight, uint32 destX, uint32 destY, uint8 sourceMip, uint8 targetMip ) final;
		bool CopyTexture3DRegion( std::shared_ptr< RTexture3D >& inSource, std::shared_ptr< RTexture3D >& inDest, uint32 sourceX, uint32 sourceY, uint32 sourceZ,
										  uint32 inWidth, uint32 inHeight, uint32 inDepth, uint32 destX, uint32 destY, uint32 destZ, uint8 sourceMip, uint8 targetMip ) final;

		bool CopyTexture1DMip( std::shared_ptr< RTexture1D >& inSource, std::shared_ptr< RTexture1D >& inDest, uint8 sourceMip, uint8 destMip ) final;
		bool CopyTexture2DMip( std::shared_ptr< RTexture2D >& inSource, std::shared_ptr< RTexture2D >& inDest, uint8 sourceMip, uint8 destMip ) final;
		bool CopyTexture3DMip( std::shared_ptr< RTexture3D >& inSource, std::shared_ptr< RTexture3D >& inDest, uint8 sourceMip, uint8 destMip ) final;


		std::shared_ptr< RRenderTarget > CreateRenderTarget( const std::shared_ptr< RTexture2D >& inSource ) final;
		void ClearRenderTarget( const std::shared_ptr< RRenderTarget >& inTarget, const Color4F& inColor ) final;
		std::shared_ptr< RDepthStencil > CreateDepthStencil( uint32 inWidth, uint32 inHeight ) final;
		bool ResizeDepthStencil( const std::shared_ptr< RDepthStencil >& inStencil, uint32 inWidth, uint32 inHeight ) final;
		void ClearDepthStencil( const std::shared_ptr< RDepthStencil >& inStencil, const Color4F& inColor ) final;

		std::shared_ptr< RViewClusters > CreateViewClusters() final;
		std::shared_ptr< RLightBuffer > CreateLightBuffer() final;

		// Rendering
		void SetNoRenderTargetAndClusterWriteAccess( const std::shared_ptr< RViewClusters >& inClusters ) final;
		void ClearClusterWriteAccess() final;
		void SetGBufferRenderTarget( const std::shared_ptr< GBuffer >& inGBuffer, const std::shared_ptr< RDepthStencil >& inStencil ) final;
		void DetachRenderTarget() final;

		void SetRenderTarget( const std::shared_ptr< RRenderTarget >& inTarget, const std::shared_ptr< RDepthStencil >& inStencil ) final;

		void RenderBatch( const std::shared_ptr< RBuffer >& inVertexBuffer, const std::shared_ptr< RBuffer >& inIndexBuffer, uint32 inIndexCount ) final;
		void RenderScreenQuad() final;
		void GetDebugFloorQuad( std::shared_ptr< RBuffer >& outVertexBuffer, std::shared_ptr< RBuffer >& outIndexBuffer ) final;

		// Matricies
		void CalculateViewMatrix( const ViewState& inView, Matrix& outViewMatrix ) final;
		void CalculateProjectionMatrix( const ScreenResolution& inResolution, float inFOV, Matrix& outProjMatrix ) final;
		void CalculateWorldMatrix( const Transform& inTransform, Matrix& outWorldMatrix ) final;
		void CalculateOrthoMatrix( const ScreenResolution& inResolution, Matrix& outOrthoMatrix ) final;
		void CalculateScreenViewMatrix( Matrix& outMatrix ) final;

		// Shader Creation
		std::shared_ptr< RVertexShader > CreateVertexShader( VertexShaderType inType ) final;
		std::shared_ptr< RGeometryShader > CreateGeometryShader( GeometryShaderType inType ) final;
		std::shared_ptr< RPixelShader > CreatePixelShader( PixelShaderType inType ) final;
		std::shared_ptr< RComputeShader > CreateComputeShader( ComputeShaderType inType ) final;

	};

}


#endif