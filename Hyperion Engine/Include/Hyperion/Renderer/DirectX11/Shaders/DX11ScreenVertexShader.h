/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/Shaders/DX11ScreenVertexShader.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RShader.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	class DX11ScreenVertexShader : public RVertexShader
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11VertexShader > m_VertexShader;
		Microsoft::WRL::ComPtr< ID3D11Buffer > m_MatrixBuffer;
		Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_Context;
		Microsoft::WRL::ComPtr< ID3D11InputLayout > m_InputLayout;

		#pragma pack( push, 1 )
		struct MatrixBuffer
		{
			DirectX::XMMATRIX worldViewProjMatrix;
		};
		#pragma pack( pop )

	public:

		DX11ScreenVertexShader();
		~DX11ScreenVertexShader();

		bool Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext );
		void Shutdown() final;
		bool IsValid() const final;

		bool Attach() final;
		void Detach() final;

		bool UploadStaticParameters( Renderer& inRenderer, uint32 inFlags ) final;

		// Parameter uploads that we dont need	
		inline bool UploadBatchTransforms( const std::vector< Matrix >& inMatricies ) final		{ return true; }
		inline bool UploadBatchMaterial( const RMaterial& inMaterial ) final					{ return true; }
		inline bool UploadLightBuffer( RLightBuffer& inBuffer ) final							{ return true; }
		inline bool UploadViewClusters( RViewClusters& inClusters ) final						{ return true; }
		inline bool UploadGBuffer( GBuffer& inGBuffer ) final									{ return true; }

		inline ID3D11VertexShader* GetDX11Shader() const { return m_VertexShader ? m_VertexShader.Get() : nullptr; }

	};

}