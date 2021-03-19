/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/Shaders/DX11ScreenVertexShader.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/Shaders/DX11ScreenVertexShader.h"
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/File/FileSystem.h"


namespace Hyperion
{

	DX11ScreenVertexShader::DX11ScreenVertexShader()
	{

	}


	DX11ScreenVertexShader::~DX11ScreenVertexShader()
	{
		Shutdown();
	}


	bool DX11ScreenVertexShader::Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
	{
		HYPERION_VERIFY( inDevice && inContext, "[DX11] Device and/or context were null!" );

		// Store context pointer
		m_Context = inContext;

		auto shaderFile = FileSystem::OpenFile( FilePath( SHADER_PATH_DX11_VERTEX_SCREEN, PathRoot::Game ), FileMode::Read );
		auto shaderSize = shaderFile ? shaderFile->GetSize() : 0;

		if( shaderSize <= 0 )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load screen vertex shader, file wasnt found or was invalid" );
			return false;
		}

		std::vector< byte > shaderData {};
		{
			DataReader reader( shaderFile );
			reader.SeekBegin();

			if( reader.ReadBytes( shaderData, shaderSize ) != DataReader::ReadResult::Success )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to load screen vertex shader, the file couldnt be read" );
				return false;
			}
		}

		// Create the shader through DX11
		if( FAILED( inDevice->CreateVertexShader( shaderData.data(), shaderData.size(), NULL, m_VertexShader.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load screen vertex shader, the shader couldnt be created" );
			Shutdown();
			return false;
		}

		// Create our input layout
		D3D11_INPUT_ELEMENT_DESC inputDesc[ 2 ] {};

		inputDesc[ 0 ].SemanticName				= "POSITION";
		inputDesc[ 0 ].SemanticIndex			= 0;
		inputDesc[ 0 ].Format					= DXGI_FORMAT_R32G32B32_FLOAT;
		inputDesc[ 0 ].InputSlot				= 0;
		inputDesc[ 0 ].AlignedByteOffset		= 0;
		inputDesc[ 0 ].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
		inputDesc[ 0 ].InstanceDataStepRate		= 0;

		inputDesc[ 1 ].SemanticName				= "TEXCOORD";
		inputDesc[ 1 ].SemanticIndex			= 0;
		inputDesc[ 1 ].Format					= DXGI_FORMAT_R32G32_FLOAT;
		inputDesc[ 1 ].InputSlot				= 0;
		inputDesc[ 1 ].AlignedByteOffset		= D3D11_APPEND_ALIGNED_ELEMENT;
		inputDesc[ 1 ].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
		inputDesc[ 1 ].InstanceDataStepRate		= 0;

		if( FAILED( inDevice->CreateInputLayout( inputDesc, 2, shaderData.data(), shaderData.size(), m_InputLayout.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: failed to load screen vertex shader, input layout couldnt be created" );
			Shutdown();
			return false;
		}

		// Create our constant buffer
		D3D11_BUFFER_DESC bufferDesc {};

		bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags			= 0;
		bufferDesc.ByteWidth			= sizeof( MatrixBuffer );
		bufferDesc.StructureByteStride	= sizeof( MatrixBuffer );

		if( FAILED( inDevice->CreateBuffer( &bufferDesc, NULL, m_MatrixBuffer.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load screen vertex shader, matrix buffer couldnt be created" );
			Shutdown();
			return false;
		}

		return true;
	}


	void DX11ScreenVertexShader::Shutdown()
	{
		m_MatrixBuffer.Reset();
		m_InputLayout.Reset();
		m_VertexShader.Reset();
		m_Context.Reset();
	}


	bool DX11ScreenVertexShader::IsValid() const
	{
		return m_VertexShader && m_InputLayout && m_MatrixBuffer;
	}


	bool DX11ScreenVertexShader::Attach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		if( !m_VertexShader || !m_InputLayout )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to attach screen vertex shader, resources invalid?" );
			return false;
		}

		m_Context->VSSetShader( m_VertexShader.Get(), NULL, 0 );
		m_Context->IASetInputLayout( m_InputLayout.Get() );

		ID3D11Buffer* bufferList[] = { m_MatrixBuffer.Get() };
		m_Context->VSSetConstantBuffers( 0, 1, bufferList );

		return true;
	}


	void DX11ScreenVertexShader::Detach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		ID3D11Buffer* bufferList[] = { NULL };
		m_Context->VSSetConstantBuffers( 0, 1, bufferList );
		m_Context->VSSetShader( NULL, NULL, 0 );
	}


	bool DX11ScreenVertexShader::UploadStaticParameters( Renderer& inRenderer, uint32 inFlags )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// Calculate our matrix for the screen vertex buffer
		RMatrix viewMatrix = inRenderer.GetScreenViewMatrix();
		RMatrix projMatrix = inRenderer.GetOrthoMatrix();

		DirectX::XMMATRIX viewProjMatrix = DirectX::XMMatrixMultiply( DirectX::XMMATRIX( viewMatrix.m ), DirectX::XMMATRIX( projMatrix.m ) );
		viewProjMatrix = DirectX::XMMatrixTranspose( viewProjMatrix );

		D3D11_MAPPED_SUBRESOURCE resource {};

		if( FAILED( m_Context->Map( m_MatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload static parameters to screen vertex shader, couldnt map matrix buffer" );
			return false;
		}

		auto* bufferPtr					= (MatrixBuffer*) resource.pData;
		bufferPtr->worldViewProjMatrix	= viewProjMatrix;

		m_Context->Unmap( m_MatrixBuffer.Get(), 0 );

		return true;
	}

}