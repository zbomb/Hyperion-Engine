/*==================================================================================================
	Hyperion Engine
	Source/Renderer/Resource/ResourceManager.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Renderer/ResourceManager.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/Resources/RTexture.h"


namespace Hyperion
{

	ResourceManager::ResourceManager()
	{

	}


	ResourceManager::~ResourceManager()
	{
		Shutdown();
	}


	void ResourceManager::Shutdown()
	{
		m_Geometry.clear();
		m_Textures.clear();
	}


	std::shared_ptr<RMaterial> ResourceManager::CreateMaterial( const std::shared_ptr<MaterialAsset>& inAsset )
	{
		HYPERION_VERIFY( inAsset != nullptr, "[ResourceManager] Null material asset passed to CreateMaterial" );
		
		// Create material isntance with values copied in
		auto mat = std::make_shared< RMaterial >( inAsset->GetIdentifier(), inAsset->ValuesBegin(), inAsset->ValuesEnd() );

		// Loop through the textures in the material asset, and get the texture asset
		for( auto it = inAsset->TexturesBegin(); it != inAsset->TexturesEnd(); it++ )
		{
			if( it->second )
			{
				auto texPtr = Get2DTexture( it->second->GetIdentifier() ); // LEFT OFF HERE: This is runnin on the game thread, because CreateProxy runs on the game thread
				HYPERION_VERIFY( texPtr != nullptr, "[ResourceManager] Failed to get pointer for texture" );

				mat->AddTexture( it->first, texPtr );
			}
		}

		mat->CacheTextures();

		return mat;
	}


	std::shared_ptr< RMeshData > ResourceManager::GetMeshData( uint32 inIdentifier )
	{
		if( inIdentifier == ASSET_INVALID ) { return nullptr; }

		auto entry = m_Geometry.find( inIdentifier );
		if( entry == m_Geometry.end() || !entry->second ) { return nullptr; }

		return entry->second->GetData();
	}


	bool ResourceManager::IsMeshLODCached( uint32 inIdentifier, uint8 inLOD )
	{
		if( inLOD > MODEL_MAX_LODS ) { return false; }

		auto ptr = GetMeshData( inIdentifier );
		return ptr ? ptr->IsLODCached( inLOD ) : false;
	}


	bool ResourceManager::IsMeshFullyCached( uint32 inIdentifier )
	{
		auto ptr = GetMeshData( inIdentifier );
		return ptr ? ptr->IsFullyCached() : false;
	}


	bool ResourceManager::IsMeshPartiallyCached( uint32 inIdentifier )
	{
		auto ptr = GetMeshData( inIdentifier );
		return ptr ? ptr->IsPartiallyCached() : false;
	}


	bool ResourceManager::UploadMeshLOD( const std::shared_ptr<StaticModelAsset>& inAsset, uint8 inLOD,
											 const std::vector< std::vector<byte> >& inVertexData, const std::vector< std::vector<byte> >& inIndexData )
	{
		// Validate parameters
		if( !inAsset || inLOD > MODEL_MAX_LODS || inVertexData.size() == 0 || inIndexData.size() == 0 || inVertexData.size() != inIndexData.size() )
		{
			Console::WriteLine( "[ERROR] ResourceManager: Failed to upload geometry data! Parameters were invalid" );
			return false;
		}

		// If were not already on the render thread, run this on there instead
		if( !IsRenderThread() )
		{
			Engine::GetRenderer()->AddCommand(
				std::make_unique< RenderCommand >( std::bind( &ResourceManager::UploadMeshLOD, this, inAsset, inLOD, inVertexData, inIndexData ) )
			);

			return true;
		}

		// Check if there is an existing geometry instance, create one if not
		std::shared_ptr< RMeshData > data;

		auto id		= inAsset->GetIdentifier();
		auto entry	= m_Geometry.find( id );

		if( entry == m_Geometry.end() )
		{
			data = std::make_shared< RMeshData >( inAsset );
			m_Geometry.emplace( id, std::shared_ptr< RMesh >( new RMesh( id, data ) ) );
		}

		auto targetLOD = data->GetLOD( inLOD );
		if( !targetLOD )
		{
			Console::WriteLine( "[ERROR] ResourceManager: Failed to upload geometry data! Invalid LOD targeted!" );
			return false;
		}

		// Now that we have the actual LOD, we need to create the buffers from data we have, lets just make sure that the number of subobjects, and the number of byte vectors arent out of line
		if( inVertexData.size() != targetLOD->batchList.size() )
		{
			Console::WriteLine( "[ERROR] ResourceManager: Failed to upload geometry data! Invalid number of subobject data sets provided!" );
			return false;
		}

		for( uint8 i = 0; i < targetLOD->batchList.size(); i++ )
		{
			if( targetLOD->batchList[ i ].indexBuffer )		{ targetLOD->batchList[ i ].indexBuffer.reset(); }
			if( targetLOD->batchList[ i ].vertexBuffer )	{ targetLOD->batchList[ i ].vertexBuffer.reset(); }

			BufferParameters vparams;

			vparams.CanCPURead	= false;
			vparams.Dynamic		= false;
			vparams.Size		= (uint32)inVertexData[ i ].size();
			vparams.Type		= BufferType::Vertex;
			vparams.Data		= inVertexData[ i ].data();
			vparams.Count		= vparams.Size / 32;
			vparams.SourceAsset = id;

			targetLOD->batchList[ i ].vertexBuffer = Engine::GetRenderer()->GetAPI()->CreateBuffer( vparams );

			BufferParameters iparams;

			iparams.CanCPURead	= false;
			iparams.Dynamic		= false;
			iparams.Size		= (uint32)inIndexData[ i ].size();
			iparams.Type		= BufferType::Index;
			iparams.Data		= inIndexData[ i ].data();
			iparams.Count		= iparams.Size / 4;
			iparams.SourceAsset = id;

			targetLOD->batchList[ i ].indexBuffer = Engine::GetRenderer()->GetAPI()->CreateBuffer( iparams );

			HYPERION_VERIFY( targetLOD->batchList[ i ].vertexBuffer && targetLOD->batchList[ i ].indexBuffer, "[ResourceManager] Failed to create buffers for geometry!" );
		}

		targetLOD->bCached = true;
		return true;
	}


	bool ResourceManager::RemoveMeshLOD( const std::shared_ptr<StaticModelAsset>& inAsset, uint8 inLOD )
	{
		// Validate Parameters
		if( !inAsset || inLOD > MODEL_MAX_LODS )
		{
			Console::WriteLine( "[ERROR] ResourceManager: Failed to remove Geometry LOD from memory! Invalid asset/LOD number" );
			return false;
		}

		// If were not already on the render thread, run this on there instead
		if( !IsRenderThread() )
		{
			Engine::GetRenderer()->AddCommand(
				std::make_unique< RenderCommand >( std::bind( &ResourceManager::RemoveMeshLOD, this, inAsset, inLOD ) )
			);

			return true;
		}

		// Find entry
		auto id		= inAsset->GetIdentifier();
		auto entry	= m_Geometry.find( id );

		if( entry != m_Geometry.end() && entry->second && entry->second->m_Data )
		{
			auto lodPtr = entry->second->m_Data->GetLOD( inLOD );
			if( lodPtr )
			{
				lodPtr->bCached = false;
				
				for( auto it = lodPtr->batchList.begin(); it != lodPtr->batchList.end(); it++ )
				{
					it->vertexBuffer.reset();
					it->indexBuffer.reset();
				}
			}
		}

		return true;
	}


	bool ResourceManager::UploadFullMesh( const std::shared_ptr<StaticModelAsset>& inAsset, 
											  const std::vector< std::vector<std::vector<byte>>>& inVertexData, const std::vector< std::vector<std::vector<byte>>>& inIndexData )
	{
		// Validate parameters
		if( !inAsset || inVertexData.size() == 0 || inVertexData.size() != inIndexData.size() )
		{
			Console::WriteLine( "[ERROR] ResourceManager: Failed to upload geometry data! Invalid parameters" );
			return false;
		}

		// If were not already on the render thread, run this on there instead
		if( !IsRenderThread() )
		{
			Engine::GetRenderer()->AddCommand(
				std::make_unique< RenderCommand >( std::bind( &ResourceManager::UploadFullMesh, this, inAsset, inVertexData, inIndexData ) )
			);

			return true;
		}

		// Check if there is an existing geometry instance, create one if not
		std::shared_ptr< RMeshData > data;

		auto id		= inAsset->GetIdentifier();
		auto entry	= m_Geometry.find( id );

		if( entry == m_Geometry.end() )
		{
			data = std::make_shared< RMeshData >( inAsset );
			m_Geometry.emplace( id, std::shared_ptr< RMesh >( new RMesh( id, data ) ) );
		}
		else
		{
			// TODO: Better way to do this? 
			data = entry->second->m_Data;
		}

		if( inVertexData.size() != data->GetLODCount() )
		{
			Console::WriteLine( "[ERROR] ResourceManager: Failed to upload full geometry data, there wasnt data for every LOD" );
			return false;
		}

		auto api = Engine::GetRenderer()->GetAPI();

		for( uint8 i = 0; i < data->GetLODCount(); i++ )
		{
			auto lodPtr = data->GetLOD( i );
			HYPERION_VERIFY( lodPtr, "[ResourceManager] Geometry data LOD instance null?" );

			auto& vertexDataList	= inVertexData.at( i );
			auto& indexDataList		= inIndexData.at( i );

			if( vertexDataList.size() != indexDataList.size() || vertexDataList.size() != lodPtr->batchList.size() )
			{
				Console::WriteLine( "[ERROR] ResourceManager: Failed to upload full geometry data, provided data structure didnt match geometry structure" );
				return false;
			}

			for( uint8 j = 0; j < lodPtr->batchList.size(); j++ )
			{
				if( lodPtr->batchList[ j ].indexBuffer ) { lodPtr->batchList[ j ].indexBuffer.reset(); }
				if( lodPtr->batchList[ j ].vertexBuffer ) { lodPtr->batchList[ j ].vertexBuffer.reset(); }

				BufferParameters vparams;

				vparams.CanCPURead	= false;
				vparams.Dynamic		= false;
				vparams.Type		= BufferType::Vertex;
				vparams.Size		= (uint32) vertexDataList[ j ].size();
				vparams.Count		= vparams.Size / 32;
				vparams.Data		= vertexDataList[ j ].data();
				vparams.SourceAsset = id;

				lodPtr->batchList[ j ].vertexBuffer = api->CreateBuffer( vparams );

				BufferParameters iparams;

				iparams.CanCPURead	= false;
				iparams.Dynamic		= false;
				iparams.Type		= BufferType::Index;
				iparams.Size		= (uint32)indexDataList[ j ].size();
				iparams.Count		= iparams.Size / 4;
				iparams.Data		= indexDataList[ j ].data();
				iparams.SourceAsset = id;

				lodPtr->batchList[ j ].indexBuffer = api->CreateBuffer( iparams );

				HYPERION_VERIFY( lodPtr->batchList[ j ].indexBuffer && lodPtr->batchList[ j ].vertexBuffer, "[ResourceManager] Failed to create buffers" );
			}

			lodPtr->bCached = true;
		}

		return true;
	}


	bool ResourceManager::RemoveFullMesh( const std::shared_ptr<StaticModelAsset>& inAsset )
	{
		if( !inAsset ) { return false; }

		if( !IsRenderThread() )
		{
			Engine::GetRenderer()->AddCommand( std::make_unique< RenderCommand >( std::bind( &ResourceManager::RemoveFullMesh, this, inAsset ) ) );
			return true;
		}

		auto id = inAsset->GetIdentifier();
		auto entry = m_Geometry.find( id );

		if( entry != m_Geometry.end() && entry->second && entry->second->m_Data )
		{
			auto data = entry->second->m_Data;
			for( uint8 i = 0; i < data->GetLODCount(); i++ )
			{
				auto lodPtr = data->GetLOD( i );
				HYPERION_VERIFY( lodPtr, "[ResourceManager] LOD instance was null?" );

				for( auto it = lodPtr->batchList.begin(); it != lodPtr->batchList.end(); it++ )
				{
					if( it->indexBuffer )	{ it->indexBuffer.reset(); }
					if( it->vertexBuffer )	{ it->vertexBuffer.reset(); }
				}

				lodPtr->bCached = false;
			}
		}

		return true;
	}


	std::shared_ptr<RMesh> ResourceManager::GetMesh( const std::shared_ptr<StaticModelAsset>& inAsset )
	{
		if( !inAsset ) { return nullptr; }
		
		// Find geometry entry
		auto id = inAsset->GetIdentifier();
		auto entry = m_Geometry.find( id );
		std::shared_ptr< RMesh > ptr;

		if( entry == m_Geometry.end() )
		{
			ptr = m_Geometry.emplace( id, std::shared_ptr< RMesh >( new RMesh( id ) ) ).first->second;
		}
		else
		{
			ptr = entry->second;
		}

		// Ensure geometry entry is valid
		if( !ptr->m_Data )
		{
			ptr->m_Data = std::make_shared< RMeshData >( inAsset );
		}

		// Return the pointer
		return ptr;
	}


	void ResourceManager::ClearMeshes()
	{
		if( IsRenderThread() )
		{
			m_Geometry.clear();
		}
		else
		{
			Engine::GetRenderer()->AddCommand( std::make_unique< RenderCommand >( std::bind( &ResourceManager::ClearMeshes, this ) ) );
		}
	}


	std::shared_ptr<RTexture1D> ResourceManager::Get1DTexture( uint32 inIdentifier )
	{
		if( !IsRenderThread() )
		{
			Console::WriteLine( "[ERROR] ResourceManager: Attempt to access cache from outside render thread!" );
			return nullptr;
		}

		if( inIdentifier == ASSET_INVALID ) { return nullptr; }

		auto entry = m_Textures.find( inIdentifier );
		if( entry == m_Textures.end() )
		{
			auto newTex = Engine::GetRenderer()->GetAPI()->CreateTexture1D();
			m_Textures.emplace( inIdentifier, TextureEntry( TextureType::Tex1D, newTex ) );

			return newTex;
		}

		if( entry->second.type != TextureType::Tex1D ) { return nullptr; }
		return std::static_pointer_cast<RTexture1D>( entry->second.ptr );
	}

	std::shared_ptr<RTexture2D> ResourceManager::Get2DTexture( uint32 inIdentifier )
	{
		if( !IsRenderThread() )
		{
			Console::WriteLine( "[ERROR] ResourceManager: Attempt to access cache from outside render thread!" );
			return nullptr;
		}

		if( inIdentifier == ASSET_INVALID ) { return nullptr; }

		auto entry = m_Textures.find( inIdentifier );
		if( entry == m_Textures.end() )
		{
			auto newTex = Engine::GetRenderer()->GetAPI()->CreateTexture2D();
			m_Textures.emplace( inIdentifier, TextureEntry( TextureType::Tex2D, newTex ) );

			return newTex;
		}

		if( entry->second.type != TextureType::Tex2D ) { return nullptr; }
		return std::static_pointer_cast<RTexture2D>( entry->second.ptr );
	}

	std::shared_ptr<RTexture3D> ResourceManager::Get3DTexture( uint32 inIdentifier )
	{
		if( !IsRenderThread() )
		{
			Console::WriteLine( "[ERROR] TextureCache: Attempt to access cache from outside render thread!" );
			return nullptr;
		}

		if( inIdentifier == ASSET_INVALID ) { return nullptr; }

		auto entry = m_Textures.find( inIdentifier );
		if( entry == m_Textures.end() )
		{
			auto newTex = Engine::GetRenderer()->GetAPI()->CreateTexture3D();
			m_Textures.emplace( inIdentifier, TextureEntry( TextureType::Tex3D, newTex ) );

			return newTex;
		}

		if( entry->second.type != TextureType::Tex3D ) { return nullptr; }
		return std::static_pointer_cast<RTexture3D>( entry->second.ptr );
	}

	std::shared_ptr<RTextureBase> ResourceManager::GetTexture( uint32 inIdentifier )
	{
		if( !IsRenderThread() )
		{
			Console::WriteLine( "[ERROR] ResourceManager: Attempt to access cache from outside render thread!" );
			return nullptr;
		}

		if( inIdentifier == ASSET_INVALID ) { return nullptr; }

		auto entry = m_Textures.find( inIdentifier );
		if( entry == m_Textures.end() ) { return nullptr; }

		return entry->second.type != TextureType::None ? entry->second.ptr : nullptr;
	}


	void ResourceManager::ClearTextures()
	{
		if( IsRenderThread() )
		{
			m_Textures.clear();
		}
		else
		{
			Engine::GetRenderer()->AddCommand( std::make_unique< RenderCommand >( std::bind( &ResourceManager::ClearTextures, this ) ) );
		}
	}


	void ResourceManager::IncreaseTextureDetail( const std::shared_ptr<TextureAsset>& inAsset, uint8 inNewMaxLOD, const std::vector<std::vector<byte>>& inData )
	{
		HYPERION_VERIFY( inAsset != nullptr && inNewMaxLOD <= TEXTURE_MAX_LODS && inData.size() > 0, "[ResourceManager] Update texture was called with invalid arguments" );

		auto renderer = Engine::GetRenderer();

		// Some more validation...
		const TextureHeader& textureHeader = inAsset->GetHeader();
		if( inNewMaxLOD >= textureHeader.LODs.size() )
		{
			Console::WriteLine( "[Warning] ResourceManager: Failed to update texture \"", inAsset->GetPath(), "\" because the selected LOD level (", (int) inNewMaxLOD, ") is invalid" );
			return;
		}

		uint8 newLODCount	= (uint8) ( textureHeader.LODs.size() - inNewMaxLOD );
		auto& maxLOD		= textureHeader.LODs.at( inNewMaxLOD );

		// Setup parameters for the new texture were going to create
		TextureParameters params;

		params.bCPURead		= false;
		params.bDynamic		= false;
		params.Format		= textureHeader.Format;
		params.Width		= maxLOD.Width;
		params.Height		= maxLOD.Height;
		params.bAutogenMips	= false;
		params.BindTargets	= RENDERER_TEXTURE_BIND_FLAG_SHADER;
		params.AssetIdentifier = inAsset->GetIdentifier();

		for( uint8 i = 0; i < newLODCount; i++ )
		{
			uint8 thisLevel		= i + inNewMaxLOD;
			auto& lodInfo		= textureHeader.LODs.at( thisLevel );

			TextureMipData mip;

			mip.Data			= const_cast< byte* >( inData.at( i ).data() );
			mip.RowSize			= lodInfo.RowSize;
			mip.LayerSize		= 0;

			params.Data.push_back( mip );
		}

		if( IsRenderThread() )
		{
			// Create the texture
			auto newTex = renderer->GetAPI()->CreateTexture2D( params );
			if( !newTex )
			{
				Console::WriteLine( "[Warning] ResourceManager: Failed to increase texture detail for \"", inAsset->GetPath(), "\" because the api couldnt create the new texture" );
				return;
			}

			// Find the existing texture entry (if there is one)
			auto it = m_Textures.find( inAsset->GetIdentifier() );
			if( it == m_Textures.end() )
			{
				// Insert new entry
				m_Textures.emplace( inAsset->GetIdentifier(), TextureEntry( TextureType::Tex2D, newTex ) );
			}
			else
			{
				// TODO: Allow 1D and 3D texture updates!
				HYPERION_VERIFY( it->second.type == TextureType::Tex2D, "[ResourceManager] Attempt to update existing texture, but its not 2D!" );

				if( it->second.ptr )
				{
					auto oldTex = std::dynamic_pointer_cast< RTexture2D >( it->second.ptr );
					HYPERION_VERIFY( oldTex != nullptr, "[ResourceManager] Failed to cast existing texture pointer to proper type!" );

					// Perform texture hotswap
					oldTex->Swap( *newTex );
					if( newTex->IsValid() ) { newTex->Shutdown(); }
					newTex.reset();
				}
				else
				{
					it->second.ptr = newTex;
				}
			}
		}
		else
		{
			if( renderer->GetAPI()->AllowAsyncTextureCreation() )
			{
				// Perform split-thread upload
				// Create the texture on this thread, and then insert a render command to update the texture cache
				auto newTex		= renderer->GetAPI()->CreateTexture2D( params );
				auto id			= inAsset->GetIdentifier();

				if( !newTex )
				{
					Console::WriteLine( "[Warning] ResourceManager: Failed to increase texture detail for \"", inAsset->GetPath(), "\" because the api couldnt create the new texture" );
					return;
				}

				renderer->AddCommand( std::make_unique< RenderCommand >(
					[newTex, id, this] ( Renderer& r )
					{
						auto entry = m_Textures.find( id );
						if( entry == m_Textures.end() )
						{
							m_Textures.emplace( id, TextureEntry( TextureType::Tex2D, newTex ) );
						}
						else
						{
							// TODO: Allow update of 1D and 3D textures
							HYPERION_VERIFY( entry->second.type == TextureType::Tex2D, "[ResourceManager] Attempt to update non-2D texture!" );

							if( entry->second.ptr )
							{
								auto oldTex = std::dynamic_pointer_cast<RTexture2D>( entry->second.ptr );
								HYPERION_VERIFY( oldTex != nullptr, "[ResourceManager] Failed to cast texture pointer to proper type" );

								oldTex->Swap( *newTex );
								if( newTex->IsValid() ) { newTex->Shutdown(); }
							}
							else
							{
								entry->second.ptr = newTex;
							}
						}
					}
				) );

			}
			else
			{
				// Call this function on the render thread instead
				renderer->AddCommand( std::make_unique< RenderCommand >( std::bind( &ResourceManager::IncreaseTextureDetail, this, inAsset, inNewMaxLOD, inData ) ) );
			}
		}
	}


	void ResourceManager::DecreaseTextureDetail( const std::shared_ptr<TextureAsset>& inAsset, uint8 inNewMaxLOD )
	{
		HYPERION_VERIFY( inAsset != nullptr && inNewMaxLOD <= TEXTURE_MAX_LODS, "[ResourceManager] Update texture was called with invalid parameters" );

		auto renderer			= Engine::GetRenderer();
		auto& textureHeader		= inAsset->GetHeader();
		auto id					= inAsset->GetIdentifier();

		if( textureHeader.LODs.size() <= inNewMaxLOD )
		{
			Console::WriteLine( "[Warning] ResourceManager: Failed to lower texture asset detail, the target detail level was invalid!" );
			return;
		}

		uint8 lodCount	= (uint8) ( textureHeader.LODs.size() - inNewMaxLOD );
		auto& maxLOD	= textureHeader.LODs.at( inNewMaxLOD );

		// Setup parameters to create the new texture
		TextureParameters params;

		params.bCPURead		= false;
		params.bDynamic		= false;
		params.bAutogenMips	= false;
		params.Format		= textureHeader.Format;
		params.Width		= maxLOD.Width;
		params.Height		= maxLOD.Height;
		params.BindTargets	= RENDERER_TEXTURE_BIND_FLAG_SHADER;
		params.AssetIdentifier = inAsset->GetIdentifier();

		if( IsRenderThread() )
		{
			auto entry = m_Textures.find( id );
			if( entry == m_Textures.end() || !entry->second.ptr || !entry->second.ptr->IsValid() )
			{
				// TODO: In this case, we need to force a texture load
				Console::WriteLine( "[ERROR] ResourceManager: Failed to lower texture detail level, the current texture was invalid!" );
				return;
			}
			else if( entry->second.type != TextureType::Tex2D )
			{
				Console::WriteLine( "[Warning] ResourceManager: Failed to lower texture detail level! The existing texture is not a 2D texture!" );
				return;
			}

			auto newTex = renderer->GetAPI()->CreateTexture2D( params );
			if( !newTex )
			{
				Console::WriteLine( "[Warning] ResourceManager: Failed to lower texture detail level! The api failed to create a new texture instance!" );
				return;
			}

			auto oldTex	= std::dynamic_pointer_cast<RTexture2D>( entry->second.ptr );
			HYPERION_VERIFY( oldTex != nullptr, "[ResourceManager] Failed to cast texture to proper type" );

			auto oldLODCount	= oldTex->GetMipCount();
			bool bFailed		= false;

			for( uint8 i = inNewMaxLOD; i < textureHeader.LODs.size(); i++ )
			{
				auto& lodInfo		= textureHeader.LODs.at( i );
				auto newTexIndex	= lodCount - ( textureHeader.LODs.size() - i );
				auto oldTexIndex	= oldLODCount - ( textureHeader.LODs.size() - i );

				if( !renderer->GetAPI()->CopyTexture2DMip( oldTex, newTex, (uint8) oldTexIndex, (uint8) newTexIndex ) )
				{
					Console::WriteLine( "[ERROR] ResourceManager: Attempt to lower texture LOD.. but the API copy call failed when copying to new texture!" );
					bFailed = true;
					break;
				}
			}

			if( !bFailed )
			{
				// Texture hotswap
				oldTex->Swap( *newTex );
				newTex->Shutdown();
				newTex.reset();
			}
		}
		else
		{
			// TODO: Can this be sped up with async texture creation??
			// Run this function on the render thread instead
			renderer->AddCommand( std::make_unique< RenderCommand >( std::bind( &ResourceManager::DecreaseTextureDetail, this, inAsset, inNewMaxLOD ) ) );
		}
	}


	void ResourceManager::RemoveTexture( const std::shared_ptr<TextureAsset>& inAsset )
	{
		HYPERION_VERIFY( inAsset != nullptr, "[ResourceManager] Remove texture was called with invalid arguments" );

		auto renderer = Engine::GetRenderer();

		if( IsRenderThread() )
		{
			auto id = inAsset->GetIdentifier();
			auto entry = m_Textures.find( id );

			if( entry != m_Textures.end() )
			{
				if( entry->second.ptr )
				{
					if( entry->second.ptr->IsValid() )
					{
						entry->second.ptr->Shutdown();
					}

					entry->second.ptr.reset();
				}

				m_Textures.erase( entry );
			}
		}
		else
		{
			renderer->AddCommand( std::make_unique< RenderCommand >( std::bind( &ResourceManager::RemoveTexture, this, inAsset ) ) );
		}
	}


}