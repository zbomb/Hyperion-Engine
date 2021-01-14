/*==================================================================================================
	Hyperion Engine
	Source/Streaming/BasicStreamingManager.cpp
	© 2021, Zachary Berry
==================================================================================================*/

#include "Hyperion/Streaming/BasicStreamingManager.h"
#include "Hyperion/Core/Engine.h"
#include "Hyperion/File/FileSystem.h"
#include "Hyperion/Renderer/Renderer.h"
#include "Hyperion/Renderer/Resource/ResourceManager.h"

#include <chrono>


namespace Hyperion
{

	void BasicStreamingManager::ReferenceTexture( std::shared_ptr< TextureAsset > inAsset )
	{
		auto newEntry = std::make_unique< TextureLoadEntry >();
		
		newEntry->bIncrementRefCount = true;
		newEntry->asset = inAsset;

		m_Queue.Push( std::move( newEntry ) );
	}


	void BasicStreamingManager::DereferenceTexture( std::shared_ptr< TextureAsset > inAsset )
	{
		auto newEntry = std::make_unique< TextureLoadEntry >();

		newEntry->bIncrementRefCount = false;
		newEntry->asset = inAsset;

		m_Queue.Push( std::move( newEntry ) );
	}


	void BasicStreamingManager::ReferenceStaticModel( std::shared_ptr< StaticModelAsset > inAsset )
	{
		auto newEntry = std::make_unique< StaticModelLoadEntry >();

		newEntry->bIncrementRefCount = true;
		newEntry->asset = inAsset;

		m_Queue.Push( std::move( newEntry ) );
	}


	void BasicStreamingManager::DereferenceStaticModel( std::shared_ptr< StaticModelAsset > inAsset )
	{
		auto newEntry = std::make_unique< StaticModelLoadEntry >();

		newEntry->bIncrementRefCount = false;
		newEntry->asset = inAsset;

		m_Queue.Push( std::move( newEntry ) );
	}


	void BasicStreamingManager::ReferenceDynamicModel( std::shared_ptr< DynamicModelAsset > inAsset )
	{
		auto newEntry = std::make_unique< DynamicModelLoadEntry >();

		newEntry->bIncrementRefCount = true;
		newEntry->asset = inAsset;

		m_Queue.Push( std::move( newEntry ) );
	}


	void BasicStreamingManager::DereferenceDynamicModel( std::shared_ptr< DynamicModelAsset > inAsset )
	{
		auto newEntry = std::make_unique< DynamicModelLoadEntry >();

		newEntry->bIncrementRefCount = false;
		newEntry->asset = inAsset;

		m_Queue.Push( std::move( newEntry ) );
	}


	void BasicStreamingManager::Initialize()
	{
		// We need to create the worker thread to process load/unload events
		HYPERION_VERIFY( !m_Thread.IsValid(), "[StreamingManager] Worker thread was already created on init?" );

		CustomThreadParameters params;

		params.AllowTasks			= false;
		params.Identifier			= "basic_streaming_manager";
		params.StartAutomatically	= true;
		params.ThreadFunction		= std::bind( &BasicStreamingManager::WorkerThreadBody, this, std::placeholders::_1 );

		m_Thread = ThreadManager::CreateThread( params );
		HYPERION_VERIFY( m_Thread.IsValid(), "[StreamingManager] Failed to create worker thread!" );
	}


	void BasicStreamingManager::Shutdown()
	{
		if( m_Thread.IsValid() )
		{
			if( m_Thread->IsRunning() )
			{
				m_Thread->Stop();
			}

			ThreadManager::DestroyThread( "basic_streaming_manager" );
			m_Thread.Clear();
		}
	}


	void BasicStreamingManager::WorkerThreadBody( CustomThread& inThread )
	{
		// Worker Loop
		while( inThread.IsRunning() )
		{
			auto it_start = std::chrono::high_resolution_clock::now();

			// Pop the next command in the loop
			auto entry = m_Queue.TryPopValue();
			if( entry.first && entry.second )
			{
				auto t = entry.second->GetType();

				if( t == 0 )
				{
					auto* texLoad = dynamic_cast<TextureLoadEntry*>( entry.second.get() );
					HYPERION_VERIFY( texLoad != nullptr && texLoad->asset, "[BasicStreamingManager] Invalid event in queue!" );

					auto tex_it = m_Textures.find( texLoad->asset->GetIdentifier() );

					if( texLoad->bIncrementRefCount )
					{
						if( tex_it == m_Textures.end() )
						{
							// Create Texture
							m_Textures.emplace( texLoad->asset->GetIdentifier(), Entry< TextureAsset >( texLoad->asset ) );
							LoadTexture( texLoad->asset );
						}
						else
						{
							tex_it->second.count++;
						}
					}
					else
					{
						if( tex_it == m_Textures.end() ) { continue; }

						auto c = --( tex_it->second.count );
						if( c == 0 )
						{
							// Destroy Texture
							std::shared_ptr< TextureAsset > asset = tex_it->second.asset;
							m_Textures.erase( tex_it );
							UnloadTexture( asset );
						}
					}
				}
				else if( t == 1 )
				{
					auto* smodelLoad = dynamic_cast<StaticModelLoadEntry*>( entry.second.get() );
					HYPERION_VERIFY( smodelLoad != nullptr && smodelLoad->asset, "[BasicStreamingManager] Invalid event in queue!" );

					auto smodel_it = m_StaticModels.find( smodelLoad->asset->GetIdentifier() );

					if( smodelLoad->bIncrementRefCount )
					{
						if( smodel_it == m_StaticModels.end() )
						{
							// Create Static Model
							m_StaticModels.emplace( smodelLoad->asset->GetIdentifier(), Entry< StaticModelAsset >( smodelLoad->asset ) );
							LoadStaticModel( smodelLoad->asset );
						}
						else
						{
							smodel_it->second.count++;
						}
					}
					else
					{
						if( smodel_it == m_StaticModels.end() ) { continue; }

						auto c = --( smodel_it->second.count );
						if( c == 0 )
						{
							// Destroy Static Model
							std::shared_ptr< StaticModelAsset > asset = smodel_it->second.asset;
							m_StaticModels.erase( smodel_it );
							UnloadStaticModel( asset );
						}
					}
				}
				else if( t == 2 )
				{
					auto* dmodelLoad = dynamic_cast<DynamicModelLoadEntry*>( entry.second.get() );
					HYPERION_VERIFY( dmodelLoad != nullptr && dmodelLoad->asset, "[BasicStreamingManager] Invalid event in queue!" );

					auto dmodel_it = m_DynamicModels.find( dmodelLoad->asset->GetIdentifier() );

					if( dmodelLoad->bIncrementRefCount )
					{
						if( dmodel_it == m_DynamicModels.end() )
						{
							// Create Dynamic Model
							m_DynamicModels.emplace( dmodelLoad->asset->GetIdentifier(), Entry< DynamicModelAsset >( dmodelLoad->asset ) );
							LoadDynamicModel( dmodelLoad->asset );
						}
						else
						{
							dmodel_it->second.count++;
						}
					}
					else
					{
						if( dmodel_it == m_DynamicModels.end() ) { continue; }

						auto c = --( dmodel_it->second.count );
						if( c == 0 )
						{
							// Destroy Dynamic Model
							std::shared_ptr< DynamicModelAsset > asset = dmodel_it->second.asset;
							m_DynamicModels.erase( dmodel_it );
							UnloadDynamicModel( asset );
						}
					}
				}
				else
				{
					HYPERION_VERIFY( true, "[BasicStreamingManager] Invalid event in queue!" );
				}
			}

			std::this_thread::sleep_until( it_start + std::chrono::milliseconds( 16 ) );
		}
	}


	void BasicStreamingManager::LoadTexture( const std::shared_ptr< TextureAsset >& inAsset )
	{
		HYPERION_VERIFY( inAsset != nullptr, "[BasicStreamingManager] Attempt to load/unload asset that is null!" );

		// We need to actually load the texture data from disk, and pass it off to the render thread
		auto path = inAsset->GetDiskPath();
		auto offset = inAsset->GetFileOffset();
		auto length = inAsset->GetFileLength();

		auto fHandle = FileSystem::OpenFile( path, FileMode::Read );
		if( !fHandle || !fHandle->IsValid() )
		{
			Console::WriteLine( "[Warning] StreamingManager: Failed to load texture \"", inAsset->GetPath(), "\" because the file is invalid/not found" );
			return;
		}

		if( fHandle->GetSize() < offset + length )
		{
			Console::WriteLine( "[Warning] StreamingManager: Failed to load texture \"", inAsset->GetPath(), "\" because the file isnt long enough" );
			return;
		}

		// Now that the file is open.. we need to actually read the LODs we need in
		// Lets use the header info to find the position in memory of the LODs
		auto& headerInfo = inAsset->GetHeader();
		std::vector< std::vector< byte > > lodData;

		{
			DataReader reader( fHandle );

			for( uint8 i = 0; i < (uint8) headerInfo.LODs.size(); i++ )
			{
				uint64 lodOffset = offset + headerInfo.LODs[ i ].FileOffset;
				uint32 lodLength = headerInfo.LODs[ i ].LODSize;

				HYPERION_VERIFY( length == 0 || ( lodLength <= offset + length ), "[StreamingManager] Failed to load LOD, texture data ran past end of file section" );

				std::vector< byte > data;
				reader.SeekOffset( lodOffset );

				if( reader.ReadBytes( data, lodLength ) != DataReader::ReadResult::Success )
				{
					Console::WriteLine( "[Warning] StreamingManager: Failed to load texture \"", inAsset->GetPath(), "\" because the file couldnt be read" );
					return;
				}

				lodData.emplace_back( std::move( data ) );
			}
		}

		// Now that the data is read in, lets send it over to the texture cache to be moved to VRAM
		Engine::GetRenderer()->GetResourceManager()->IncreaseTextureDetail( inAsset, 0, lodData );
	}


	void BasicStreamingManager::UnloadTexture( const std::shared_ptr< TextureAsset >& inAsset )
	{
		HYPERION_VERIFY( inAsset != nullptr, "[BasicStreamingManager] Attempt to load/unload asset that is null!" );
		Engine::GetRenderer()->GetResourceManager()->RemoveTexture( inAsset );
	}


	void BasicStreamingManager::LoadStaticModel( const std::shared_ptr< StaticModelAsset >& inAsset )
	{
		HYPERION_VERIFY( inAsset != nullptr, "[BasicStreamingManager] Attempt to load/unload asset that is null!" );

		auto fpath		= inAsset->GetDiskPath();
		auto offset		= inAsset->GetFileOffset();
		auto length		= inAsset->GetFileLength();

		auto fHandle = FileSystem::OpenFile( fpath, FileMode::Read );
		if( !fHandle || !fHandle->IsValid() )
		{
			Console::WriteLine( "[Warning] StreamingManager: Failed to load model \"", inAsset->GetPath(), "\" because the file couldnt be opened" );
			return;
		}

		if( fHandle->GetSize() < offset + length )
		{
			Console::WriteLine( "[Warning] StreamingManager: Failed to load model \"", inAsset->GetPath(), "\" because the file isnt long enough" );
			return;
		}

		// Read in all LODs
		std::vector< std::vector< std::vector< byte > > > indexData;
		std::vector< std::vector< std::vector< byte > > > vertexData;
		uint8 index = 0;

		{
			DataReader reader( fHandle );

			for( auto it = inAsset->LODBegin(); it != inAsset->LODEnd(); it++ )
			{
				if( it->SubObjects.size() == 0 )
				{
					Console::WriteLine( "[Warning] StreamingManager: Invalid LOD in model \"", inAsset->GetPath(), "\" because there are no subobjects?" );
					continue;
				}

				auto& indexList = indexData.emplace_back( std::vector< std::vector< byte > >() );
				auto& vertexList = vertexData.emplace_back( std::vector< std::vector< byte > >() );

				for( auto sit = it->SubObjects.begin(); sit != it->SubObjects.end(); sit++ )
				{
					auto& di = indexList.emplace_back( std::vector< byte >() );
					auto& vi = vertexList.emplace_back( std::vector< byte >() );

					reader.SeekOffset( offset + sit->IndexOffset );
					if( reader.ReadBytes( di, sit->IndexLength ) != DataReader::ReadResult::Success )
					{
						Console::WriteLine( "[Warning] StreamingManager: Failed to load LOD in \"", inAsset->GetPath(), "\" because the index data couldnt be read from file" );
						return;
					}

					reader.SeekOffset( offset + sit->VertexOffset );
					if( reader.ReadBytes( vi, sit->VertexLength ) != DataReader::ReadResult::Success )
					{
						Console::WriteLine( "[Warning] StreamingManager: Failed to laod LOD in \"", inAsset->GetPath(), "\" because the vertex data couldnt be read from file" );
						return;
					}

				}

				index++;
			}
		}

		// Now, upload data to VRAM
		Engine::GetRenderer()->GetResourceManager()->UploadFullGeometry( inAsset, vertexData, indexData );
	}


	void BasicStreamingManager::UnloadStaticModel( const std::shared_ptr< StaticModelAsset >& inAsset )
	{
		HYPERION_VERIFY( inAsset != nullptr, "[BasicStreamingManager] Attempt to load/unload asset that is null!" );
		Engine::GetRenderer()->GetResourceManager()->RemoveFullGeometry( inAsset );
	}


	void BasicStreamingManager::LoadDynamicModel( const std::shared_ptr< DynamicModelAsset >& inAsset )
	{
		HYPERION_VERIFY( inAsset != nullptr, "[BasicStreamingManager] Attempt to load/unload asset that is null!" );
		HYPERION_NOT_IMPLEMENTED( "Dynamic Models arent implemented yet!" );
	}


	void BasicStreamingManager::UnloadDynamicModel( const std::shared_ptr< DynamicModelAsset >& inAsset )
	{
		HYPERION_VERIFY( inAsset != nullptr, "[BasicStreamingManager] Attempt to load/unload asset that is null!" );
		HYPERION_NOT_IMPLEMENTED( "Dynamic Models arent implemented yet!" );
	}


	void BasicStreamingManager::Reset()
	{
		Engine::GetRenderer()->GetResourceManager()->ClearGeometry();
		Engine::GetRenderer()->GetResourceManager()->ClearTextures();
	}
}