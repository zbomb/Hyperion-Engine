/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/Shaders/DX11SceneVertexShader.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RShader.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	class DX11SceneVertexShader : public RVertexShader
	{

	private:

		Microsoft::WRL::ComPtr< ID3D11VertexShader > m_VertexShader;
		Microsoft::WRL::ComPtr< ID3D11Buffer > m_StaticMatrixBuffer;
		Microsoft::WRL::ComPtr< ID3D11Buffer > m_ObjectMatrixBuffer;
		Microsoft::WRL::ComPtr< ID3D11InputLayout > m_InputLayout;
		Microsoft::WRL::ComPtr< ID3D11DeviceContext1 > m_Context;

		#pragma pack( push, 1 )
		struct StaticMatrixBuffer
		{
			DirectX::XMMATRIX ViewMatrix;
			DirectX::XMMATRIX ProjectionMatrix;
		};
		#pragma pack( pop )

		#pragma pack( push, 1 )
		struct ObjectMatrixBuffer
		{
			DirectX::XMMATRIX matrixList[ RENDERER_MAX_INSTANCES_PER_BATCH ];
		};
		#pragma pack( pop )

	public:

		DX11SceneVertexShader();
		~DX11SceneVertexShader();

		bool Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext );
		void Shutdown() final;
		bool IsValid() const final;

		bool Attach() final;
		void Detach() final;

		bool UploadStaticParameters( Renderer& inRenderer, uint32 inFlags ) final;
		bool UploadBatchTransforms( const std::vector< Matrix >& inMatricies ) final;

		inline bool UploadBatchMaterial( const RMaterial& inMaterial ) final	{ return true; }
		inline bool UploadLightBuffer( RLightBuffer& inBuffer ) final			{ return true; }
		inline bool UploadViewClusters( RViewClusters& inClusters ) final		{ return true; }
		inline bool UploadGBuffer( GBuffer& inGBuffer ) final					{ return true; }

		inline ID3D11VertexShader* GetDX11Shader() const { return m_VertexShader ? m_VertexShader.Get() : nullptr; }

	};

}