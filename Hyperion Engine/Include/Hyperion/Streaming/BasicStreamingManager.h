/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/BasicStreamingManager.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Assets/StaticModelAsset.h"
#include "Hyperion/Assets/DynamicModelAsset.h"
#include "Hyperion/Core/ThreadManager.h"
#include "Hyperion/Core/Types/ConcurrentQueue.h"



namespace Hyperion
{

	class BasicStreamingManager : public Object
	{

	public:

		struct LoadEntryBase
		{
			bool bIncrementRefCount;
			virtual int GetType() = 0;
		};

	private:

		template< typename _Ty >
		struct Entry
		{
			uint32 count;
			std::shared_ptr< _Ty > asset;

			Entry( const std::shared_ptr< _Ty > inAsset )
				: count( 1 ), asset( inAsset )
			{}
		};

		struct TextureLoadEntry : public LoadEntryBase
		{
			std::shared_ptr< TextureAsset > asset;

			int GetType() override { return 0; }
		};

		struct StaticModelLoadEntry : public LoadEntryBase
		{
			std::shared_ptr< StaticModelAsset > asset;

			int GetType() override { return 1; }
		};

		struct DynamicModelLoadEntry : public LoadEntryBase
		{
			std::shared_ptr< DynamicModelAsset > asset;

			int GetType() override { return 2; }
		};

		std::map< uint32, Entry< TextureAsset > > m_Textures;
		std::map< uint32, Entry< StaticModelAsset > > m_StaticModels;
		std::map< uint32, Entry< DynamicModelAsset > > m_DynamicModels;

		HypPtr< Thread > m_Thread;

		ConcurrentQueue< std::unique_ptr< LoadEntryBase > > m_Queue;

		void WorkerThreadBody( CustomThread& );

		void LoadTexture( const std::shared_ptr< TextureAsset >& inAsset );
		void UnloadTexture( const std::shared_ptr< TextureAsset >& inAsset );

		void LoadStaticModel( const std::shared_ptr< StaticModelAsset >& inAsset );
		void UnloadStaticModel( const std::shared_ptr< StaticModelAsset >& inAsset );

		void LoadDynamicModel( const std::shared_ptr< DynamicModelAsset >& inAsset );
		void UnloadDynamicModel( const std::shared_ptr< DynamicModelAsset >& inAsset );

	public:

		void Initialize() final;
		void Shutdown() final;

		void ReferenceTexture( std::shared_ptr< TextureAsset > inAsset );
		void DereferenceTexture( std::shared_ptr< TextureAsset > inAsset );

		void ReferenceStaticModel( std::shared_ptr< StaticModelAsset > inAsset );
		void DereferenceStaticModel( std::shared_ptr< StaticModelAsset > inAsset );

		void ReferenceDynamicModel( std::shared_ptr< DynamicModelAsset > inAsset );
		void DereferenceDynamicModel( std::shared_ptr< DynamicModelAsset > inAsset );

		void Reset();

	};

}