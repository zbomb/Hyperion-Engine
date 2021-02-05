/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/Renderer.h
	� 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"
#include "Hyperion/Renderer/IGraphics.h"
#include "Hyperion/Renderer/DataTypes.h"
#include "Hyperion/Core/Types/ConcurrentQueue.h"
#include "Hyperion/Console/ConsoleVar.h"
#include "Hyperion/Library/Geometry.h"
#include "Hyperion/Streaming/BasicStreamingManager.h"
#include "Hyperion/Renderer/ResourceManager.h"
#include "Hyperion/Renderer/BatchCollector.h"
#include "Hyperion/Renderer/Resources/RShader.h"

#include <atomic>


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
	class TextureAsset;
	class RenderPipeline;

	/*
		Renderer Instance
	*/
	class Renderer
	{

	protected:

		/*
		*	Resources
		*/
		std::shared_ptr< IGraphics > m_API;
		std::shared_ptr< ProxyScene > m_Scene;
		std::shared_ptr< ResourceManager > m_ResourceManager;
		HypPtr< BasicStreamingManager > m_StreamingManager;

		std::shared_ptr< RViewClusters > m_ViewClusters;
		std::shared_ptr< RLightBuffer > m_LightBuffer;
		std::shared_ptr< GBuffer > m_GBuffer;
		
		/*
		*	Command Members
		*/
		ConcurrentQueue< std::unique_ptr< RenderCommandBase > > m_ImmediateCommands;
		ConcurrentQueue< std::unique_ptr< RenderCommandBase > > m_Commands;
		std::atomic< bool > m_AllowCommands;

		/*
		*	Resolution Members
		*/
		ScreenResolution m_Resolution;
		std::atomic< ScreenResolution > m_CachedResolution;
		std::atomic< bool > m_bCachedVSync;
		std::vector< ScreenResolution > m_AvailableResolutions;
		std::atomic< bool > m_bCanChangeResolution;

		/*
		*	Other Members
		*/
		bool m_bVSync;
		void* m_pWindow;
		GraphicsAPI m_APIType;
		Matrix m_ViewMatrix, m_ProjectionMatrix, m_OrthoMatrix, m_ScreenViewMatrix;
		ViewState m_ViewState;
		Color3F m_AmbientLightColor;
		float m_AmbientLightIntensity;

		std::shared_ptr< RenderPipeline > m_AttachedPipeline;
		std::shared_ptr< RComputeShader > m_BuildClustersShader;
		std::shared_ptr< RComputeShader > m_FindClustersShader;
		std::shared_ptr< RComputeShader > m_CullLightsShader;

		/*
		*	Pure Virtual Functions
		*/
		virtual void RenderScene() = 0;
		virtual void OnResolutionChanged( const ScreenResolution& inRes ) = 0;

	public:

		/*
		*	Type Aliases
		*/
		using DebugBatch = std::tuple< Transform, std::shared_ptr< RBuffer >, std::shared_ptr< RBuffer >, std::shared_ptr< RMaterial > >;

		/*
		*	Constructors/Destructor
		*/
		Renderer() = delete;
		Renderer( GraphicsAPI inAPI, void* inWindow, const ScreenResolution& inRes, bool bVSync );
		virtual ~Renderer();

		Renderer( const Renderer& ) = delete;
		Renderer( Renderer&& ) = delete;

		/*
		*	Assignment Operators
		*/
		Renderer& operator=( const Renderer& ) = delete;
		Renderer& operator=( Renderer&& ) = delete;

		/*
		*	Getters
		*/
		inline std::shared_ptr< IGraphics > GetAPI() const						{ return m_API; }
		inline GraphicsAPI GetAPIType() const									{ return m_APIType; }
		inline ScreenResolution GetResolutionSafe() const						{ return m_CachedResolution.load(); }
		inline bool IsVSyncOnSafe() const										{ return m_bCachedVSync.load(); }
		inline HypPtr< BasicStreamingManager > GetStreamingManager() const		{ return m_StreamingManager; }
		inline std::shared_ptr< ResourceManager > GetResourceManager() const	{ return m_ResourceManager; }
		inline std::shared_ptr< ProxyScene > GetScene() const					{ return m_Scene; }
		inline Matrix GetViewMatrix() const										{ return m_ViewMatrix; }
		inline Matrix GetProjectionMatrix() const								{ return m_ProjectionMatrix; }
		inline Matrix GetOrthoMatrix() const									{ return m_OrthoMatrix; }
		inline Matrix GetScreenViewMatrix() const								{ return m_ScreenViewMatrix; }
		inline ViewState GetViewState() const									{ return m_ViewState; }
		inline Color3F GetAmbientLightColor() const								{ return m_AmbientLightColor; }
		inline float GetAmbientLightIntensity() const							{ return m_AmbientLightIntensity; }
		inline std::shared_ptr< RLightBuffer > GetLightBuffer() const			{ return m_LightBuffer; }
		inline std::shared_ptr< RViewClusters > GetViewClusters() const			{ return m_ViewClusters; }
		inline std::shared_ptr< GBuffer > GetGBuffer() const					{ return m_GBuffer; }

		// These two functions can only be called from the render thread
		inline bool IsVSyncOnUnsafe() const										{ return m_bVSync; }
		inline ScreenResolution GetResolutionUnsafe() const						{ return m_Resolution; }

		/*
		*	Proxy Scene Managment
		*/
		bool AddPrimitive( std::shared_ptr< ProxyPrimitive >& );
		bool AddLight( std::shared_ptr< ProxyLight >& );
		bool RemovePrimitive( uint32 inIdentifier );
		bool RemoveLight( uint32 inIdentifier );
		void ShutdownProxy( const std::shared_ptr< ProxyBase >& );

		/*
		*	Commands
		*/
		void AddImmediateCommand( std::unique_ptr< RenderCommandBase >&& inCommand );
		void AddCommand( std::unique_ptr< RenderCommandBase >&& inCommand );

		/*
		*	Render Pipelines
		*/
		bool AttachPipeline( const std::shared_ptr< RenderPipeline >& inPipeline );
		inline std::shared_ptr< RenderPipeline > GetAttachedPipeline() const { return m_AttachedPipeline; }
		void DetachPipeline();

		uint32 DispatchPipeline( uint32 inFlags = 0 );
		uint32 DispatchPipeline( const BatchCollector& inBatches, uint32 inFlags = 0 );

		void CollectBatches( BatchCollector& outBatches, uint32 inFlags );

		/*
		*	Other Member Functions
		*/
		virtual bool Initialize();
		virtual void Shutdown();

		void Frame();
		void UpdateScene();

		void GetViewState( ViewState& outState ) const;
		bool ChangeResolution( const ScreenResolution& inRes );

		bool RebuildLightBuffer();
		bool RebuildViewClusters();
		bool AreViewClustersDirty() const;
		bool ResetViewClusters();

		bool DispatchComputeShader( const std::shared_ptr< RComputeShader >& inShader, uint32 inFlags = 0 );

		void SetGBuffer( const std::shared_ptr< GBuffer >& inBuffer );

	};

}