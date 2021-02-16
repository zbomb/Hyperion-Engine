/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/Shaders/DX11ForwardPreZShader.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/Shaders/DX11ForwardPreZShader.h"
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/File/FileSystem.h"


namespace Hyperion
{

	DX11ForwardPreZShader::DX11ForwardPreZShader()
	{

	}


	DX11ForwardPreZShader::~DX11ForwardPreZShader()
	{
		Shutdown();
	}


	bool DX11ForwardPreZShader::Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
	{
		HYPERION_VERIFY( inDevice && inContext, "[DX11] Device and/or context were null" );

		// Store context pointer
		m_Context = inContext;

		auto shaderFile = FileSystem::OpenFile( FilePath( SHADER_PATH_DX11_PIXEL_FORWARD_PRE_Z, PathRoot::Game ), FileMode::Read );
		auto shaderSize = shaderFile ? shaderFile->GetSize() : 0;

		if( shaderSize <= 0 )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load forward pre-z pixel shader, file wasnt found or was invalid" );
			return false;
		}

		std::vector< byte > shaderData {};
		{
			DataReader reader( shaderFile );
			reader.SeekBegin();

			if( reader.ReadBytes( shaderData, shaderSize ) != DataReader::ReadResult::Success )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to load forward pre-z pixel shader, the file couldnt be read" );
				return false;
			}
		}

		// Create the shader through DX11
		if( FAILED( inDevice->CreatePixelShader( shaderData.data(), shaderData.size(), NULL, m_PixelShader.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load forward pre-z pixel shader, the shader couldnt be created" );
			Shutdown();
			return false;
		}

		D3D11_BUFFER_DESC bufferDesc {};

		bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags			= 0;
		bufferDesc.ByteWidth			= sizeof( ConstantBufferType );
		bufferDesc.StructureByteStride	= sizeof( ConstantBufferType );

		if( FAILED( inDevice->CreateBuffer( &bufferDesc, NULL, m_ConstantBuffer.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load forward pre-z pixel shader, the buffer couldnt be created" );
			Shutdown();
			return false;
		}

		return true;
	}


	void DX11ForwardPreZShader::Shutdown()
	{
		m_ConstantBuffer.Reset();
		m_PixelShader.Reset();
		m_Context.Reset();
	}


	bool DX11ForwardPreZShader::IsValid() const
	{
		return m_PixelShader && m_ConstantBuffer;
	}


	bool DX11ForwardPreZShader::UploadStaticParameters( Renderer& inRenderer, uint32 inFlags )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// Get the parameters we need
		auto screenRes = inRenderer.GetResolutionUnsafe();

		// Map the constant buffer and write out the parameters
		D3D11_MAPPED_SUBRESOURCE resource {};

		if( FAILED( m_Context->Map( m_ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload static parameters to pre-z pixel shader, mapping failed" );
			return false;
		}

		auto* bufferPtr = (ConstantBufferType*) resource.pData;

		bufferPtr->ScreenWidth		= (float) screenRes.Width;
		bufferPtr->ScreenHeight		= (float) screenRes.Height;
		bufferPtr->DepthSliceA		= (float) RENDERER_CLUSTER_COUNT_Z / logf( SCREEN_FAR / SCREEN_NEAR );
		bufferPtr->DepthSliceB		= bufferPtr->DepthSliceA * logf( SCREEN_NEAR );

		m_Context->Unmap( m_ConstantBuffer.Get(), 0 );

		return true;
	}


	bool DX11ForwardPreZShader::Attach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		m_Context->PSSetShader( m_PixelShader.Get(), NULL, 0 );

		ID3D11Buffer* bufferList[] = { m_ConstantBuffer.Get() };
		m_Context->PSSetConstantBuffers( 0, 1, bufferList );

		return true;
	}


	void DX11ForwardPreZShader::Detach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		m_Context->PSSetShader( NULL, NULL, 0 );
		
		ID3D11Buffer* bufferList[] = { NULL };
		m_Context->PSSetConstantBuffers( 0, 1, bufferList );
	}

}