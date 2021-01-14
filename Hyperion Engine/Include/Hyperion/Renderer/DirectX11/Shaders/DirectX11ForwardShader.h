/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/DirectX11/Shaders/DirectX11ForwardShader.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resource/Shader.h"
#include "Hyperion/Renderer/DirectX11/DirectX11.h"


namespace Hyperion
{

	class DirectX11ForwardShader : public RForwardShader
	{

	public:

		DirectX11ForwardShader( const String& inPixelShader, const String& inVertexShader );
		~DirectX11ForwardShader();

		void Shutdown();
		bool IsValid() const;

		bool Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext );
		bool UploadMaterial( const std::shared_ptr< RMaterial >& inMaterial );
		bool UploadMatrixData( const Matrix& inWorldMatrix, const Matrix& inViewMartix, const Matrix& inProjectionMatrix );
		bool UploadLighting();

		inline ID3D11VertexShader* GetVertexShader() const { return nullptr; }
		inline ID3D11PixelShader* GetPixelShader() const { return nullptr; }
		inline ID3D11InputLayout* GetInputLayout() const { return nullptr; }

	};

}