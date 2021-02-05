/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/Shaders/DX11ForwardPixelShader.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/Shaders/DX11ForwardPixelShader.h"
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/Resources/RMaterial.h"
#include "Hyperion/Renderer/DirectX11/DirectX11Texture.h"
#include "Hyperion/File/FileSystem.h"
#include "Hyperion/Renderer/DirectX11/DirectX11LightBuffer.h"
#include "Hyperion/Renderer/DirectX11/DirectX11ViewClusters.h"
#include "Hyperion/Renderer/GBuffer.h"


namespace Hyperion
{



	DX11ForwardPixelShader::DX11ForwardPixelShader()
	{

	}


	DX11ForwardPixelShader::~DX11ForwardPixelShader()
	{
		Shutdown();
	}


	bool DX11ForwardPixelShader::Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
	{
		HYPERION_VERIFY( inDevice && inContext, "[DX11] Device and/or context were null!" );

		// Store context pointer
		m_Context = inContext;

		auto shaderFile = FileSystem::OpenFile( FilePath( SHADER_PATH_DX11_PIXEL_FORWARD, PathRoot::Game ), FileMode::Read );
		auto shaderSize = shaderFile ? shaderFile->GetSize() : 0;

		if( shaderSize <= 0 )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load lighting pixel shader, file wasnt found or was invalid" );
			return false;
		}

		std::vector< byte > shaderData {};
		{
			DataReader reader( shaderFile );
			reader.SeekBegin();

			if( reader.ReadBytes( shaderData, shaderSize ) != DataReader::ReadResult::Success )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to load lighting pixel shader, the file couldnt be read" );
				return false;
			}
		}

		// Create the shader through DX11
		if( FAILED( inDevice->CreatePixelShader( shaderData.data(), shaderData.size(), NULL, m_PixelShader.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load lighting pixel shader, the shader couldnt be created" );
			Shutdown();
			return false;
		}

		// Create the sampler state
		D3D11_SAMPLER_DESC samplerDesc {};

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[ 0 ] = 0;
		samplerDesc.BorderColor[ 1 ] = 0;
		samplerDesc.BorderColor[ 2 ] = 0;
		samplerDesc.BorderColor[ 3 ] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		if( FAILED( inDevice->CreateSamplerState( &samplerDesc, m_Sampler.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load lighting pixel shader, couldnt create sampler state" );
			Shutdown();
			return false;
		}

		// Create constant buffer
		D3D11_BUFFER_DESC bufferDesc {};

		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = sizeof( RenderInfoBuffer );
		bufferDesc.ByteWidth = sizeof( RenderInfoBuffer );

		if( FAILED( inDevice->CreateBuffer( &bufferDesc, NULL, m_RenderInfoBuffer.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load lighting pixel shader, couldnt create constant buffer" );
			Shutdown();
			return false;
		}

		return true;
	}


	void DX11ForwardPixelShader::Shutdown()
	{
		m_Sampler.Reset();
		m_RenderInfoBuffer.Reset();
		m_PixelShader.Reset();
		m_Context.Reset();
	}


	bool DX11ForwardPixelShader::IsValid() const
	{
		return m_PixelShader && m_RenderInfoBuffer && m_Sampler;
	}


	bool DX11ForwardPixelShader::UploadStaticParameters( Renderer& inRenderer, uint32 inFlags )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null!" );

		// Calculate the inverse view matrix
		DirectX::XMMATRIX invViewMatrix( inRenderer.GetViewMatrix().GetData() );
		invViewMatrix = DirectX::XMMatrixTranspose( DirectX::XMMatrixInverse( nullptr, invViewMatrix ) );

		// Get some other values..
		ScreenResolution screenRes = inRenderer.GetResolutionUnsafe();
		ViewState viewState = inRenderer.GetViewState();
		Matrix projectionMatrix = inRenderer.GetProjectionMatrix();
		Color3F ambientLightColor = inRenderer.GetAmbientLightColor();

		D3D11_MAPPED_SUBRESOURCE resource {};

		if( FAILED( m_Context->Map( m_RenderInfoBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload static parameters to lighting pixel shader, the constant buffer couldnt be mapped" );
			return false;
		}

		auto* bufferPtr = (RenderInfoBuffer*) resource.pData;

		bufferPtr->CameraPosition = DirectX::XMFLOAT3( viewState.Position.X, viewState.Position.Y, viewState.Position.Z );
		bufferPtr->_pad_vb_1 = 0.f;
		bufferPtr->ProjectionTermA = projectionMatrix[ 0 ][ 0 ];
		bufferPtr->ProjectionTermB = projectionMatrix[ 1 ][ 1 ];
		bufferPtr->ScreenNear = SCREEN_NEAR;
		bufferPtr->ScreenFar = SCREEN_FAR;
		bufferPtr->ScreenWidth = (float) screenRes.Width;
		bufferPtr->ScreenHeight = (float) screenRes.Height;
		bufferPtr->DepthSliceTermA = (float) RENDERER_CLUSTER_COUNT_Z / logf( SCREEN_FAR / SCREEN_NEAR );
		bufferPtr->DepthSliceTermB = bufferPtr->DepthSliceTermA * logf( SCREEN_NEAR );
		bufferPtr->AmbientLightColor = DirectX::XMFLOAT3( ambientLightColor.r, ambientLightColor.g, ambientLightColor.b );
		bufferPtr->AmbientLightIntensity = inRenderer.GetAmbientLightIntensity();
		bufferPtr->InverseViewMatrix = invViewMatrix;

		m_Context->Unmap( m_RenderInfoBuffer.Get(), 0 );

		ID3D11Buffer* bufferList[] = { m_RenderInfoBuffer.Get() };
		m_Context->PSSetConstantBuffers( 0, 1, bufferList );

		return true;
	}


	bool DX11ForwardPixelShader::UploadLightBuffer( RLightBuffer& inBuffer )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		DirectX11LightBuffer& dx11Buffer = dynamic_cast<DirectX11LightBuffer&>( inBuffer );

		ID3D11ShaderResourceView* resourceList[] = { dx11Buffer.GetView() };
		m_Context->PSSetShaderResources( 4, 1, resourceList );

		return true;
	}


	bool DX11ForwardPixelShader::UploadViewClusters( RViewClusters& inClusters )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		DirectX11ViewClusters& dx11Clusters = dynamic_cast<DirectX11ViewClusters&>( inClusters );

		ID3D11ShaderResourceView* resourceList[] = { dx11Clusters.GetClusterInfoSRV(), dx11Clusters.GetClusterLightSRV(), dx11Clusters.GetLightIdentifierSRV() };
		m_Context->PSSetShaderResources( 1, 3, resourceList );

		return true;
	}


	bool DX11ForwardPixelShader::UploadPrimitiveParameters( const Matrix& inWorldMatrix, const RMaterial& inMaterial )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		// Get the shader resource view for the base map texture
		auto dx11Texture = std::dynamic_pointer_cast<DirectX11Texture2D>( inMaterial.GetTexture( "base_map" ) );
		if( !dx11Texture || !dx11Texture->IsValid() )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload primitive material, the material didnt have a base_map!" );
			return false;
		}

		ID3D11ShaderResourceView* textureView = dx11Texture->GetView();
		HYPERION_VERIFY( textureView != nullptr, "[DX11] Texture is valid, but resource view was null?" );

		// Set texture resource in shader to our base map texture
		ID3D11ShaderResourceView* viewList[] = { textureView };
		m_Context->PSSetShaderResources( 0, 1, viewList );

		return true;
	}


	bool DX11ForwardPixelShader::Attach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null!" );

		m_Context->PSSetShader( m_PixelShader.Get(), NULL, 0 );

		ID3D11SamplerState* samplerList[] = { m_Sampler.Get() };
		m_Context->PSSetSamplers( 0, 1, samplerList );

		return true;
	}


	void DX11ForwardPixelShader::Detach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null!" );

		ID3D11Buffer* bufferList[] = { NULL };
		m_Context->PSSetConstantBuffers( 0, 1, bufferList );

		ID3D11ShaderResourceView* resourceList[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };
		m_Context->PSSetShaderResources( 0, 7, resourceList );

		ID3D11SamplerState* samplerList[] = { NULL };
		m_Context->PSSetSamplers( 0, 1, samplerList );

		m_Context->PSSetShader( NULL, NULL, 0 );
	}

}