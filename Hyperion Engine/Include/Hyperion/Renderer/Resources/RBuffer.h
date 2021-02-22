/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resources/RBuffer.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RGraphicsResource.h"


namespace Hyperion
{
	/*
	*	BufferType Enum
	*/
	enum class BufferType
	{
		Vertex				= 1,
		Index				= 2,
		Constants			= 3,
		Structured			= 4,
		StructuredAppend	= 5,
		Other				= 6
	};

	/*
	*	Buffer Parameters Structure
	*/
	struct BufferParameters
	{
		BufferType Type;
		uint32 ResourceAccess;
		bool CanCPURead;
		bool Dynamic;
		uint32 ElementSize;
		uint32 Count;
		const void* Data;
		uint32 SourceAsset;

		BufferParameters()
			: Type( BufferType::Vertex ), CanCPURead( false ), Dynamic( false ), Count( 0 ), Data( nullptr ), ResourceAccess( RENDERER_RESOURCE_ACCESS_NONE ), ElementSize( 0 )
		{}
	};

	/*
	*	Buffer Interface
	*/
	class RBuffer : public RGraphicsResource
	{

	public:

		virtual ~RBuffer() {}
		virtual BufferType GetType() const = 0;
		virtual void Shutdown() = 0;
		virtual bool IsValid() const = 0;
		virtual uint32 GetSize() const = 0;
		virtual uint32 GetCount() const = 0;
		virtual uint32 GetElementSize() const = 0;
		virtual uint32 GetAssetIdentifier() const = 0;

	};

}