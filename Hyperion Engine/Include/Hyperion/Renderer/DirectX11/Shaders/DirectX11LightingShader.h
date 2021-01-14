/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/Shaders/DirectX11LightingShader.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resource/Shader.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

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
		bool UploadLighting();

		void ClearGBufferResources();

		inline ID3D11VertexShader* GetVertexShader() const { return m_VertexShader.Get();; }
		inline ID3D11PixelShader* GetPixelShader() const { return m_PixelShader.Get(); }
		inline ID3D11InputLayout* GetInputLayout() const { return  m_InputLayout.Get(); }
	};

}