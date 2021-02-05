/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Resource/ResourceManager.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Renderer/Resources/RMesh.h"
#include "Hyperion/Renderer/Resources/RMaterial.h"
#include "Hyperion/Assets/MaterialAsset.h"
#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Assets/StaticModelAsset.h"


namespace Hyperion
{

	class ResourceManager
	{

	private:

		enum class TextureType
		{
			None,
			Tex1D,
			Tex2D,
			Tex3D
		};

		struct TextureEntry
		{
			TextureType type;
			std::shared_ptr< RTextureBase > ptr;

			TextureEntry( TextureType inT, const std::shared_ptr< RTextureBase >& inP )
				: type( inT ), ptr( inP )
			{}

			~TextureEntry()
			{
				ptr.reset();
			}
		};


		std::map< uint32, std::shared_ptr< RMesh > > m_Geometry;
		std::map< uint32, TextureEntry > m_Textures;


		std::shared_ptr< RMeshData > GetMeshData( uint32 inIdentifier );

	public:

		ResourceManager();
		~ResourceManager();

		void Shutdown();

		/*
		*	RMaterial Creation
		*/
		std::shared_ptr< RMaterial > CreateMaterial( const std::shared_ptr< MaterialAsset >& inAsset );

		/*
		*	Geometry Cache
		*/
		bool IsMeshLODCached( uint32 inIdentifier, uint8 inLOD );
		bool IsMeshFullyCached( uint32 inIdentifier );
		bool IsMeshPartiallyCached( uint32 inIdentifier );

		bool UploadMeshLOD( const std::shared_ptr< StaticModelAsset >& inAsset, uint8 inLOD, 
								const std::vector< std::vector< byte > >& inVertexData, 
								const std::vector< std::vector< byte > >& inIndexData );
		bool RemoveMeshLOD( const std::shared_ptr< StaticModelAsset >& inAsset, uint8 inLOD );
		bool UploadFullMesh( const std::shared_ptr< StaticModelAsset >& inAsset, 
								 const std::vector< std::vector< std::vector< byte > > >& inVertexData, 
								 const std::vector< std::vector< std::vector< byte > > >& inIndexData );
		bool RemoveFullMesh( const std::shared_ptr< StaticModelAsset >& inAsset );

		std::shared_ptr< RMesh > GetMesh( const std::shared_ptr< StaticModelAsset >& inAsset );
		void ClearMeshes();

		/*
		*	Texture Cache
		*/
		std::shared_ptr< RTexture1D > Get1DTexture( uint32 inIdentifier );
		std::shared_ptr< RTexture2D > Get2DTexture( uint32 inIdentifier );
		std::shared_ptr< RTexture3D > Get3DTexture( uint32 inIdentifier );
		std::shared_ptr< RTextureBase > GetTexture( uint32 inIdentifier );

		void ClearTextures();
		void IncreaseTextureDetail( const std::shared_ptr< TextureAsset >& inAsset, uint8 inNewMaxLOD, const std::vector< std::vector< byte > >& inData );
		void DecreaseTextureDetail( const std::shared_ptr< TextureAsset >& inAsset, uint8 inNewMaxLOD );
		void RemoveTexture( const std::shared_ptr< TextureAsset >& inAsset );


	};

}