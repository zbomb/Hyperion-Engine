/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/Shaders/DX11FXAAShader.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/Shaders/DX11FXAAShader.h"
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/File/FileSystem.h"
#include "Hyperion/Renderer/DirectX11/DirectX11Texture.h"


namespace Hyperion
{



	DX11FXAAShader::DX11FXAAShader()
	{

	}


	DX11FXAAShader::~DX11FXAAShader()
	{

	}


	bool DX11FXAAShader::Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
	{
		HYPERION_VERIFY( inDevice && inContext, "[DX11] Device/Context was null!" );

		// Store context pointer
		m_Context = inContext;

		auto shaderFile = FileSystem::OpenFile( FilePath( SHADER_PATH_DX11_FX_FXAA, PathRoot::Game ), FileMode::Read );
		auto shaderSize = shaderFile ? shaderFile->GetSize() : 0;

		if( shaderSize <= 0 )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load FXAA shader, file wasnt found or was invalid" );
			return false;
		}

		std::vector< byte > shaderData {};
		{
			DataReader reader( shaderFile );
			reader.SeekBegin();

			if( reader.ReadBytes( shaderData, shaderSize ) != DataReader::ReadResult::Success )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to load FXAA shader, the file couldnt be read" );
				return false;
			}
		}

		// Create the shader through DX11
		if( FAILED( inDevice->CreatePixelShader( shaderData.data(), shaderData.size(), NULL, m_Shader.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load FXAA shader, the shader couldnt be created" );
			Shutdown();
			return false;
		}

		// Create the sampler state
		D3D11_SAMPLER_DESC samplerDesc {};

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
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
			Console::WriteLine( "[ERROR] DX11: Failed to load FXAA shader, couldnt create sampler state" );
			Shutdown();
			return false;
		}

		D3D11_BUFFER_DESC bufferDesc {};

		bufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags			= D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags			= 0;
		bufferDesc.StructureByteStride	= sizeof( ConstantBufferType );
		bufferDesc.ByteWidth			= sizeof( ConstantBufferType );
		
		if( FAILED( inDevice->CreateBuffer( &bufferDesc, NULL, m_ConstantBuffer.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load FXAA shader, couldnt create constant buffer" );
			Shutdown();
			return false;
		}

		return true;
	}


	void DX11FXAAShader::Shutdown()
	{
		m_Shader.Reset();
		m_Sampler.Reset();
		m_ConstantBuffer.Reset();
		m_Context.Reset();
	}


	bool DX11FXAAShader::IsValid() const
	{
		return m_Shader && m_Sampler;
	}


	bool DX11FXAAShader::UploadStaticParameters( Renderer& inRenderer, uint32 inFlags )
	{
		return true;
	}


	bool DX11FXAAShader::Attach( const std::shared_ptr<RTexture2D>& inSource )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null!" );

		// Validate the source buffer
		if( !inSource || !inSource->IsValid() )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to attach FXAA shader, the source buffer was invalid" );
			return false;
		}

		// Cast to DX11 type, and set the shader parameters
		auto* tex = dynamic_cast< DirectX11Texture2D* >( inSource.get() );
		auto* srv = tex ? tex->GetView() : nullptr;

		if( !srv )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to attach FXAA shader, the source buffer resource view was invalid" );
			return false;
		}

		m_Context->PSSetShader( m_Shader.Get(), NULL, 0 );
		
		ID3D11ShaderResourceView* viewList[]	= { srv };
		ID3D11SamplerState* stateList[]			= { m_Sampler.Get() };

		m_Context->PSSetShaderResources( 0, 1, viewList );
		m_Context->PSSetSamplers( 0, 1, stateList );

		D3D11_MAPPED_SUBRESOURCE resource {};

		if( FAILED( m_Context->Map( m_ConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to attach FXAA shader, the constant buffer couldnt be mapped" );
			return false;
		}

		auto* bufferPtr = (ConstantBufferType*) resource.pData;

		D3D11_TEXTURE2D_DESC sourceDesc {};
		tex->Get()->GetDesc( &sourceDesc );

		bufferPtr->ScreenWidth		= (float) sourceDesc.Width;
		bufferPtr->ScreenHeight		= (float) sourceDesc.Height;

		m_Context->Unmap( m_ConstantBuffer.Get(), 0 );
		
		ID3D11Buffer* bufferList[] = { m_ConstantBuffer.Get() };
		m_Context->PSSetConstantBuffers( 0, 1, bufferList );

		return true;
	}


	void DX11FXAAShader::Detach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null!" );

		ID3D11ShaderResourceView* viewList[]	= { NULL };
		ID3D11SamplerState* samplerList[]		= { NULL };
		ID3D11Buffer* bufferList[]				= { NULL };

		m_Context->PSSetConstantBuffers( 0, 1, bufferList );
		m_Context->PSSetShaderResources( 0, 1, viewList );
		m_Context->PSSetSamplers( 0, 1, samplerList );
		m_Context->PSSetShader( NULL, NULL, 0 );
	}

}