/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/AdaptiveModel.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Streaming/AdaptiveBase.h"


namespace Hyperion
{
	// Forward Declarations
	struct AdaptiveModelLoadRequest;
	struct AdaptiveModelUnloadRequest;


	class AdaptiveModel : public AdaptiveBase
	{

	protected:


	public:

		virtual bool IsValid() const = 0;
		virtual bool IsDynamic() const = 0;
		virtual void CancelStaleRequests() = 0;
		virtual void GenerateRequests( std::vector< std::shared_ptr< AdaptiveModelLoadRequest > >& outLoads,
									   std::vector< std::shared_ptr< AdaptiveModelUnloadRequest > >& outUnloads ) = 0;		

	};


	struct AdaptiveModelLoadRequest
	{

	protected:

		bool m_Valid;
		uint8 m_Index;
		uint32 m_Memory;

	public:

		virtual bool IsValid() const = 0;
		virtual float GetPriority() const = 0;
		virtual uint32 GetMemory() const = 0;
		virtual void SetOwnerLock( bool ) = 0;
		virtual void OnComplete() = 0;
		virtual void OnFailed() = 0;
		virtual bool IsDynamic() const = 0;
		virtual void Cancel() = 0;
		virtual void SetOwner( const std::shared_ptr< AdaptiveModel >& ) = 0;

		friend class AdaptiveStaticModel;
		friend class AdpaptiveDynamicModel;

	};

	struct AdaptiveModelUnloadRequest
	{

	protected:

		bool m_Valid;
		uint8 m_Index;
		uint32 m_Memory;

	public:

		virtual bool IsValid() const = 0;
		virtual uint32 GetMemory() const = 0;
		virtual void SetOwnerLock( bool ) = 0;
		virtual void OnComplete() = 0; // This has to update the owning 'model' objects LOD list
		virtual void OnFailed() = 0;
		virtual bool IsDynamic() const = 0;
		virtual void SetOwner( const std::shared_ptr< AdaptiveModel >& ) = 0;
		virtual void Cancel() = 0;

		friend class AdaptiveStaticModel;
		friend class AdaptiveDynamicModel;
	};
}