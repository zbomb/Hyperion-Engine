/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/Shaders/DirectX11ForwardShader.cpp
	© 2021, Zachary Berry
==================================================================================================*/


#include "Hyperion/Renderer/DirectX11/Shaders/DirectX11ForwardShader.h"



namespace Hyperion
{



	DirectX11ForwardShader::DirectX11ForwardShader( const String& inPixelShader, const String& inVertexShader )
	{
	}

	DirectX11ForwardShader::~DirectX11ForwardShader()
	{
		Shutdown();
	}

	void DirectX11ForwardShader::Shutdown()
	{
	}

	bool DirectX11ForwardShader::IsValid() const
	{
		return false;
	}

	bool DirectX11ForwardShader::Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
	{
		return false;
	}

	bool DirectX11ForwardShader::UploadMatrixData( const Matrix& inWorld, const Matrix& inView, const Matrix& inProjection )
	{
		return false;
	}

	bool DirectX11ForwardShader::UploadMaterial( const std::shared_ptr<RMaterial>& inMaterial )
	{
		return false;
	}

	bool DirectX11ForwardShader::UploadLighting()
	{
		return false;
	}

}