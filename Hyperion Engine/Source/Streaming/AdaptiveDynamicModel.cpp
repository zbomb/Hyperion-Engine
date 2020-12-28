/*==================================================================================================
	Hyperion Engine
	Source/Streaming/AdaptiveDynamicModel.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Streaming/AdaptiveDynamicModel.h"



namespace Hyperion
{

	AdaptiveDynamicModel::AdaptiveDynamicModel( const std::shared_ptr< DynamicModelAsset >& inAsset )
	{
		HYPERION_NOT_IMPLEMENTED( "Adaptive Dynamic Model Class" );
	}


	AdaptiveDynamicModel::~AdaptiveDynamicModel()
	{

	}

	void AdaptiveDynamicModel::CancelStaleRequests()
	{
	}

	void AdaptiveDynamicModel::GenerateRequests( std::vector< std::shared_ptr< AdaptiveModelLoadRequest > >& outLoads,
												 std::vector< std::shared_ptr< AdaptiveModelUnloadRequest > >& outUnloads )
	{

	}

	bool AdaptiveDynamicModel::Update( float globalMult, float charMult, float dynMult, float lvlMult, float staticMult, uint32 objCount )
	{
		return false;
	}

	bool AdaptiveDynamicModelLoadRequest::IsValid() const
	{
		return false;
	}

	float AdaptiveDynamicModelLoadRequest::GetPriority() const
	{
		return 0.0f;
	}

	uint32 AdaptiveDynamicModelLoadRequest::GetMemory() const
	{
		return uint32();
	}

	void AdaptiveDynamicModelLoadRequest::SetOwnerLock( bool bValue )
	{
	}

	void AdaptiveDynamicModelLoadRequest::OnComplete()
	{
	}

	void AdaptiveDynamicModelLoadRequest::OnFailed()
	{
	}

	void AdaptiveDynamicModelLoadRequest::SetOwner( const std::shared_ptr< AdaptiveModel >& inModel )
	{
		
	}

	bool AdaptiveDynamicModelUnloadRequest::IsValid() const
	{
		return false;
	}

	uint32 AdaptiveDynamicModelUnloadRequest::GetMemory() const
	{
		return uint32();
	}

	void AdaptiveDynamicModelUnloadRequest::SetOwnerLock( bool bValue )
	{
	}

	void AdaptiveDynamicModelUnloadRequest::OnComplete()
	{
	}

	void AdaptiveDynamicModelUnloadRequest::OnFailed()
	{
	}

	void AdaptiveDynamicModelUnloadRequest::SetOwner( const std::shared_ptr< AdaptiveModel >& inModel )
	{

	}

}
