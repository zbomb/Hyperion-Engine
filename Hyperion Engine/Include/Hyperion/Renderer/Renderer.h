/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Renderer.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"
#include "Hyperion/Renderer/IGraphics.h"
#include "Hyperion/Renderer/DataTypes.h"
#include "Hyperion/Core/Types/ConcurrentQueue.h"
#include "Hyperion/Console/ConsoleVar.h"
#include "Hyperion/Core/Types/Transform.h"


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class ProxyScene;
	class ProxyPrimitive;
	class ProxyLight;
	class ProxyCamera;
	class ProxyBase;
	
	template< typename T >
	class AssetRef;

	class TextureAsset;
	struct RawImageData;


	/*
		Renderer Instance
	*/
	class Renderer
	{

	protected:

		ScreenResolution m_Resolution;
		bool m_bVSync;
		IRenderOutput m_Output;

		std::shared_ptr< IGraphics > m_API;
		std::shared_ptr< ProxyScene > m_Scene;

		ConcurrentQueue< std::unique_ptr< RenderCommandBase > > m_ImmediateCommands;
		ConcurrentQueue< std::unique_ptr< RenderCommandBase > > m_Commands;

		std::map< uint32, std::shared_ptr< ITexture2D > > m_TextureCache;

		bool Initialize();
		void Shutdown();
		void Frame();

		void UpdateScene();

		void ShutdownProxy( const std::shared_ptr< ProxyBase >& );

		Transform3D GetViewTransform();

	public:

		Renderer() = delete;
		Renderer( std::shared_ptr< IGraphics >& inAPI, const IRenderOutput& inOutput, const ScreenResolution& inResolution, bool bVSync );
		~Renderer();

		Renderer( const Renderer& ) = delete;
		Renderer( Renderer&& ) = delete;

		Renderer& operator=( const Renderer& ) = delete;
		Renderer& operator=( Renderer&& ) = delete;

		void AddImmediateCommand( std::unique_ptr< RenderCommandBase >&& inCommand );
		void AddCommand( std::unique_ptr< RenderCommandBase >&& inCommand );

		bool AddPrimitive( std::shared_ptr< ProxyPrimitive >& );
		bool AddLight( std::shared_ptr< ProxyLight >& );
		bool AddCamera( std::shared_ptr< ProxyCamera >& );

		bool RemovePrimitive( uint32 inIdentifier );
		bool RemoveLight( uint32 inIdentifier );
		bool RemoveCamera( uint32 inIdentifier );

		inline std::shared_ptr< ProxyScene > GetScene() const { return m_Scene; }

		bool IncreaseTextureAssetLOD( std::shared_ptr< TextureAsset >& inAsset, uint8 inMaxLevel, const std::vector< std::vector< byte > >& inData );
		bool LowerTextureAssetLOD( std::shared_ptr< TextureAsset >& inAsset, uint8 inMaxLevel );
		void RemoveTextureAsset( uint32 inIdentifier );
		void ClearTextureAssetCache();

		friend class RenderManager;

	};

}