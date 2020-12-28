/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/AdaptiveStaticModel.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Streaming/AdaptiveModel.h"
#include "Hyperion/Assets/StaticModelAsset.h"


namespace Hyperion
{
	// Forward Declarations
	struct AdaptiveStaticModelLoadRequest;
	struct AdaptiveStaticModelUnloadRequest;


	struct AdaptiveStaticModelLODState
	{
		bool bLoaded;
		float Priority;
		std::shared_ptr< AdaptiveStaticModelLoadRequest > LoadRequest;
		std::shared_ptr< AdaptiveStaticModelUnloadRequest > UnloadRequest;
	};

	class AdaptiveStaticModel : public AdaptiveModel
	{

	private:

		std::shared_ptr< StaticModelAsset > m_Asset;
		std::map< uint8, AdaptiveStaticModelLODState > m_LODs;

	public:

		AdaptiveStaticModel( const std::shared_ptr< StaticModelAsset >& inAsset );
		~AdaptiveStaticModel();

		AdaptiveStaticModel() = delete;

		void CancelStaleRequests() final;
		void GenerateRequests( std::vector< std::shared_ptr< AdaptiveModelLoadRequest > >& outLoads,
							   std::vector< std::shared_ptr< AdaptiveModelUnloadRequest > >& outUnloads ) final;

		inline std::shared_ptr< StaticModelAsset > GetAsset() const { return m_Asset; }
		inline bool IsDynamic() const final { return false; }


		bool Update( float globalMult, float charMult, float dynMult, float lvlMult, float staticMult, uint32 objCount ) final;

		friend struct AdaptiveStaticModelLoadRequest;
		friend struct AdaptiveStaticModelUnloadRequest;
	};


	struct AdaptiveStaticModelLoadRequest : public AdaptiveModelLoadRequest
	{

	private:

		std::shared_ptr< AdaptiveStaticModel > m_Target;

	public:

		bool IsValid() const final;
		float GetPriority() const final;
		uint32 GetMemory() const final;
		void SetOwnerLock( bool bValue ) final;
		void OnComplete() final;
		void OnFailed() final;
		inline bool IsDynamic() const final { return false; }
		void Cancel() final;
		void SetOwner( const std::shared_ptr< AdaptiveModel >& inModel ) final;

		inline std::shared_ptr< AdaptiveStaticModel > GetTarget() const { return m_Target; }
		std::shared_ptr< StaticModelAsset > GetAsset() const;

		friend class AdaptiveStaticModel;

	};


	struct AdaptiveStaticModelUnloadRequest : public AdaptiveModelUnloadRequest
	{

	private:

		std::shared_ptr< AdaptiveStaticModel > m_Target;

	public:

		bool IsValid() const final;
		uint32 GetMemory() const final;
		void SetOwnerLock( bool bValue ) final;
		void OnComplete() final;
		void OnFailed() final;
		inline bool IsDynamic() const final { return false; }
		void Cancel() final;
		void SetOwner( const std::shared_ptr< AdaptiveModel >& inModel ) final;

		inline std::shared_ptr< AdaptiveStaticModel > GetTarget() const { return m_Target; }
		std::shared_ptr< StaticModelAsset > GetAsset() const;

		friend class AdaptiveStaticModel;

	};

}