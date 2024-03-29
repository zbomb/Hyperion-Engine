/*==================================================================================================
	Hyperion Engine
	Source/Streaming/AdaptiveTexture.cpp
	� 2019, Zachary Berry
==================================================================================================*/


#include "Hyperion/Streaming/AdaptiveTexture.h"
#include "Hyperion/Streaming/AdaptiveAssetManager.h" // For console vars
#include "Hyperion/Library/Math.h"


namespace Hyperion
{

	AdaptiveTexture::AdaptiveTexture( const std::shared_ptr< TextureAsset >& inAsset )
		: m_Asset( inAsset ), m_ActiveLOD( 255 ), m_PendingLOD( 255 ), m_MinimumLOD( 255 ), m_Priority( 1.f )
	{
		if( !inAsset )
		{
			Console::WriteLine( "[ERROR] AdaptiveAssetManager: Attempt to create an adaptive texture with an invalid texture asset!" );
		}
		else
		{
			m_Identifier = inAsset->GetIdentifier();

			// We want to calculate the minimum LOD
			uint32 maxResidentMemory	= g_CVar_TextureMaxResidentMemory.GetValue();
			auto& textureHeader			= inAsset->GetHeader();

			uint32 memoryCounter	= 0;
			uint8 minLevel			= 255;

			for( uint8 i = (uint8)textureHeader.LODs.size() - 1; i >= 0; i-- ) // Loop through LODs from smallest to largest
			{
				memoryCounter += textureHeader.LODs.at( i ).LODSize;
				if( memoryCounter <= maxResidentMemory )
				{
					minLevel = i;
				}
				else break;
			}

			m_MinimumLOD = minLevel;
		}

		m_ActiveMemory		= 0;
		m_Updating			= false;
		m_MaxScreenSize		= 0.f;
		m_TotalScreenSize	= 0.f;
		m_ObjectUsage		= 0.f;
		m_Priority_Mult		= 0.f;
	}


	AdaptiveTexture::~AdaptiveTexture()
	{

	}


	bool AdaptiveTexture::UpdateTargetLOD()
	{
		if( m_Updating || !m_Asset )
		{
			return false;
		}

		auto& list = m_Asset->GetHeader().LODs;
		if( list.size() == 0 )
		{
			return false;
		}

		uint8 targetLevel = 255;
		uint8 i = (uint8) list.size() - 1;

		while( true )
		{
			auto& lod		= list.at( i );
			float lodSize	= Math::Max( lod.Width, lod.Height );

			if( i == 0 || lodSize > m_MaxScreenSize )
			{
				targetLevel = i;
				break;
			}

			i--;
		}

		m_TargetLOD = Math::Min( targetLevel, m_MinimumLOD );

		// Check if this invalidates the current request
		if( m_TargetLOD != m_PendingLOD )
		{
			CancelPendingRequest();
		}

		return true;
	}


	bool AdaptiveTexture::CancelPendingRequest( bool bForce /* = false */ )
	{
		if( !bForce && m_Updating ) { return false; }

		m_PendingLOD = m_ActiveLOD;
		auto req = m_PendingRequest.lock();

		if( req )
		{
			req->Cancel();
		}

		m_PendingRequest.reset();
		return true;
	}


	std::shared_ptr< AdaptiveTextureLoadRequest > AdaptiveTexture::GenerateLoadRequest( const std::shared_ptr< AdaptiveTexture >& inPtr )
	{
		if( m_Updating ) { return nullptr; }

		if( m_TargetLOD < m_ActiveLOD )
		{
			auto newRequest = std::make_shared< AdaptiveTextureLoadRequest >( inPtr, m_TargetLOD );
			
			m_PendingLOD		= m_TargetLOD;
			m_PendingRequest	= std::weak_ptr( newRequest );

			return newRequest;
		}

		return nullptr;
	}


	std::shared_ptr< AdaptiveTextureUnloadRequest > AdaptiveTexture::GenerateDropRequest( const std::shared_ptr< AdaptiveTexture >& inPtr )
	{
		if( m_Updating ) { return nullptr; }

		if( m_TargetLOD > m_ActiveLOD )
		{
			auto newRequest = std::make_shared< AdaptiveTextureUnloadRequest >( inPtr, m_TargetLOD );

			m_PendingLOD		= m_TargetLOD;
			m_PendingRequest	= std::weak_ptr( newRequest );

			return newRequest;
		}

		return nullptr;
	}


	void AdaptiveTexture::UpdatePriority()
	{
		m_Priority = CalculatePriority( m_ActiveLOD );
	}


	float AdaptiveTexture::CalculatePriority( uint8 atLevel )
	{
		// First, we need to calculate the LOD bias
		float lodBias = 0.5f;
		if( m_Asset )
		{
			auto& list = m_Asset->GetHeader().LODs;
			if( atLevel < list.size() )
			{
				auto& targetLOD = list.at( atLevel );
				float size = Math::Max( targetLOD.Width, targetLOD.Height );

				lodBias = ( ( ( size - m_MaxScreenSize ) / Math::Max( size, m_MaxScreenSize ) ) + 1.f ) / 2.f;
			}
		}

		// Next, we need to calculate max screen size factor
		static const float maxScreenSizeUpperBounds = 16384.f; // Upper bounds clamped at 16k
		float maxSizeFactor = Math::Max( maxScreenSizeUpperBounds, m_MaxScreenSize ) / maxScreenSizeUpperBounds;

		// Calculate total scene size factor
		float totalSizeFactor = ( Math::Max( maxScreenSizeUpperBounds, ( m_TotalScreenSize / (float) m_Refs.size() ) ) / maxScreenSizeUpperBounds ) * Math::Max( 1.f, m_ObjectUsage );

		return( ( ( maxSizeFactor * 0.65f ) + ( totalSizeFactor * 0.35f ) ) * lodBias * m_Priority_Mult );
	}


	uint32 AdaptiveTexture::GetTopLevelMemoryUsage()
	{
		if( m_Asset )
		{
			auto& list = m_Asset->GetHeader().LODs;
			if( m_ActiveLOD < list.size() )
			{
				return list.at( m_ActiveLOD ).LODSize;
			}
		}

		return 0;
	}


	void AdaptiveTexture::PerformDrop( uint8 inLevel )
	{
		if( !m_Asset )
		{
			Console::WriteLine( "[ERROR] AdaptiveAssetManager: Failed to update texture data structure during a drop! Asset became invalid??" );
			return;
		}

		auto& lodList = m_Asset->GetHeader().LODs;
		if( inLevel >= lodList.size() || inLevel > m_MinimumLOD )
		{
			Console::WriteLine( "[ERROR] AdaptiveAssetManager: Failed to update texture data structure during a drop! Invalid LOD level??" );
			return;
		}

		m_ActiveLOD		= inLevel;
		m_PendingLOD	= inLevel;

		// Recalculate active memory
		m_ActiveMemory = 0;
		for( uint8 i = inLevel; i < lodList.size(); i++ )
		{
			m_ActiveMemory += lodList.at( i ).LODSize;
		}

		// Update Priority
		UpdatePriority();
	}


	void AdaptiveTexture::PerformIncrease( uint8 inLevel )
	{
		if( !m_Asset )
		{
			Console::WriteLine( "[ERROR] AdaptiveAssetManager: Failed to update texture data structure during increase! Asset became invalid??" );
			return;
		}

		auto& lodList = m_Asset->GetHeader().LODs;
		if( inLevel >= lodList.size() || inLevel > m_MinimumLOD )
		{
			Console::WriteLine( "[ERROR] AdaptiveAssetManager: Failed to update texture data structure during increase! Invalid LOD level??" );
			return;
		}

		m_ActiveLOD		= inLevel;
		m_PendingLOD	= inLevel;

		// Recalculate active memory
		m_ActiveMemory = 0;
		for( uint8 i = inLevel; i < lodList.size(); i++ )
		{
			m_ActiveMemory += lodList.at( i ).LODSize;
		}

		// Get rid of the active load request
		m_PendingRequest.reset();

		// Update Priority
		UpdatePriority();
	}

	bool AdaptiveTexture::Update( float globalMult, float characterMult, float dynamicMult, float levelMult, float staticMult, uint32 objectCount )
	{
		bool bValidEntry = false;

		m_MaxScreenSize		= 0.f;
		m_TotalScreenSize	= 0.f;
		m_Priority_Mult		= 0.f;

		for( auto It = m_Refs.begin(); It != m_Refs.end(); )
		{
			auto obj = It->second.lock();

			if( obj && obj->m_Valid )
			{
				float mult = globalMult;

				switch( obj->m_Type )
				{
				case AdaptiveAssetObjectType::Character:
					mult *= characterMult;
					break;
				case AdaptiveAssetObjectType::Dynamic:
					mult *= dynamicMult;
					break;
				case AdaptiveAssetObjectType::Level:
					mult *= levelMult;
					break;
				case AdaptiveAssetObjectType::Static:
				default:
					mult *= staticMult;
					break;
				}

				m_Priority_Mult = Math::Max( m_Priority_Mult, mult );

				float objScreenSize		= obj->m_ScreenSize * mult;
				m_TotalScreenSize		+= objScreenSize;
				m_MaxScreenSize			= Math::Max( m_MaxScreenSize, objScreenSize );

				bValidEntry = true;
				It++;
			}
			else
			{
				It = m_Refs.erase( It );
			}
		}

		// We want to calculate what percentage of the scene objects are using this texture, range (0,100]
		m_ObjectUsage = (float) m_Refs.size() / (float) objectCount;

		return bValidEntry;
	}


	AdaptiveTextureRequestBase::AdaptiveTextureRequestBase( const std::shared_ptr< AdaptiveTexture >& inTarget, uint8 inLevel )
	{
		if( inTarget && inTarget->GetAsset() )
		{
			m_Target	= inTarget;
			m_Level		= inLevel;
			m_Memory	= 0;
			m_Valid		= true;
		}
		else
		{
			Console::WriteLine( "[ERROR] AdaptiveAssetManager: Attempt to create a texture request with an invalid/null texture!" );
		}
	}


	void AdaptiveTextureRequestBase::Cancel()
	{
		m_Valid = false;
	}



	/*==================================================================================================================
		AdaptiveTextureLoadRequest
	==================================================================================================================*/
	AdaptiveTextureLoadRequest::AdaptiveTextureLoadRequest( const std::shared_ptr< AdaptiveTexture >& inTarget, uint8 inLevel )
		: AdaptiveTextureRequestBase( inTarget, inLevel )
	{
		if( m_Valid )
		{
			// Fill out the missing fields
			auto& header = inTarget->GetAsset()->GetHeader();
			HYPERION_VERIFY( header.LODs.size() > inLevel, "Attempt to create texture load request with out-of-bounds LOD level!" );

			// If there are multiple LODs that need to be loaded, we need to add all the LOD sizes from the current to the target
			uint8 currentLevel = Math::Min( inTarget->GetActiveLevel(), (uint8)header.LODs.size() );
			m_Memory = 0;

			for( uint8 i = inLevel; i < currentLevel; i++ )
			{
				m_Memory += header.LODs.at( i ).LODSize;
			}
		}

		UpdatePriority();
	}

	AdaptiveTextureLoadRequest::~AdaptiveTextureLoadRequest()
	{
		m_Valid = false;
		m_Target.reset();
	}

	void AdaptiveTextureLoadRequest::UpdatePriority()
	{
		if( m_Target && !m_Target->IsLocked() )
		{
			m_Priority = m_Target->CalculatePriority( m_Level );
		}
		else
		{
			m_Priority = 0.5f;
		}
	}

	void AdaptiveTextureLoadRequest::Reset( uint8 inLevel, uint32 inMemory, float inPriority )
	{
		m_Level		= inLevel;
		m_Memory	= inMemory;
		m_Priority	= inPriority;
	}

	/*==================================================================================================================
		AdaptiveTextureUnloadRequest
	==================================================================================================================*/
	AdaptiveTextureUnloadRequest::AdaptiveTextureUnloadRequest( const std::shared_ptr< AdaptiveTexture >& inTarget, uint8 inLevel )
		: AdaptiveTextureRequestBase( inTarget, inLevel )
	{
		if( m_Valid )
		{
			// Fill out missing fields
			auto& header = inTarget->GetAsset()->GetHeader();
			HYPERION_VERIFY( header.LODs.size() > ( inLevel ) && inLevel > 0, "Attempt to create texture unload request with invalid LOD level!" );

			// If were unloading multiple levels, we need to add together their sizes
			uint8 currentLevel = inTarget->GetActiveLevel();
			m_Memory = 0;

			for( uint8 i = currentLevel; i < inLevel; i++ )
			{
				m_Memory += header.LODs.at( i ).LODSize;
			}
		}
	}

	AdaptiveTextureUnloadRequest::~AdaptiveTextureUnloadRequest()
	{
		m_Valid = false;
		m_Target.reset();
	}
}