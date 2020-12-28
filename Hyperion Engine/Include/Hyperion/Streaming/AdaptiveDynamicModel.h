/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/AdaptiveDynamicModel.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Streaming/AdaptiveModel.h"
#include "Hyperion/Assets/DynamicModelAsset.h"


namespace Hyperion
{

	class AdaptiveDynamicModel : public AdaptiveModel
	{

	private:

	public:

		AdaptiveDynamicModel( const std::shared_ptr< DynamicModelAsset >& inAsset );
		~AdaptiveDynamicModel();

		AdaptiveDynamicModel() = delete;

		void CancelStaleRequests() final;
		void GenerateRequests( std::vector< std::shared_ptr< AdaptiveModelLoadRequest > >& outLoads,
							   std::vector< std::shared_ptr< AdaptiveModelUnloadRequest > >& outUnloads ) final;

		inline bool IsDynamic() const final { return true; }
		bool Update( float globalMult, float charMult, float dynMult, float lvlMult, float staticMult, uint32 objCount ) final;

	};


	struct AdaptiveDynamicModelLoadRequest : public AdaptiveModelLoadRequest
	{

	public:

		bool IsValid() const final;
		float GetPriority() const final;
		uint32 GetMemory() const final;
		void SetOwnerLock( bool bValue ) final;
		void OnComplete() final;
		void OnFailed() final;
		inline bool IsDynamic() const final { return true; }

		void SetOwner( const std::shared_ptr< AdaptiveModel >& inModel ) final;
	};


	struct AdaptiveDynamicModelUnloadRequest : public AdaptiveModelUnloadRequest
	{

	public:

		bool IsValid() const final;
		uint32 GetMemory() const final;
		void SetOwnerLock( bool bValue ) final;
		void OnComplete() final;
		void OnFailed() final;
		inline bool IsDynamic() const final { return true; }

		void SetOwner( const std::shared_ptr< AdaptiveModel >& inModel ) final;
	};

}