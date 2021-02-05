/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/Shaders/DX11SceneVertexShader.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/Shaders/DX11SceneVertexShader.h"
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/File/FileSystem.h"


namespace Hyperion
{

	DX11SceneVertexShader::DX11SceneVertexShader()
		: m_Context( nullptr )
	{

	}


	DX11SceneVertexShader::~DX11SceneVertexShader()
	{
		Shutdown();
	}


	bool DX11SceneVertexShader::Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
	{
		HYPERION_VERIFY( inDevice && inContext, "[DX11] Device and/or context was null!" );

		// Store context pointer
		m_Context = inContext;

		auto shaderFile		= FileSystem::OpenFile( FilePath( SHADER_PATH_DX11_VERTEX_SCENE, PathRoot::Game ), FileMode::Read );
		auto shaderSize		= shaderFile ? shaderFile->GetSize() : 0;

		if( shaderSize <= 0 )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load scene vertex shader, file wasnt found or was invalid" );
			return false;
		}

		std::vector< byte > shaderData {};
		{
			DataReader reader( shaderFile );
			reader.SeekBegin();

			if( reader.ReadBytes( shaderData, shaderSize ) != DataReader::ReadResult::Success )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to load scene vertex shader, the file couldnt be read" );
				return false;
			}
		}

		// Create the shader through DX11
		if( FAILED( inDevice->CreateVertexShader( shaderData.data(), shaderData.size(), NULL, m_VertexShader.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load scene vertex shader, the shader couldnt be created" );
			Shutdown();
			return false;
		}

		// Create supporting resources
		D3D11_BUFFER_DESC bufferDesc {};

		bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags			= 0;
		bufferDesc.ByteWidth			= sizeof( StaticMatrixBuffer );
		bufferDesc.StructureByteStride	= sizeof( StaticMatrixBuffer );

		if( FAILED( inDevice->CreateBuffer( &bufferDesc, NULL, m_StaticMatrixBuffer.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load scene vertex shader, couldnt create static matrix buffer" );
			Shutdown();
			return false;
		}

		bufferDesc.ByteWidth			= sizeof( ObjectMatrixBuffer );
		bufferDesc.StructureByteStride	= sizeof( ObjectMatrixBuffer );

		if( FAILED( inDevice->CreateBuffer( &bufferDesc, NULL, m_ObjectMatrixBuffer.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load scene vertex shader, couldnt create object matrix buffer" );
			Shutdown();
			return false;
		}

		// Create input layout
		D3D11_INPUT_ELEMENT_DESC inputDesc[ 3 ] {};

		inputDesc[ 0 ].SemanticName				= "POSITION";
		inputDesc[ 0 ].SemanticIndex			= 0;
		inputDesc[ 0 ].Format					= DXGI_FORMAT_R32G32B32_FLOAT;
		inputDesc[ 0 ].InputSlot				= 0;
		inputDesc[ 0 ].AlignedByteOffset		= 0;
		inputDesc[ 0 ].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
		inputDesc[ 0 ].InstanceDataStepRate		= 0;

		inputDesc[ 1 ].SemanticName				= "NORMAL";
		inputDesc[ 1 ].SemanticIndex			= 0;
		inputDesc[ 1 ].Format					= DXGI_FORMAT_R32G32B32_FLOAT;
		inputDesc[ 1 ].InputSlot				= 0;
		inputDesc[ 1 ].AlignedByteOffset		= D3D11_APPEND_ALIGNED_ELEMENT;
		inputDesc[ 1 ].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
		inputDesc[ 1 ].InstanceDataStepRate		= 0;

		inputDesc[ 2 ].SemanticName				= "TEXCOORD";
		inputDesc[ 2 ].SemanticIndex			= 0;
		inputDesc[ 2 ].Format					= DXGI_FORMAT_R32G32_FLOAT;
		inputDesc[ 2 ].InputSlot				= 0;
		inputDesc[ 2 ].AlignedByteOffset		= D3D11_APPEND_ALIGNED_ELEMENT;
		inputDesc[ 2 ].InputSlotClass			= D3D11_INPUT_PER_VERTEX_DATA;
		inputDesc[ 2 ].InstanceDataStepRate		= 0;

		if( FAILED( inDevice->CreateInputLayout( inputDesc, 3, shaderData.data(), shaderData.size(), m_InputLayout.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load scene vertex shader, couldnt create input layout" );
			Shutdown();
			return false;
		}
		
		return true;
	}


	void DX11SceneVertexShader::Shutdown()
	{
		m_Context.Reset();
		m_InputLayout.Reset();
		m_ObjectMatrixBuffer.Reset();
		m_StaticMatrixBuffer.Reset();
		m_VertexShader.Reset();
	}


	bool DX11SceneVertexShader::IsValid() const
	{
		return m_VertexShader && m_StaticMatrixBuffer && m_ObjectMatrixBuffer &&
			m_InputLayout && m_Context;
	}


	bool DX11SceneVertexShader::Attach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		if( !m_VertexShader )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to attach DX11 scene vertex shader, shader was null" );
			return false;
		}

		m_Context->VSSetShader( m_VertexShader.Get(), NULL, 0 );
		m_Context->IASetInputLayout( m_InputLayout.Get() );

		return true;
	}


	void DX11SceneVertexShader::Detach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		ID3D11Buffer* bufferList[] = { NULL, NULL };

		m_Context->VSSetConstantBuffers( 0, 2, bufferList );
		m_Context->VSSetShader( NULL, NULL, 0 );
	}


	bool DX11SceneVertexShader::UploadStaticParameters( Renderer& inRenderer, uint32 inFlags )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// Upload the view and projection matrix
		// Convert to DX11 matricies
		DirectX::XMMATRIX dx11ViewMatrix( inRenderer.GetViewMatrix().GetData() );
		DirectX::XMMATRIX dx11ProjMatrix( inRenderer.GetProjectionMatrix().GetData() );

		// Map our constant buffer
		D3D11_MAPPED_SUBRESOURCE bufferData {};

		if( FAILED( m_Context->Map( m_StaticMatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload parameters to the scene vertex shader, the static constant buffer couldnt be mapped" );
			return false;
		}

		auto* bufferPtr					= (StaticMatrixBuffer*) bufferData.pData;
		bufferPtr->ViewMatrix			= DirectX::XMMatrixTranspose( dx11ViewMatrix );
		bufferPtr->ProjectionMatrix		= DirectX::XMMatrixTranspose( dx11ProjMatrix );

		m_Context->Unmap( m_StaticMatrixBuffer.Get(), 0 );
		
		ID3D11Buffer* bufferList[] = { m_StaticMatrixBuffer.Get() };
		m_Context->VSSetConstantBuffers( 0, 1, bufferList );

		return true;
	}


	bool DX11SceneVertexShader::UploadPrimitiveParameters( const Matrix& inWorldMatrix, const RMaterial& inMaterial )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// Convert matrix, to dx11 matrix
		DirectX::XMMATRIX dx11WorldMatrix( inWorldMatrix.GetData() );

		// Map our object matrix buffer
		D3D11_MAPPED_SUBRESOURCE bufferData {};
		if( FAILED( m_Context->Map( m_ObjectMatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload primitive parameters to the scene vertex shader, the object constant buffer couldnt be mapped" );
			return false;
		}

		auto* bufferPtr			= (ObjectMatrixBuffer*) bufferData.pData;
		bufferPtr->WorldMatrix	= DirectX::XMMatrixTranspose( dx11WorldMatrix );

		m_Context->Unmap( m_ObjectMatrixBuffer.Get(), 0 );

		ID3D11Buffer* bufferList[] = { m_ObjectMatrixBuffer.Get() };
		m_Context->VSSetConstantBuffers( 1, 1, bufferList );

		return true;
	}

}