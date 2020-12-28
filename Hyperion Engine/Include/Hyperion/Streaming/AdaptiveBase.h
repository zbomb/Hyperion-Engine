/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/AdaptiveBase.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Streaming/DataTypes.h"
#include "Hyperion/Library/Math/MathCore.h"



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
			AdaptiveBase::Update
			- Main Thread
			- Called to iterate through the ref list, check if ref count > 0
			- Also, calculates max and total screen size of this resource
		-------------------------------------------------------------------------------*/
		virtual bool Update( float globalMult, float charMult, float dynMult, float lvlMult, float staticMult, uint32 objCount ) = 0;

	};

}