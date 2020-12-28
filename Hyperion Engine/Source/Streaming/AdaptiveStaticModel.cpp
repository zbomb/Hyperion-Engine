/*==================================================================================================
	Hyperion Engine
	Source/Streaming/AdaptiveStaticModel.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Streaming/AdaptiveStaticModel.h"



namespace Hyperion
{

	/*-----------------------------------------------------------------------------------------
		Adaptive Static Model Class
	-----------------------------------------------------------------------------------------*/
	AdaptiveStaticModel::AdaptiveStaticModel( const std::shared_ptr< StaticModelAsset >& inAsset )
		: m_Asset( inAsset )
	{
		if( !inAsset )
		{
			Console::WriteLine( "[ERROR] AAManager: Failed to create adaptive static model instance.. the static model asset was null!" );
			m_Identifier = ASSET_INVALID;
		}
		else
		{
			m_Identifier = inAsset->GetIdentifier();

			// Generate the LOD structure
			for( auto It = inAsset->LODBegin(); It != inAsset->LODEnd(); It++ )
			{
				m_LODs[ It->Index ].bLoaded			= false;
				m_LODs[ It->Index ].LoadRequest		= nullptr;
				m_LODs[ It->Index ].UnloadRequest	= nullptr;
				m_LODs[ It->Index ].Priority		= 0.0f;
			}
		}
	}


	AdaptiveStaticModel::~AdaptiveStaticModel()
	{

	}


	bool AdaptiveStaticModel::Update( float globalMult, float charMult, float dynMult, float lvlMult, float staticMult, uint32 objCount )
	{
		bool bValidRefs = false;

		// First, lets reset the priority of each LOD
		for( auto It = m_LODs.begin(); It != m_LODs.end(); It++ )
		{
			It->second.Priority = 0.0f;
		}

		// Loop through each reference, and determine its scaled screen size
		for( auto It = m_Refs.begin(); It != m_Refs.end(); )
		{
			auto obj = It->second.lock();
			if( !obj || !obj->m_Valid )
			{
				It = m_Refs.erase( It );
				continue;
			}

			float mult = globalMult;

			switch( obj->m_Type )
			{
			case AdaptiveAssetObjectType::Character:
				mult *= charMult;
				break;
			case AdaptiveAssetObjectType::Dynamic:
				mult *= dynMult;
				break;
			case AdaptiveAssetObjectType::Level:
				mult *= lvlMult;
				break;
			case AdaptiveAssetObjectType::Static:
			default:
				mult *= staticMult;
				break;
			}

			m_Priority_Mult			= Math::Max( m_Priority_Mult, mult );
			float objScreenSize		= obj->m_ScreenSize * mult;
			bValidRefs				= true;

			// Now, we need to determine which LOD this ref should be using, and its priority
			uint8 targetIndex = 0;
			for( uint8 index = m_Asset->GetNumLODs() - 1; index >= 0; index-- )
			{
				auto lod = m_Asset->GetLOD( index );
				if( lod == m_Asset->LODEnd() ) continue;

				if( objScreenSize > lod->MinScreenSize )
				{
					targetIndex = index;
					break;
				}
			}

			// Finally, lets mark this levels priority
			// TODO: CALCULATE PRIOIRITY
			float priority = 1.f;
			
			m_LODs[ targetIndex ].Priority = Math::Max( m_LODs[ targetIndex ].Priority, priority );
		}

		return bValidRefs;
	}


	void AdaptiveStaticModel::CancelStaleRequests()
	{
		for( auto It = m_LODs.begin(); It != m_LODs.end(); It++ )
		{
			auto& lod = It->second;
			if( lod.Priority <= 0.0f )
			{
				// Check if we can cancel a load or unload request
				if( lod.LoadRequest )
				{
					lod.LoadRequest->Cancel();
				}
			}
			else
			{
				if( lod.UnloadRequest )
				{
					lod.UnloadRequest->Cancel();
				}
			}
		}
	}


	void AdaptiveStaticModel::GenerateRequests( std::vector<std::shared_ptr<AdaptiveModelLoadRequest>>& outLoads, std::vector< std::shared_ptr< AdaptiveModelUnloadRequest > >& outUnloads )
	{
		for( auto It = m_LODs.begin(); It != m_LODs.end(); It++ )
		{
			auto& lod = It->second;
			if( lod.Priority > 0.0f && !lod.bLoaded && !lod.LoadRequest )
			{
				if( lod.UnloadRequest ) 
				{ 
					lod.UnloadRequest->Cancel(); 
					lod.UnloadRequest = nullptr; 
				}

				// TODO: Generate the request, store it localy
				auto req = std::make_shared< AdaptiveStaticModelLoadRequest >();

				req->m_Memory		= 0;
				req->m_Index		= It->first;
				req->m_Target		= nullptr; // Set after this function is called TODO: This is messy
				req->m_Valid		= true;

				lod.LoadRequest = req;
				outLoads.push_back( req );
			}
			else if( lod.Priority <= 0.0f && lod.bLoaded && !lod.UnloadRequest )
			{
				if( lod.LoadRequest )
				{
					lod.LoadRequest->Cancel();
					lod.LoadRequest = nullptr;
				}

				// TODO: Generate the request, store locally
				auto req = std::make_shared< AdaptiveStaticModelUnloadRequest >();

				req->m_Index	= It->first;
				req->m_Memory	= 0;
				req->m_Target	= nullptr; // Set after this function is called TODO: This is messy
				req->m_Valid	= true;

				lod.UnloadRequest = req;
				outUnloads.push_back( req );
			}
		}
	}



	/*-----------------------------------------------------------------------------------------
		Adaptive Static Model Load Request Class
	-----------------------------------------------------------------------------------------*/
	bool AdaptiveStaticModelLoadRequest::IsValid() const
	{
		return m_Valid && m_Target && m_Target->IsValid();
	}


	float AdaptiveStaticModelLoadRequest::GetPriority() const
	{
		if( m_Target )
		{
			auto entry = m_Target->m_LODs.find( m_Index );
			if( entry != m_Target->m_LODs.end() )
			{
				return entry->second.Priority;
			}
		}

		return 0.f;
	}


	uint32 AdaptiveStaticModelLoadRequest::GetMemory() const
	{
		return m_Memory;
	}


	void AdaptiveStaticModelLoadRequest::SetOwnerLock( bool bValue )
	{
		if( IsValid() )
		{
			m_Target->SetLock( bValue );
		}
	}


	void AdaptiveStaticModelLoadRequest::OnComplete()
	{

	}


	void AdaptiveStaticModelLoadRequest::OnFailed()
	{
		
	}


	std::shared_ptr< StaticModelAsset > AdaptiveStaticModelLoadRequest::GetAsset() const
	{
		auto target = GetTarget();
		return target ? target->GetAsset() : nullptr;
	}

	
	void AdaptiveStaticModelLoadRequest::Cancel()
	{
		m_Valid		= false;
	}

	void AdaptiveStaticModelLoadRequest::SetOwner( const std::shared_ptr< AdaptiveModel >& inModel )
	{
		if( inModel == nullptr )
		{
			m_Target = nullptr;
		}
		else
		{
			auto casted_ptr = std::dynamic_pointer_cast<AdaptiveStaticModel>( inModel );
			HYPERION_VERIFY( casted_ptr != nullptr, "Attempt to set dynamic adaptive model as the owner of a static model load request" );

			m_Target = casted_ptr;
		}
	}


	/*-----------------------------------------------------------------------------------------
		Adaptive Static Model Unload Request Class
	-----------------------------------------------------------------------------------------*/
	bool AdaptiveStaticModelUnloadRequest::IsValid() const
	{
		return m_Valid && m_Target && m_Target->IsValid();
	}


	uint32 AdaptiveStaticModelUnloadRequest::GetMemory() const
	{
		auto asset = m_Target ? m_Target->GetAsset() : nullptr;
		if( asset )
		{
			auto it = asset->GetLOD( m_Index );
			if( it != asset->LODEnd() )
			{
				// TODO: How to calculate memory? With multiple subobjects this gets hard
				return 0;
			}
		}
		return 0;
	}


	void AdaptiveStaticModelUnloadRequest::SetOwnerLock( bool bValue )
	{
		if( IsValid() )
		{
			m_Target->SetLock( bValue );
		}
	}


	void AdaptiveStaticModelUnloadRequest::OnComplete()
	{

	}


	void AdaptiveStaticModelUnloadRequest::OnFailed()
	{

	}

	std::shared_ptr< StaticModelAsset > AdaptiveStaticModelUnloadRequest::GetAsset() const
	{
		auto target = GetTarget();
		return target ? target->GetAsset() : nullptr;
	}


	void AdaptiveStaticModelUnloadRequest::Cancel()
	{
		m_Valid = false;
	}

	void AdaptiveStaticModelUnloadRequest::SetOwner( const std::shared_ptr< AdaptiveModel >& inModel )
	{
		if( inModel == nullptr )
		{
			m_Target = nullptr;
		}
		else
		{
			auto casted_ptr = std::dynamic_pointer_cast<AdaptiveStaticModel>( inModel );
			HYPERION_VERIFY( casted_ptr != nullptr, "Attempt to set dynamic adaptive model as the owner of a static model unload request" );

			m_Target = casted_ptr;
		}
	}

}
