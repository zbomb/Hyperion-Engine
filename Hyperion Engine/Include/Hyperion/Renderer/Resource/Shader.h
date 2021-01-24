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
	class RViewClusters;


	/*
	*	TODO: Better shader system
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
		virtual bool UploadClusterData( const std::shared_ptr< RViewClusters >& inClusters ) = 0;
	};


	class RLightingShader : public RShader
	{

	public:

		virtual ~RLightingShader() {}

		inline ShaderType GetType() const final { return ShaderType::Lighting; }
		virtual bool UploadGBuffer( const std::shared_ptr< GBuffer >& inBuffer ) = 0;
		virtual bool UploadGBufferData( const Matrix& inViewMatrix, const Matrix& inProjectionMatrix ) = 0;
		virtual bool UploadMatrixData( const Matrix& inWorldMatrix, const Matrix& inViewMatrix, const Matrix& inProjectionMatrix ) = 0;
		virtual bool UploadLighting( const Color3F& inAmbientColor, float inAmbientIntensity, const std::vector< std::shared_ptr< ProxyLight > >& inLightList ) = 0;
		virtual bool UploadClusterData( const std::shared_ptr< RViewClusters >& inClusters ) = 0;
		virtual void ClearResources() = 0;

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

		virtual void Attach() = 0;
		virtual void Detach() = 0;
		inline ShaderType GetType() const final { return ShaderType::Compute; }
	};


	class RBuildClusterShader : public RComputeShader
	{

	public:

		virtual ~RBuildClusterShader() {}
		virtual bool UploadViewInfo( const Matrix& inProjectionMatrix, const ScreenResolution& inRes, float inScreenNear, float inScreenFar ) = 0;
		virtual bool Dispatch( const std::shared_ptr< RViewClusters >& outClusters ) = 0;
	};


	class RCompressClustersShader : public RComputeShader
	{

	public:

		virtual ~RCompressClustersShader() {}
		virtual bool Dispatch( const std::shared_ptr< RViewClusters >& outClusters ) = 0;
	};


	class RCustomShader : public RShader
	{

	public:

		virtual ~RCustomShader() {}

		inline ShaderType GetType() const final { return ShaderType::Custom; }

	};

}