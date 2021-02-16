/*==================================================================================================
	Hyperion Engine
	Source/Renderer/DirectX11/Shaders/DX11GBufferPixelShader.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/DirectX11/Shaders/DX11GBufferPixelShader.h"
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/Resources/RMaterial.h"
#include "Hyperion/Renderer/DirectX11/DirectX11Texture.h"
#include "Hyperion/File/FileSystem.h"


namespace Hyperion
{

	DX11GBufferPixelShader::DX11GBufferPixelShader()
		: m_AttachedBaseMap( ASSET_INVALID )
	{

	}


	DX11GBufferPixelShader::~DX11GBufferPixelShader()
	{
		Shutdown();
	}


	bool DX11GBufferPixelShader::Initialize( ID3D11Device* inDevice, ID3D11DeviceContext* inContext )
	{
		HYPERION_VERIFY( inDevice && inContext, "[DX11] Device and/or context were null" );

		// Store context pointer
		m_Context = inContext;

		auto shaderFile = FileSystem::OpenFile( FilePath( SHADER_PATH_DX11_PIXEL_GBUFFER, PathRoot::Game ), FileMode::Read );
		auto shaderSize = shaderFile ? shaderFile->GetSize() : 0;

		if( shaderSize <= 0 )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load gbuffer pixel shader, file wasnt found or was invalid" );
			return false;
		}

		std::vector< byte > shaderData {};
		{
			DataReader reader( shaderFile );
			reader.SeekBegin();

			if( reader.ReadBytes( shaderData, shaderSize ) != DataReader::ReadResult::Success )
			{
				Console::WriteLine( "[ERROR] DX11: Failed to load gbuffer pixel shader, the file couldnt be read" );
				return false;
			}
		}

		// Create the shader through DX11
		if( FAILED( inDevice->CreatePixelShader( shaderData.data(), shaderData.size(), NULL, m_PixelShader.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load gbuffer pixel shader, the shader couldnt be created" );
			Shutdown();
			return false;
		}

		D3D11_SAMPLER_DESC samplerDesc {};

		samplerDesc.Filter				= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias			= 0.0f;
		samplerDesc.MaxAnisotropy		= 1;
		samplerDesc.ComparisonFunc		= D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[ 0 ]	= 0;
		samplerDesc.BorderColor[ 1 ]	= 0;
		samplerDesc.BorderColor[ 2 ]	= 0;
		samplerDesc.BorderColor[ 3 ]	= 0;
		samplerDesc.MinLOD				= 0;
		samplerDesc.MaxLOD				= D3D11_FLOAT32_MAX;

		if( FAILED( inDevice->CreateSamplerState( &samplerDesc, m_WrapSampler.ReleaseAndGetAddressOf() ) ) )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to load gbuffer pixel shader, couldnt create sampler state" );
			Shutdown();
			return false;
		}

		return true;
	}


	void DX11GBufferPixelShader::Shutdown()
	{
		m_WrapSampler.Reset();
		m_PixelShader.Reset();
		m_Context.Reset();
	}


	bool DX11GBufferPixelShader::IsValid() const
	{
		return m_PixelShader && m_WrapSampler;
	}


	bool DX11GBufferPixelShader::UploadBatchMaterial( const RMaterial& inMaterial )
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null" );

		auto baseMap = inMaterial.GetBaseMap();
		if( baseMap == nullptr || !baseMap->IsValid() )
		{
			Console::WriteLine( "[ERROR] DX11: Failed to upload batch material, the material didnt have a valid base map" );
			return false;
		}

		auto baseMapIdentifier = baseMap->GetAssetIdentifier();
		if( m_AttachedBaseMap == ASSET_INVALID || m_AttachedBaseMap != baseMapIdentifier )
		{
			auto dx11Texture = std::dynamic_pointer_cast< DirectX11Texture2D >( baseMap );
			HYPERION_VERIFY( dx11Texture != nullptr, "[DX11] API type mismatch?" );

			ID3D11ShaderResourceView* viewList[] = { dx11Texture->GetView() };
			m_Context->PSSetShaderResources( 0, 1, viewList );

			m_AttachedBaseMap = baseMapIdentifier;
		}

		return true;
	}


	bool DX11GBufferPixelShader::Attach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null!" );

		m_Context->PSSetShader( m_PixelShader.Get(), NULL, 0 );
		
		ID3D11SamplerState* samplers[] = { m_WrapSampler.Get() };
		m_Context->PSSetSamplers( 0, 1, samplers );

		return true;
	}


	void DX11GBufferPixelShader::Detach()
	{
		HYPERION_VERIFY( m_Context, "[DX11] Device context was null!" );

		ID3D11SamplerState* samplers[]		= { NULL };
		ID3D11ShaderResourceView* views[]	= { NULL };

		m_Context->PSSetShaderResources( 0, 1, views );
		m_Context->PSSetSamplers( 0, 1, samplers );
		m_Context->PSSetShader( NULL, NULL, 0 );

		m_AttachedBaseMap = ASSET_INVALID;
	}

}