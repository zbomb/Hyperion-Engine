/*==================================================================================================
	Hyperion Engine
	Source/Assets/TestAsset.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Assets/TestAsset.h"


namespace Hyperion
{

	TestAsset::TestAsset()
	{

	}

	TestAsset::~TestAsset()
	{

	}

	TestAsset::TestAsset( const String& In )
		: m_Data( In )
	{

	}

	String TestAsset::GetAssetName() const
	{
		return "#test_asset";
	}

}