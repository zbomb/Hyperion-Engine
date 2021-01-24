/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/Shaders/DirectX11LightingShader.cpp
	© 2021, Zachary Berry
==================================================================================================*/


#include "Hyperion/Renderer/DirectX11/Shaders/DirectX11LightingShader.h"
#include "Hyperion/File/FileSystem.h"
#include "Hyperion/Library/Geometry.h"
#include "Hyperion/Renderer/GBuffer.h"
#include "Hyperion/Renderer/DirectX11/DirectX11Texture.h"
#include "Hyperion/Renderer/DirectX11/DirectX11ViewClusters.h"



namespace Hyperion
{



	DirectX11LightingShader::DirectX11LightingShader( const String& inPixelShader, const String& inVertexShader )
		: m_PixelShaderPath( inPixelShader ), m_VertexShaderPath( inVertexShader )
	{
	}


	DirectX11LightingShader::~DirectX11LightingShader()
	{
		Shutdown();
	}


	bool DirectX11LightingShader::Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
	{
		HYPERION_VERIFY( inDevice != nullptr && inContext != nullptr, "[DX11] Failed to create shader, device was nullptr" );
		m_Context = inContext;

		// First, lets get the data for both shaders
		if( m_PixelShaderPath.IsWhitespaceOrEmpty() || m_VertexShaderPath.IsWhitespaceOrEmpty() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create Lighting shader, path(s) were invalid" );
			return false;
		}

		auto pixelFile		= FileSystem::OpenFile( FilePath( m_PixelShaderPath, PathRoot::Game ), FileMode::Read );
		auto vertexFile		= FileSystem::OpenFile( FilePath( m_VertexShaderPath, PathRoot::Game ), FileMode::Read );

		if( !pixelFile || !vertexFile || !pixelFile->IsValid() || !vertexFile->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create Lighting shader, the shaders werent found on disk" );
			return false;
		}

		std::vector< byte > pixelShaderData;
		std::vector< byte > vertexShaderData;

		{
			DataReader reader( pixelFile );
			reader.SeekBegin();

			if( reader.ReadBytes( pixelShaderData, reader.Size() ) != DataReader::ReadResult::Success || pixelShaderData.size() == 0 )
			{
				Console::WriteLine( "[Warning] DX11: Failed to create Lighting shader, the pixel shader couldnt be read" );
				return false;
			}
		}
		{
			DataReader reader( vertexFile );

			if( reader.ReadBytes( vertexShaderData, reader.Size() ) != DataReader::ReadResult::Success || vertexShaderData.size() == 0 )
			{
				Console::WriteLine( "[Warning] DX11: Failed to create Lighting shader, the vertex shader couldnt be read" );
				return false;
			}
		}

		// Now, lets use DX11 to create the shaders
		if( FAILED( inDevice->CreateVertexShader( vertexShaderData.data(), vertexShaderData.size(), NULL, m_VertexShader.GetAddressOf() ) ) || m_VertexShader.Get() == NULL )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create Lighting shader, the vertex shader couldnt be created" );
			return false;
		}

		if( FAILED( inDevice->CreatePixelShader( pixelShaderData.data(), pixelShaderData.size(), NULL, m_PixelShader.GetAddressOf() ) ) || m_PixelShader.Get() == NULL )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create Lighting shader, the pixel shader couldnt be created" );
			Shutdown();

			return false;
		}

		// Next, create the input layout
		D3D11_INPUT_ELEMENT_DESC inputDesc[ 2 ];

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

		if( FAILED( inDevice->CreateInputLayout( inputDesc, 2, vertexShaderData.data(), vertexShaderData.size(), m_InputLayout.GetAddressOf() ) ) || m_InputLayout.Get() == NULL )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create Lighting shader, the input layout couldnt be created" );
			Shutdown();

			return false;
		}

		// Now, we need to setup samplers and buffers
		D3D11_BUFFER_DESC matrixBufferDesc;
		ZeroMemory( &matrixBufferDesc, sizeof( matrixBufferDesc ) );

		matrixBufferDesc.Usage					= D3D11_USAGE_DYNAMIC;
		matrixBufferDesc.ByteWidth				= sizeof( MatrixBuffer );
		matrixBufferDesc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDesc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
		matrixBufferDesc.MiscFlags				= 0;
		matrixBufferDesc.StructureByteStride	= 0;

		if( FAILED( inDevice->CreateBuffer( &matrixBufferDesc, NULL, m_MatrixBuffer.GetAddressOf() ) ) || m_MatrixBuffer.Get() == NULL )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create Lighting shader, the matrix buffer coudnt be created" );
			Shutdown();

			return false;
		}

		// Camera Buffer
		D3D11_BUFFER_DESC cameraBufferDesc;
		ZeroMemory( &cameraBufferDesc, sizeof( cameraBufferDesc ) );

		cameraBufferDesc.Usage					= D3D11_USAGE_DYNAMIC;
		cameraBufferDesc.ByteWidth				= sizeof( CameraBuffer );
		cameraBufferDesc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
		cameraBufferDesc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
		cameraBufferDesc.MiscFlags				= 0;
		cameraBufferDesc.StructureByteStride	= 0;

		if( FAILED( inDevice->CreateBuffer( &cameraBufferDesc, NULL, m_CameraBuffer.GetAddressOf() ) ) || m_CameraBuffer.Get() == NULL )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create lighting shader, the camera buffer couldnt be created" );
			Shutdown();

			return false;
		}

		// Light Buffer
		D3D11_BUFFER_DESC lightBufferDesc;
		ZeroMemory( &lightBufferDesc, sizeof( lightBufferDesc ) );

		lightBufferDesc.Usage					= D3D11_USAGE_DYNAMIC;
		lightBufferDesc.ByteWidth				= sizeof( LightBuffer );
		lightBufferDesc.BindFlags				= D3D11_BIND_CONSTANT_BUFFER;
		lightBufferDesc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
		lightBufferDesc.MiscFlags				= 0;
		lightBufferDesc.StructureByteStride		= 0;

		if( FAILED( inDevice->CreateBuffer( &lightBufferDesc, NULL, m_LightBuffer.GetAddressOf() ) ) || m_LightBuffer.Get() == NULL )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create lighting shader, the light buffer couldnt be created" );
			Shutdown();

			return false;
		}

		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory( &samplerDesc, sizeof( samplerDesc ) );

			// Create texture sampler description
		samplerDesc.Filter				= D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU			= D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV			= D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW			= D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias			= 0.0f;
		samplerDesc.MaxAnisotropy		= 1;
		samplerDesc.ComparisonFunc		= D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[ 0 ]	= 0;
		samplerDesc.BorderColor[ 1 ]	= 0;
		samplerDesc.BorderColor[ 2 ]	= 0;
		samplerDesc.BorderColor[ 3 ]	= 0;
		samplerDesc.MinLOD				= 0;
		samplerDesc.MaxLOD				= D3D11_FLOAT32_MAX;

		if( FAILED( inDevice->CreateSamplerState( &samplerDesc, m_SamplerState.GetAddressOf() ) ) || m_SamplerState.Get() == NULL )
		{
			Console::WriteLine( "[Warning] DX11: Failed to create Lighting shader, the sampler couldnt be created" );
			Shutdown();

			return false;
		}

		Console::WriteLine( "DX11: Lighting shader initialized successfully!" );
		return true;
	}


	void DirectX11LightingShader::Shutdown()
	{
		m_CameraBuffer.Reset();
		m_LightBuffer.Reset();
		m_SamplerState.Reset();
		m_MatrixBuffer.Reset();
		m_InputLayout.Reset();
		m_PixelShader.Reset();
		m_VertexShader.Reset();
	}


	bool DirectX11LightingShader::IsValid() const
	{
		return m_PixelShader && m_VertexShader;
	}


	bool DirectX11LightingShader::UploadGBuffer( const std::shared_ptr<GBuffer>& inBuffer )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null!" );

		// Upload the G-Buffer to the pixel shader
		if( !inBuffer || !inBuffer->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to upload G-Buffer to pixel shader, was null!" );
			return false;
		}

		auto* diffuseTexture	= dynamic_cast< DirectX11Texture2D* >( inBuffer->GetDiffuseRoughnessTexture().get() );
		auto* normalTexture		= dynamic_cast< DirectX11Texture2D* >( inBuffer->GetNormalDepthTexture().get() );
		auto* specularTexture	= dynamic_cast< DirectX11Texture2D* >( inBuffer->GetSpecularTexture().get() );

		if( !diffuseTexture || !normalTexture || !specularTexture || !diffuseTexture->IsValid() || !normalTexture->IsValid() || !specularTexture->IsValid() )
		{
			Console::WriteLine( "[Warning] DX11: Failed to upload G-Buffer to pixel shader, textures were null" );
			return false;
		}

		ID3D11ShaderResourceView* resourceList[] = { diffuseTexture->GetView(), normalTexture->GetView(), specularTexture->GetView() };
		m_Context->PSSetShaderResources( 0, 3, resourceList );
	
		ID3D11SamplerState* samplerList[] = { m_SamplerState.Get() };
		m_Context->PSSetSamplers( 0, 1, samplerList );

		return true;
	}


	bool DirectX11LightingShader::UploadGBufferData( const Matrix& inView, const Matrix& inProjection )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// We need to extract camera position from view matrix
		DirectX::XMMATRIX gBufferView( inView.GetData() );
		DirectX::XMMATRIX gBufferProj( inProjection.GetData() );

		DirectX::XMVECTOR cameraPos, cameraRot, _scale;
		if( !DirectX::XMMatrixDecompose( &_scale, &cameraRot, &cameraPos, gBufferView ) )
		{
			Console::WriteLine( "[Warning] DX11: Failed to decompose view matrix" );
			return false;
		}

		// Now, we want to build a matrix we can use to project pixels back into 3d space to help calculate lighting
		auto gbufferTransform	= DirectX::XMMatrixMultiply( DirectX::XMMatrixInverse( nullptr, gBufferProj ), DirectX::XMMatrixInverse( nullptr, gBufferView ) );

		// In the shader, we need the X and Y values to range from [-1,1]
		// Where -1 is screen min, and 1 is screen max in that particular dimension
		// Then, depth should range between [0,1] where 0 is SCREEN_NEAR and 1 is SCREEN_FAR
		// With the X values.... pixel 0 maps to 0
		// With Y values.... pixel 0 maps to 1
		D3D11_MAPPED_SUBRESOURCE resource;
	
		if( FAILED( m_Context->Map( m_CameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ) )
		{
			Console::WriteLine( "[Warning] DX11: Failed to map camera buffer" );
			return false;
		}

		auto* buffer = (CameraBuffer*) resource.pData;

		buffer->CameraPosition			= DirectX::XMFLOAT3( cameraPos.m128_f32[ 0 ], cameraPos.m128_f32[ 1 ], cameraPos.m128_f32[ 3 ] );
		buffer->InverseViewProjMatrix	= DirectX::XMMatrixTranspose( gbufferTransform );

		m_Context->Unmap( m_CameraBuffer.Get(), 0 );

		ID3D11Buffer* bufferList[] = { m_CameraBuffer.Get() };
		m_Context->PSSetConstantBuffers( 0, 1, bufferList );

		return true;
	}


	bool DirectX11LightingShader::UploadMatrixData( const Matrix& inWorld, const Matrix& inView, const Matrix& inProjection )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// Prepare the matricies to be uploaded to the graphics card
		DirectX::XMMATRIX world( inWorld.GetData() );
		DirectX::XMMATRIX view( inView.GetData() );
		DirectX::XMMATRIX projection( inProjection.GetData() );

		world		= DirectX::XMMatrixTranspose( world );
		view		= DirectX::XMMatrixTranspose( view );
		projection	= DirectX::XMMatrixTranspose( projection );

		// Now we need to map the buffer and set the matricies
		D3D11_MAPPED_SUBRESOURCE mappedResource{};
		auto res = m_Context->Map( m_MatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
		if( FAILED( res ) )
		{
			Console::WriteLine( "[Warning] DX11: Failed to map matrix buffer!" );
			return false;
		}

		auto* bufferPtr			= (MatrixBuffer*) mappedResource.pData;
		bufferPtr->World		= world;
		bufferPtr->View			= view;
		bufferPtr->Projection	= projection;

		m_Context->Unmap( m_MatrixBuffer.Get(), 0 );

		// Now, set the buffer in the vertex shader
		ID3D11Buffer* bufferList[] = { m_MatrixBuffer.Get() };
		m_Context->VSSetConstantBuffers( 0, 1, bufferList );

		return true;
	}


	void DirectX11LightingShader::ClearResources()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		ID3D11ShaderResourceView* resources[] = { NULL, NULL, NULL, NULL };
		m_Context->PSSetShaderResources( 0, 4, resources );
	}


	bool DirectX11LightingShader::UploadLighting( const Color3F& inAmbientColor, float inAmbientIntensity, const std::vector< std::shared_ptr< ProxyLight > >& inLights )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null!" );

		D3D11_MAPPED_SUBRESOURCE mappedResource{};
		if( FAILED( m_Context->Map( m_LightBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource ) ) )
		{
			Console::WriteLine( "[Warning] DX11: Failed to map light buffer" );
			return false;
		}

		auto* bufferPtr = (LightBuffer*) mappedResource.pData;

		bufferPtr->AmbientColor			= DirectX::XMFLOAT3( inAmbientColor.r, inAmbientColor.g, inAmbientColor.b );
		bufferPtr->AmbientIntensity		= inAmbientIntensity;
		bufferPtr->LightCount			= inLights.size();
	
		for( int i = 0; i < inLights.size(); i++ )
		{
			auto& in_light		= inLights.at( i );
			auto& out_light		= bufferPtr->lights[ i ];

			if( !in_light )
			{
				out_light.Color			= DirectX::XMFLOAT3( 0.f, 0.f, 0.f );
				out_light.Brightness	= 0.f;
				out_light.Position		= DirectX::XMFLOAT3( 0.f, 0.f, 0.f );
				out_light.Radius		= 0.001f;
			}
			else
			{
				auto in_color	= in_light->GetColor();
				auto in_pos		= in_light->GetTransform().Position;

				out_light.Color			= DirectX::XMFLOAT3( in_color.r, in_color.g, in_color.b );
				out_light.Brightness	= in_light->GetBrightness();
				out_light.Position		= DirectX::XMFLOAT3( in_pos.X, in_pos.Y, in_pos.Z );
				out_light.Radius		= in_light->GetRadius();
			}
		}

		m_Context->Unmap( m_LightBuffer.Get(), 0 );

		ID3D11Buffer* bufferList[] = { m_LightBuffer.Get() };
		m_Context->PSSetConstantBuffers( 1, 1, bufferList );

		return true;
	}


	bool DirectX11LightingShader::UploadClusterData( const std::shared_ptr< RViewClusters >& inClusters )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null!" );

		DirectX11ViewClusters* clusterInfo = dynamic_cast< DirectX11ViewClusters* >( inClusters.get() );
		ID3D11ShaderResourceView* srv = clusterInfo ? clusterInfo->GetClusterInfoSRV() : nullptr;

		if( !srv )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload cluster info to lighting pixel shader, resource view was null" );
			return false;
		}

		ID3D11ShaderResourceView* srvList[] = { srv };

		m_Context->PSSetShaderResources( 3, 1, srvList );
		return true;
	}

}