/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/AdaptiveBase.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Streaming/DataTypes.h"
#include "Hyperion/Library/Math.h"



namespace Hyperion
{

	class AdaptiveBase
	{

	protected:

		std::map< uint32, std::weak_ptr< AdaptiveAssetManagerObjectInfo > > m_Refs;

		uint32 m_Identifier;
		uint32 m_ActiveMemory;
		
		float m_ObjectUsage;
		float m_MaxScreenSize;
		float m_TotalScreenSize;
		float m_Priority_Mult;

		std::atomic< bool > m_Updating;

	public:

		void AddObjectReference( const std::shared_ptr< AdaptiveAssetManagerObjectInfo >& inObj )
		{
			if( inObj )
			{
				m_Refs[ inObj->m_Identifier ] = std::weak_ptr( inObj );
			}
		}

		bool RemoveObjectReference( uint32 inIdentifier )
		{
			return m_Refs.erase( inIdentifier ) > 0;
		}

		bool RemoveObjectReference( const std::shared_ptr< AdaptiveAssetManagerObjectInfo >& inObj )
		{
			return inObj ? RemoveObjectReference( inObj->m_Identifier ) : false;
		}

		inline uint32 GetIdentifier() const { return m_Identifier; }
		inline uint32 GetActiveMemory() const { return m_ActiveMemory; }
		inline bool IsLocked() { return m_Updating.load(); }
		inline void SetLock( bool inValue ) { m_Updating.store( inValue ); }
		inline float GetMaxScreenSize() const { return m_MaxScreenSize; }
		inline float GetTotalScreenSize() const { return m_TotalScreenSize; }

		/*-------------------------------------------------------------------------------
			AdaptiveBase::UpdateRefs
			- Main Thread
			- Called to iterate through the ref list, check if ref count > 0
			- Also, calculates max and total screen size of this resource
		-------------------------------------------------------------------------------*/
		bool Update( float globalMult, float characterMult, float dynamicMult, float levelMult, float staticMult, uint32 objectCount )
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


	};

}