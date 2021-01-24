/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/Shaders/DirectX11GBufferShader.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resource/Shader.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"
#include "Hyperion/Renderer/DirectX11/DirectX11ViewClusters.h"


namespace Hyperion
{

	class DirectX11GBufferShader : public RGBufferShader
	{

		template< typename _Ty >
		using ComPtr = Microsoft::WRL::ComPtr< _Ty >;

	private:

		ComPtr< ID3D11VertexShader > m_VertexShader;
		ComPtr< ID3D11PixelShader > m_PixelShader;
		ComPtr< ID3D11InputLayout > m_InputLayout;
		ComPtr< ID3D11Buffer > m_MatrixBuffer;
		ComPtr< ID3D11SamplerState > m_SamplerState;
		ComPtr< ID3D11Buffer > m_ClusterInfoBuffer;

		ID3D11DeviceContext* m_Context;

		String m_PixelShaderPath;
		String m_VertexShaderPath;

		#pragma pack( push, 1 )
		struct ClusterInfoBuffer
		{
			float depthCalcTermA;
			float depthCalcTermB;
			float clusterSizeX;
			float clusterSizeY;
			uint32 clusterCountX;
			uint32 clusterCountY;
			float _pad_1;
			float _pad_2;
		};
		#pragma pack( pop )

	public:

		DirectX11GBufferShader( const String& inPixelShader, const String& inVertexShader );
		~DirectX11GBufferShader();

		bool Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext );
		void Shutdown();
		bool IsValid() const;

		bool UploadMaterial( const std::shared_ptr< RMaterial >& inMaterial );
		bool UploadMatrixData( const Matrix& inWorldMatrix, const Matrix& inViewMatrix, const Matrix& inProjectionMatrix );
		bool UploadClusterData( const std::shared_ptr< RViewClusters >& inClusters );

		inline ID3D11VertexShader* GetVertexShader() const { return m_VertexShader.Get(); }
		inline ID3D11PixelShader* GetPixelShader() const { return m_PixelShader.Get(); }
		inline ID3D11InputLayout* GetInputLayout() const { return m_InputLayout.Get(); }
	};

}