/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Proxy/ProxyStaticModel.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/Proxy/ProxyStaticModel.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Renderer/Renderer.h"


namespace Hyperion
{

	ProxyStaticModel::ProxyStaticModel( uint32 inIdentifier )
		: ProxyPrimitive( inIdentifier ), m_LODCount( 0 ), m_ActiveLOD( 0 )
	{

	}

	ProxyStaticModel::~ProxyStaticModel()
	{
		HYPERION_VERIFY( !m_Model && !m_ModelAsset, "[Proxy] StaticModel wasnt cleaned up before being destroyed! (model instance/asset)" );
		HYPERION_VERIFY( m_Materials.empty() && m_MaterialAssets.empty(), "[Proxy] StaticModel wasnt cleaned up before being destroyed! (material instances/assets)" );
	}


	void ProxyStaticModel::GameInit()
	{
		HYPERION_VERIFY( IsGameThread(), "[Proxy] GameInit wasnt on game thread?" );
	}


	void ProxyStaticModel::RenderInit()
	{
		HYPERION_VERIFY( IsRenderThread(), "[Proxy] RenderInit wasnt on the render thread?" );

		// Turn the assets into render resource pointers
		if( !m_ModelAsset )
		{
			Console::WriteLine( "[Warning] ProxyStaticModel: Failed to init render resources.. the model asset was invalid" );
			return;
		}

		// First, get geometry resource pointer
		auto resManager = Engine::GetRenderer()->GetResourceManager();
		m_Model = resManager->GetMesh( m_ModelAsset );

		if( !m_Model )
		{
			Console::WriteLine( "[Warning] ProxyStaticModel: Failed to init render resources.. geometry couldnt be initialized" );
			return;
		}

		m_LODCount = m_ModelAsset->GetNumLODs();

		// Next, get material resource pointers
		for( auto it = m_MaterialAssets.begin(); it != m_MaterialAssets.end(); it++ )
		{
			if( it->second )
			{
				m_Materials[ it->first ] = resManager->CreateMaterial( it->second );
				if( !m_Materials[ it->first ] )
				{
					Console::WriteLine( "[Warning] ProxyStaticModel: Failed to init render resources.. material instance couldnt be created" );
				}
			}
		}
	}


	void ProxyStaticModel::BeginShutdown()
	{
		HYPERION_VERIFY( IsRenderThread(), "[Proxy] RenderShutdown wasnt on the render thread" );

		m_Model.reset();
		m_ModelAsset.reset();
		m_Materials.clear();
		m_MaterialAssets.clear();
	}


	void ProxyStaticModel::Shutdown()
	{

	}


	std::shared_ptr< RMaterial > ProxyStaticModel::GetMaterial( uint8 inSlot )
	{
		auto entry = m_Materials.find( inSlot );
		if( entry == m_Materials.end() ) { return nullptr; }

		return entry->second;
	}


	uint8 ProxyStaticModel::GetActiveLOD() const
	{
		return m_ActiveLOD;
	}


	AABB ProxyStaticModel::GetAABB() const
	{
		return m_ModelAsset ? m_ModelAsset->GetAABB() : AABB();
	}


	BoundingSphere ProxyStaticModel::GetBoundingSphere() const
	{
		return m_ModelAsset ? m_ModelAsset->GetBoundingSphere() : BoundingSphere();
	}


	void ProxyStaticModel::CacheMeshes()
	{
		m_CachedMeshes.clear();

		auto data		= m_Model ? m_Model->GetData() : nullptr;
		auto lod		= data ? data->GetLOD( m_ActiveLOD ) : nullptr;
		bool bSuccess	= true;

		if( lod->bCached )
		{
			for( auto it = lod->batchList.begin(); it != lod->batchList.end(); it++ )
			{
				auto mat = GetMaterial( it->materialSlot );
				if( mat && mat->AreTexturesLoaded() )
				{
					auto vert = it->vertexBuffer && it->vertexBuffer->IsValid() ? it->vertexBuffer : nullptr;
					auto indx = it->indexBuffer && it->indexBuffer->IsValid() ? it->indexBuffer : nullptr;

					if( vert && indx )
					{
						m_CachedMeshes.emplace_back( mat->IsTranslucent(), indx, vert, mat );
					}
					else
					{
						bSuccess = false;
					}
				}
				else
				{
					bSuccess = false;
				}
			}
		}
		else
		{
			bSuccess = false;
		}

		// Mark as clean, if we managed to fully cache the LOD
		m_bCacheDirty = !bSuccess;
	}


	void ProxyStaticModel::CollectBatches( BatchCollector& inCollector )
	{
		for( int i = 0; i < m_CachedMeshes.size(); i++ )
		{
			auto& batch = m_CachedMeshes[ i ];
			if( batch.Translucent )
			{
				inCollector.CollectOpaque( m_WorldMatrix, batch.Material, batch.IndexBuffer, batch.VertexBuffer );
			}
			else
			{
				inCollector.CollectOpaque( m_WorldMatrix, batch.Material, batch.IndexBuffer, batch.VertexBuffer );
			}
		}
	}

}