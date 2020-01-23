/*==================================================================================================
	Hyperion Engine
	Source/Assets/TestAsset.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Assets/TestAsset.h"


namespace Hyperion
{
	TestAsset::~TestAsset()
	{

	}

	TestAsset::TestAsset( const String& Data )
		: m_Data( Data )
	{

	}

	String TestAsset::GetAssetType() const
	{
		return "Test";
	}
}