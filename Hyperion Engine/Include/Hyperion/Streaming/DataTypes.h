/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Streaming/DataTypes.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Assets/TextureAsset.h"
#include "Hyperion/Assets/ModelAsset.h"



namespace Hyperion
{

	enum class AdaptiveAssetObjectType
	{
		Character = 0,
		Dynamic = 1,
		Static = 2,
		Level = 3
	};

	struct AdaptiveAssetManagerObjectInfo
	{
		AdaptiveAssetObjectType m_Type;
		uint32 m_Identifier;
		Vector3D m_Position;
		float m_Radius;
		bool m_Valid;
		bool m_Dirty;
		float m_ScreenSize;

		AdaptiveAssetManagerObjectInfo& operator=( const AdaptiveAssetManagerObjectInfo& Other )
		{
			m_Type = Other.m_Type;
			m_Identifier = Other.m_Identifier;
			m_Position = Other.m_Position;
			m_Radius = Other.m_Radius;
			m_Valid = Other.m_Valid;
			m_ScreenSize = Other.m_ScreenSize;
			m_Dirty = true;

			return *this;
		}
	};

	struct AdaptiveTextureInfo
	{
		AssetRef< TextureAsset > m_Asset;
	};

	struct AdaptiveModelInfo
	{
		AssetRef< ModelAsset > m_Asset;
		bool m_Dynamic;
	};

	struct AdaptiveAssetManagerCameraInfo
	{
		Vector3D Position;
		float FOV;
		uint32 ScreenHeight;
		bool bDirty;

		AdaptiveAssetManagerCameraInfo()
			: Position( 0.f, 0.f, 0.f ), FOV( 3.14156f / 4.f ), ScreenHeight( 1024 ), bDirty( true )
		{
		}
	};

}