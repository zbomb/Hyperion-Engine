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


	Transform ProxyStaticModel::GetWorldTransform() const
	{
		return m_Transform;
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


	bool ProxyStaticModel::GetLODResources( uint8 inLOD, std::vector<std::tuple<std::shared_ptr<RBuffer>, std::shared_ptr<RBuffer>, std::shared_ptr<RMaterial>>>& outData )
	{
		// First, we need to grab the model data instance
		auto data = m_Model ? m_Model->GetData() : nullptr;
		if( !data ) { return false; }

		auto lod = data->GetLOD( inLOD );
		if( !lod || !lod->bCached ) { return false; }

		for( auto it = lod->subObjects.begin(); it != lod->subObjects.end(); it++ )
		{
			outData.emplace_back( std::make_tuple(
				it->vertexBuffer,
				it->indexBuffer,
				GetMaterial( it->materialSlot )
			) );
		}

		return true;
	}

}