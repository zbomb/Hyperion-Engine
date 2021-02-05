/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Renderer/RenderPipeline.h
	© 2021, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"


namespace Hyperion
{
	// Forward Decl.
	class RVertexShader;
	class RGeometryShader;
	class RPixelShader;


	class RenderPipeline
	{

	private:

		std::shared_ptr< RVertexShader > m_VertexShader;
		std::shared_ptr< RGeometryShader > m_GeometryShader;
		std::shared_ptr< RPixelShader > m_PixelShader;

		uint32 m_CollectionFlags;
		GeometryCollectionSource m_CollectionSource;
		PipelineRenderTarget m_RenderTarget;
		PipelineDepthStencilTarget m_StencilTarget;

		bool m_bUseViewClusters;
		bool m_bUseGBuffer;
		bool m_bUseLightBuffer;
		bool m_bUseZBuffer;
		bool m_bAlphaBlending;
		bool m_bClearRenderTargets;
		bool m_bClearDepthBuffers;

	public:

		RenderPipeline();
		~RenderPipeline();

		bool IsValid() const;

		bool AttachVertexShader( const std::shared_ptr< RVertexShader >& inShader );
		bool AttachGeometryShader( const std::shared_ptr< RGeometryShader >& inShader );
		bool AttachPixelShader( const std::shared_ptr< RPixelShader >& inShader );

		inline std::shared_ptr< RVertexShader > GetVertexShader() const			{ return m_VertexShader; }
		inline std::shared_ptr< RGeometryShader > GetGeometryShader() const		{ return m_GeometryShader; }
		inline std::shared_ptr< RPixelShader > GetPixelShader() const			{ return m_PixelShader; }

		inline uint32 GetCollectionFlags() const						{ return m_CollectionFlags; }
		inline GeometryCollectionSource GetCollectionSource() const		{ return m_CollectionSource; }
		inline PipelineRenderTarget GetRenderTarget() const				{ return m_RenderTarget; }
		inline PipelineDepthStencilTarget GetDepthStencilTarget() const { return m_StencilTarget; }
		inline bool IsViewClustersEnabled() const						{ return m_bUseViewClusters; }
		inline bool IsGBufferEnabled() const							{ return m_bUseGBuffer; }
		inline bool IsLightBufferEnabled() const						{ return m_bUseLightBuffer; }
		inline bool IsZBufferEnabled() const							{ return m_bUseZBuffer; }
		inline bool IsAlphaBlendingEnabled() const						{ return m_bAlphaBlending; }
		inline bool IsRenderTargetClearingEnabled() const				{ return m_bClearRenderTargets; }
		inline bool IsDepthBufferClearingEnabled() const				{ return m_bClearDepthBuffers; }

		void SetCollectionFlags( uint32 inFlags );
		void SetCollectionSource( GeometryCollectionSource inSource );
		void SetRenderTarget( PipelineRenderTarget inTarget );
		void SetDepthStencilTarget( PipelineDepthStencilTarget inTarget );

		inline void EnableViewClusters()			{ m_bUseViewClusters = true; }
		inline void DisableViewClusters()			{ m_bUseViewClusters = false; }
		inline void EnableGBuffer()					{ m_bUseGBuffer = true; }
		inline void DisableGBuffer()				{ m_bUseGBuffer = false; }
		inline void EnableLightBuffer()				{ m_bUseLightBuffer = true; }
		inline void DisableLightBuffer()			{ m_bUseLightBuffer = false; }
		inline void EnableZBuffer()					{ m_bUseZBuffer = true; }
		inline void DisableZBuffer()				{ m_bUseZBuffer = false; }
		inline void EnableAlphaBlending()			{ m_bAlphaBlending = true; }
		inline void DisableAlphaBlending()			{ m_bAlphaBlending = false; }
		inline void EnableRenderTargetClearing()	{ m_bClearRenderTargets = true; }
		inline void DisableRenderTargetClearing()	{ m_bClearRenderTargets = false; }
		inline void EnableDepthBufferClearing()		{ m_bClearDepthBuffers = true; }
		inline void DisableDepthBufferClearing()	{ m_bClearDepthBuffers = false; }

	};

}