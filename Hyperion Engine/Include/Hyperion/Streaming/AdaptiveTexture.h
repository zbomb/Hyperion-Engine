/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/AdaptiveTexture.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Streaming/AdaptiveBase.h"
#include "Hyperion/Assets/TextureAsset.h"


namespace Hyperion
{
	// Forward Declarations
	struct AdaptiveTextureRequestBase;
	struct AdaptiveTextureLoadRequest;
	struct AdaptiveTextureUnloadRequest;


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
		std::shared_ptr< TextureAsset > m_Asset;

	public:

		AdaptiveTexture( const std::shared_ptr< TextureAsset >& inAsset );
		~AdaptiveTexture();

		AdaptiveTexture() = delete;

		inline std::shared_ptr< TextureAsset >& GetAsset() { return m_Asset; }
		inline auto GetActiveLevel() const { return m_ActiveLOD; }
		inline auto GetTargetLevel() const { return m_TargetLOD; }
		inline auto GetPendingLevel() const { return m_PendingLOD; }
		inline auto GetMinimumLevel() const { return m_PendingLOD; }
		inline auto GetPriority() const { return m_Priority; }

		bool UpdateTargetLOD();
		bool CancelPendingRequest( bool bForce = false );

		std::shared_ptr< AdaptiveTextureLoadRequest > GenerateLoadRequest( const std::shared_ptr< AdaptiveTexture >& inPtr );
		std::shared_ptr< AdaptiveTextureUnloadRequest > GenerateDropRequest( const std::shared_ptr< AdaptiveTexture >& inPtr );

		void UpdatePriority();
		float CalculatePriority( uint8 atLevel );

		uint32 GetTopLevelMemoryUsage();

		void PerformDrop( uint8 inLevel );
		void PerformIncrease( uint8 inLevel );
	};


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

}