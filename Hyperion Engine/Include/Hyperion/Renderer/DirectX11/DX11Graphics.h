/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/DX11Graphics.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"

#ifdef HYPERION_SUPPORT_DIRECTX

#include "Hyperion/Renderer/IGraphics.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"

namespace Hyperion
{
	


	class DX11Graphics : public IGraphics
	{

		template< typename _Ty >
		using ComPtr = Microsoft::WRL::ComPtr< _Ty >;

	private:

		/*
		*	DirectX11 Resources
		*/
		ComPtr< ID3D11Device2 > m_Device;
		ComPtr< ID3D11DeviceContext2 > m_Context;
		ComPtr< IDXGISwapChain2 > m_SwapChain;
		ComPtr< ID3D11Debug > m_Debug;
		ComPtr< ID3D11RenderTargetView > m_BackBufferRenderTarget;
		ComPtr< ID3D11DepthStencilState > m_DepthDisabledState;
		ComPtr< ID3D11DepthStencilState > m_DepthEnabledState;
		ComPtr< ID3D11Texture2D > m_DepthBuffer;
		ComPtr< ID3D11DepthStencilView > m_DepthStencilView;
		ComPtr< ID3D11RasterizerState1 > m_RasterState;
		ComPtr< ID3D11BlendState > m_BlendEnabledState;
		ComPtr< ID3D11BlendState > m_BlendDisabledState;
		ComPtr< ID3D11Buffer > m_ScreenQuadVertexBuffer;

		/*
		*	Generic Resources
		*/
		std::shared_ptr< RTexture2D > m_BackBuffer;
		std::shared_ptr< RBuffer > m_AttachedIndexBuffer;
		std::shared_ptr< RBuffer > m_AttachedVertexBuffer;
		std::shared_ptr< RBuffer > m_FloorVertexBuffer;
		std::shared_ptr< RBuffer > m_FloorIndexBuffer;
		std::vector< Matrix > m_FloorMatricies;

		/*
		*	Other Data Members
		*/
		HWND m_OSTarget;
		ScreenResolution m_Resolution;
		bool m_bVSync;
		std::vector< ScreenResolution > m_AvailableResolutions;
		uint32 m_DepthStencilWidth;
		uint32 m_DepthStencilHeight;

		String m_GraphicsDeviceName;
		uint32 m_DedicatedVideoMemory;
		uint32 m_SharedVideoMemory;
		DXGI_RATIONAL m_DisplayRefreshRate;
		bool m_bAsyncResourceCreation;

		AlphaBlendingState m_AlphaBlendState;
		DepthStencilState m_DepthState;

		std::vector< std::shared_ptr< RTexture2D > > m_AttachedRenderTargets;

		/*
		*	Helper Functions
		*/
		void ResizeWindow( HWND inWindow, uint32 inWidth, uint32 inHeight );
		void RebuildScreenQuad( uint32 inWidth, uint32 inHeight );
		void RebuildDebugFloorBuffers();

	public:

		/*
		*	Constructor/Destructor
		*/
		DX11Graphics();
		~DX11Graphics();

		/*
		*	Init and Shutdown
		*/
		bool Initialize( const InitializationParameters& inParameters ) final;
		void Shutdown() final;

		/*
		*	Resolution Managment
		*/
		bool UpdateResolution( const ScreenResolution& inResolution ) final;
		ScreenResolution GetResolution() final;
		void GetAvailableResolutions( std::vector< ScreenResolution >& outResolutions ) final;
		float GetDisplayRefreshRate() final;

		/*
		*	Depth Stencil Managment
		*/
		std::pair< uint32, uint32 > GetDepthStencilResolution() final;
		bool SetDepthStencilResolution( uint32 inWidth, uint32 inHeight ) final;
		bool ClearDepthStencil( float inDepth = 1.f, uint8 inStencil = 0 ) final;

		/*
		*	VSync Managment
		*/
		bool GetVSyncEnabled() final;
		bool SetVSyncEnabled( bool inEnabled ) final;

		/*
		*	Depth Stencil and Alpha Blending Managment
		*/
		void SetAlphaBlendingState( AlphaBlendingState inState ) final;
		AlphaBlendingState GetAlphaBlendingState() final;
		void SetDepthStencilState( DepthStencilState inState ) final;
		DepthStencilState GetDepthStencilState() final;

		/*
		*	Render Target Managment
		*/
		std::shared_ptr< RTexture2D > GetBackBuffer() final;
		bool ClearRenderTarget( const std::shared_ptr< RTexture2D >& inTarget, float inR = 0.f, float inG = 0.f, float inB = 0.f, float inA = 0.f ) final;
		bool AttachRenderTargets( const std::vector< std::shared_ptr< RTexture2D > >& inTargets, bool bUseDepthStencil = true ) final;
		bool AttachRenderTarget( const std::shared_ptr< RTexture2D >& inTarget, bool bUseDepthStencil = true ) final;
		void DetachRenderTargets() final;

		/*
		*	Debugging
		*/
		void GetFloorMesh( std::shared_ptr< RBuffer >& outVertexBuffer, std::shared_ptr< RBuffer >& outIndexBuffer, std::vector< Matrix >& outMatricies ) final;

		/*
		*	Rendering API
		*/
		void RenderScreenQuad() final;
		void AttachMesh( const std::shared_ptr< RBuffer >& inVertexBuffer, const std::shared_ptr< RBuffer >& inIndexBuffer, uint32 inIndexCount ) final;
		void Render( uint32 inInstanceCount ) final;

		void DisplayFrame() final;

		/*
		*	General Resource Managment
		*/
		bool IsAsyncResourceCreationAllowed() const final;

		/*
		*	Texture Managment
		*/
		std::shared_ptr< RTexture1D > CreateTexture1D( const TextureParameters& inParams ) final;
		std::shared_ptr< RTexture2D > CreateTexture2D( const TextureParameters& inParams ) final;
		std::shared_ptr< RTexture3D > CreateTexture3D( const TextureParameters& inParams ) final;

		std::shared_ptr< RTexture1D > CreateTexture1D() final;
		std::shared_ptr< RTexture2D > CreateTexture2D() final;
		std::shared_ptr< RTexture3D > CreateTexture3D() final;

		/*
		*	Buffer Managment
		*/
		std::shared_ptr< RBuffer > CreateBuffer( const BufferParameters& inParams ) final;
		std::shared_ptr< RBuffer > CreateBuffer( BufferType inType ) final;

		/*
		*	Resource Copying
		*/
		bool CopyResource( const std::shared_ptr< RGraphicsResource >& inSource, const std::shared_ptr< RGraphicsResource >& inTarget ) final;
		bool CopyResourceRange( const std::shared_ptr< RGraphicsResource >& inSource, const std::shared_ptr< RGraphicsResource >& inTarget, const ResourceRangeParameters& inParams ) final;

		/*
		*	Shader Creation
		*/
		std::shared_ptr< RVertexShader > CreateVertexShader( VertexShaderType inType ) final;
		std::shared_ptr< RGeometryShader > CreateGeometryShader( GeometryShaderType inType ) final;
		std::shared_ptr< RPixelShader > CreatePixelShader( PixelShaderType inType ) final;
		std::shared_ptr< RComputeShader > CreateComputeShader( ComputeShaderType inType ) final;
		std::shared_ptr< RPostProcessShader > CreatePostProcessShader( PostProcessShaderType inType ) final;


		////////////////////////////////// OLD ///////////////////////////////////


	private:

		/*
			Data Members
		
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
		std::vector< Matrix > m_FloorMatricies;

		// TESTING
		// Shadow Map System
		struct ShadowShaderStaticMatrixType
		{
			DirectX::XMMATRIX View;
			DirectX::XMMATRIX Projection;
		};

		struct ShadowShaderObjectMatrixType
		{
			DirectX::XMMATRIX World[ 512 ];
		};

		std::vector< ComPtr< ID3D11Texture2D > > m_ShadowMapTextures;
		ComPtr< ID3D11RenderTargetView > m_ShadowRenderTarget;
		//std::vector< std::vector< ComPtr< ID3D11RenderTargetView > > > m_ShadowRenderTargets;
		ComPtr< ID3D11Buffer > m_ShadowMapMemory;
		ComPtr< ID3D11VertexShader > m_ShadowVertexShader;
		ComPtr< ID3D11PixelShader > m_ShadowPixelShader;
		ComPtr< ID3D11Buffer > m_ShadowShaderStaticMatrix;
		ComPtr< ID3D11Buffer > m_ShadowShaderObjectMatrix;
		ComPtr< ID3D11InputLayout > m_ShadowInputLayout;

		/*
			Matricies
		
		DirectX::XMMATRIX m_WorldMatrix, m_ProjectionMatrix, m_OrthoMatrix, m_ScreenViewMatrix, m_ViewMatrix;

		/*
		*	Camera Info
		
		DirectX::XMFLOAT3 m_CameraPosition;
		DirectX::XMVECTOR m_CameraRotation;
		float m_FOV;

		/*
		*	Additional State
		
		uint32 m_AttachedGeometryAssetIdentifier;

		/*
			Helper Functions
		
		bool InitializeResources( HWND Target, ScreenResolution& Resolution );
		void ShutdownResources();
		void GenerateMatricies( const ScreenResolution& inRes, float inFOV, float inNear, float inFar );
		void GenerateScreenGeometry( uint32 inWidth, uint32 inHeight );

		void GenerateFloorGeometry();

		/*
		*	Debugging/Testing
		
		void PerformTiledResourceTests();

	public:

		DirectX11Graphics();
		~DirectX11Graphics();

		// Shadow Test
		void PerformShadowTest( BatchCollector& inBatches ) override;

		bool SetResolution( const ScreenResolution& inResolution ) override;
		void SetVSync( bool bVSync ) override;

		bool Initialize( void* pWindow ) override;
		void Shutdown() override;

		void BeginFrame() override;
		void EndFrame() override;

		void TransformAABB( const Transform& inTransform, const AABB& inBounds, OBB& outBounds ) override;

		void SetCameraInfo( const ViewState& inView ) override;
		bool CheckViewCull( ProxyPrimitive& inPrimitive ) override;
		bool CheckViewCull( ProxyLight& inLight ) override;

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
		void ClearRenderTarget( const std::shared_ptr< RRenderTarget >& inTarget, float inR = 0.f, float inG = 0.f, float inB = 0.f, float inA = 0.f ) final;
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

		void RenderGeometry( const std::shared_ptr< RBuffer >& inIndexBuffer, const std::shared_ptr< RBuffer >& inVertexBuffer, uint32 inIndexCount ) final;
		void UploadGeometry( const std::shared_ptr< RBuffer >& inIndexBuffer, const std::shared_ptr< RBuffer >& inVertexBuffer ) final;
		void RenderBatch( uint32 inInstanceCount, uint32 inIndexCount ) final;
		void RenderScreenQuad() final;
		void GetDebugFloorQuad( std::shared_ptr< RBuffer >& outVertexBuffer, std::shared_ptr< RBuffer >& outIndexBuffer, std::vector< Matrix >& outMatricies ) final;

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
		std::shared_ptr< RPostProcessShader > CreatePostProcessShader( PostProcessShaderType inType ) final;
		*/
	};

}


#endif