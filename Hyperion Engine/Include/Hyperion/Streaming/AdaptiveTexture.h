/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/AdaptiveTexture.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Streaming/AdaptiveBase.h"


namespace Hyperion
{
	/*
	struct AdaptiveTextureLoadRequest;


	class AdaptiveTexture : public AdaptiveBase
	{

	public:

		AssetRef< TextureAsset > m_Asset;

		uint8 m_TargetLOD;
		uint8 m_ActiveLOD;
		uint8 m_PendingLOD;
		uint8 m_MinLOD;

		float m_LODBias;
		float m_Priority;

		uint32 m_ForceDropMemorySavings; // TODO: Ensure this is updated each tick

		std::weak_ptr< AdaptiveTextureRequestBase > m_ActiveRequest;

		AdaptiveTexture()
			: m_TargetLOD( 255 ), m_LODBias( 0.5f ), m_ActiveLOD( 255 ), m_PendingLOD( 255 ), m_Priority( 1.f )
		{
		}
	};
	*/

	/*	
		AdaptiveTextureLoadRequest
		* For now, the way we load an LOD for textures is by actually loading all of the LODs up
		to the desired one, and just creating a brand new texture, were only going to do this a few times
		a frame, and its in the update phase before the frame is drawn anyway
	*/
	struct AdaptiveTextureRequestBase
	{

	protected:

		std::shared_ptr< AdaptiveTexture > m_Target;
		uint32 m_Memory;

		uint8 m_Level;
		bool m_Valid;

		AdaptiveTextureRequestBase( const std::shared_ptr< AdaptiveTexture >& inTarget, uint8 inLevel );

	public:

		AdaptiveTextureRequestBase() = delete;

		inline std::shared_ptr< AdaptiveTexture > GetTarget() const { return m_Target; }
		inline uint32 GetMemory() const { return m_Memory; }
		inline uint8 GetLevel() const { return m_Level; }
		inline bool IsValid() const
		{
			return m_Target && m_Valid && m_Target->GetAsset();
		}

		void Cancel();
	};

	struct AdaptiveTextureLoadRequest : public AdaptiveTextureRequestBase
	{

	protected:

		float m_Priority;

	public:

		AdaptiveTextureLoadRequest( const std::shared_ptr< AdaptiveTexture >& inTarget, uint8 inLevel );
		~AdaptiveTextureLoadRequest();

		AdaptiveTextureLoadRequest() = delete;

		inline float GetPriority() const { return m_Priority; }
		void UpdatePriority();

		void Reset( uint8 inLevel, uint32 inMemory, float inPriority );

	};

	struct AdaptiveTextureUnloadRequest : public AdaptiveTextureRequestBase
	{

	public:

		AdaptiveTextureUnloadRequest( const std::shared_ptr< AdaptiveTexture >& inTarget, uint8 inLevel );
		~AdaptiveTextureUnloadRequest();

		AdaptiveTextureUnloadRequest() = delete;

	};


	/*------------------------------------------------------------------------------------------
		AdaptiveTexture
		- Representation of a texture within the Adaptive Asset system
	-----------------------------------------------------------------------------------------*/
	class AdaptiveTexture : public AdaptiveBase
	{

	private:

		uint8 m_ActiveLOD;
		uint8 m_TargetLOD;
		uint8 m_PendingLOD;
		uint8 m_MinimumLOD;

		float m_Priority;

		std::weak_ptr< AdaptiveTextureRequestBase > m_PendingRequest;
		AssetRef< TextureAsset > m_Asset;

	public:

		AdaptiveTexture( const AssetRef< TextureAsset >& inAsset );
		~AdaptiveTexture();

		AdaptiveTexture() = delete;

		inline AssetRef< TextureAsset >& GetAsset() { return m_Asset; }
		inline auto GetActiveLevel() const { return m_ActiveLOD; }
		inline auto GetTargetLevel() const { return m_TargetLOD; }
		inline auto GetPendingLevel() const { return m_PendingLOD; }
		inline auto GetMinimumLevel() const { return m_PendingLOD; }
		inline auto GetPriority() const { return m_Priority; }

		bool UpdateTargetLOD();
		bool CancelPendingRequest();

		std::shared_ptr< AdaptiveTextureLoadRequest > GenerateLoadRequest();
		std::shared_ptr< AdaptiveTextureUnloadRequest > GenerateDropRequest();

		void UpdatePriority();
		float CalculatePriority( uint8 atLevel );

		uint32 GetTopLevelMemoryUsage();
		void PerformDrop( uint8 inLevel );
	};

}