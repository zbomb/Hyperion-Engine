/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resources/RShader.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{
	// Forward declarations
	class Renderer;
	class RMaterial;
	class RLightBuffer;
	class RViewClusters;
	class GBuffer;

	class RShaderBase
	{

	public:

		virtual ~RShaderBase() {}
		virtual void Shutdown() = 0;
		virtual bool IsValid() const = 0;

		virtual bool UploadStaticParameters( Renderer& inRenderer, uint32 inFlags ) = 0;
		virtual bool UploadLightBuffer( RLightBuffer& inLights ) = 0;
		virtual bool UploadViewClusters( RViewClusters& inClusters ) = 0;
		virtual bool UploadGBuffer( GBuffer& inGBuffer ) = 0;

		virtual bool Attach() = 0;
		virtual void Detach() = 0;

	};


	class RComputeShader : public RShaderBase
	{

	public:

		virtual ~RComputeShader() {}
		virtual bool Dispatch() = 0;

		virtual bool RequiresGBuffer() const = 0;

	};


	class RPixelShader : public RShaderBase
	{

	public:

		virtual ~RPixelShader() {}
		virtual bool UploadPrimitiveParameters( const Matrix& inWorldMatrix, const RMaterial& inMaterial ) = 0;

	};


	class RVertexShader : public RShaderBase
	{

	public:

		virtual ~RVertexShader() {}
		virtual bool UploadPrimitiveParameters( const Matrix& inWorldMatrix, const RMaterial& inMaterial ) = 0;

	};


	class RGeometryShader : public RShaderBase
	{

	public:

		virtual ~RGeometryShader() {}
		virtual bool UploadPrimitiveParameters( const Matrix& inWorldMatrix, const RMaterial& inMaterial ) = 0;

	};

}