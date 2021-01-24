/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/Shaders/DirectX11LightingShader.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resource/Shader.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"
#include "Hyperion/Library/Color.h"


namespace Hyperion
{
	#pragma pack( push, 1 )
	struct LightInfo
	{
		DirectX::XMFLOAT3 Position;
		float Radius;
		// ------ 16 ------
		DirectX::XMFLOAT3 Color;
		float Brightness;
		// ----- 32 -------
		DirectX::XMFLOAT4 _pad1;
		// ------ 48 ---------
	};
	#pragma pack( pop )

	#pragma pack( push, 1 )
	struct CameraBuffer
	{
		DirectX::XMFLOAT3 CameraPosition;
		float _pad4;
		float _pad5;
		float _pad1;
		float _pad2;
		float _pad3;
		// ------------ 32 bytes -------------
		DirectX::XMMATRIX InverseViewProjMatrix;
		// ------ 96 bytes -----------
	};
	#pragma pack( pop )

	#pragma pack( push, 1 )
	struct LightBuffer
	{
		DirectX::XMFLOAT3 AmbientColor;
		float AmbientIntensity;
		int LightCount;
		DirectX::XMFLOAT3 _pad;
		LightInfo lights[ 50 ];
	};
	#pragma pack( pop )

	class DirectX11LightingShader : public RLightingShader
	{

		template< typename _Ty >
		using ComPtr = Microsoft::WRL::ComPtr< _Ty >;

	private:

		ComPtr< ID3D11VertexShader > m_VertexShader;
		ComPtr< ID3D11PixelShader > m_PixelShader;
		ComPtr< ID3D11InputLayout > m_InputLayout;
		ComPtr< ID3D11Buffer > m_MatrixBuffer;
		ComPtr< ID3D11SamplerState > m_SamplerState;
		ComPtr< ID3D11Buffer > m_CameraBuffer;
		ComPtr< ID3D11Buffer > m_LightBuffer;

		ID3D11DeviceContext* m_Context;

		String m_PixelShaderPath;
		String m_VertexShaderPath;

	public:

		DirectX11LightingShader( const String& inPixelShader, const String& inVertexShader );
		~DirectX11LightingShader();

		bool Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext );
		void Shutdown();
		bool IsValid() const;

		bool UploadGBuffer( const std::shared_ptr< GBuffer >& inBuffer );
		bool UploadMatrixData( const Matrix& inWorldMatriix, const Matrix& inViewMatrix, const Matrix& inProjectionMatrix );
		bool UploadGBufferData( const Matrix& inView, const Matrix& inProjection );
		bool UploadLighting( const Color3F& inAmbientColor, float inAmbientIntensity, const std::vector< std::shared_ptr< ProxyLight > >& inLights );
		bool UploadClusterData( const std::shared_ptr< RViewClusters >& inClusters );

		void ClearResources();

		inline ID3D11VertexShader* GetVertexShader() const { return m_VertexShader.Get();; }
		inline ID3D11PixelShader* GetPixelShader() const { return m_PixelShader.Get(); }
		inline ID3D11InputLayout* GetInputLayout() const { return  m_InputLayout.Get(); }
	};

}