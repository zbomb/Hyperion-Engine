/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resource/Shader.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/DataTypes.h"



namespace Hyperion
{
	class RMaterial;
	class RMesh;
	class GBuffer;
	struct Matrix;


	/*
	*	TODO: Better shader system
	*	The current shader class doesnt represent a single 'shader'.. it represents a whole pipeline, IA -> Vertex -> Fragment shader
	*	So.. we could actually create a system where you can create indivisual shader programs, and then write a code file to represent this program in code
	*	Then, we can build a ShaderPipeline, using the shader classes
	*	Then we can do... Renderer::SetShaderPipeline( myPipeline);
	*	For example:
	*	
	*	-- In a Init function --
	*	auto myPixelShader = ShaderManager::GetPixelShader< MyPixelShaderClass >();
	*	auto myVertexShader = ShaderManager::GetVertexShader< MyVertexShaderClass >();
	*	auto shaderPipline = ShaderManager::CreatePipeline();
	*	
	*	-- In the render loop --
	*	api->SetShaderPipeline( shaderPipeline );
	*	api->SetPipelineOutput( myGBuffer );
	* 
	*	auto myPixelShaderType = std::dynamic_cast< ... >( shaderPipeline->GetPixelShader() );
	*	
	*	for( ... )
	*	{
	*		shaderPipeline->UploadMatrixData( worldMatrix, viewMatrix, projectionMatrix );
	*		myPixelShaderType->UploadMaterial( myMat );
	* 
	*		api->RenderMesh( myMesh );
	*	}
	*	
	*	api->SetShaderPipeline( lightingPipeline );
	*	api->SetPipelineOutputToScreen();
	*	
	*	auto myOtherShader = std::dynamic_cast< ... >( ... );
	*	myOtherShader->UploadGBuffer( myGBuffer );
	*	
	*	// Get screen geometry
	*	api->RenderScreenGeometry();
	*/
	class RShader
	{

	public:

		virtual ~RShader() {}

		virtual void Shutdown() = 0;
		virtual bool IsValid() const = 0;
		virtual ShaderType GetType() const = 0;

	};


	class RGBufferShader : public RShader
	{

	public:

		virtual ~RGBufferShader() {}

		inline ShaderType GetType() const final { return ShaderType::GBuffer; }
		virtual bool UploadMaterial( const std::shared_ptr< RMaterial >& inMaterial ) = 0;
		virtual bool UploadMatrixData( const Matrix& inWorldMatrix, const Matrix& inViewMatrix, const Matrix& inProjectionMatrix ) = 0;

	};


	class RLightingShader : public RShader
	{

	public:

		virtual ~RLightingShader() {}

		inline ShaderType GetType() const final { return ShaderType::Lighting; }
		virtual bool UploadGBuffer( const std::shared_ptr< GBuffer >& inBuffer ) = 0;
		virtual bool UploadMatrixData( const Matrix& inWorldMatrix, const Matrix& inViewMatrix, const Matrix& inProjectionMatrix ) = 0;
		virtual bool UploadLighting() = 0; // TODO: How to do this?
		virtual void ClearGBufferResources() = 0;

	};


	class RForwardShader : public RShader
	{

	public:

		virtual ~RForwardShader() {}

		inline ShaderType GetType() const final { return ShaderType::Forward; }
		virtual bool UploadMaterial( const std::shared_ptr< RMaterial >& inMaterial ) = 0;
		virtual bool UploadMatrixData( const Matrix& inWorldMatrix, const Matrix& inViewMatrix, const Matrix& inProjectionMatrix ) = 0;
		virtual bool UploadLighting() = 0;

	};


	class RComputeShader : public RShader
	{

	public:

		virtual ~RComputeShader() {}

		inline ShaderType GetType() const final { return ShaderType::Compute; }

	};


	class RCustomShader : public RShader
	{

	public:

		virtual ~RCustomShader() {}

		inline ShaderType GetType() const final { return ShaderType::Custom; }

	};

}