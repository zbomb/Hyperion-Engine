/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Material/MaterialInstance.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Asset.h"
#include <any>


namespace Hyperion
{
	// Forward Declarations
	class MaterialAsset;
	class TextureAsset;


	class MaterialInstance
	{

	private:

		bool bIsDynamic;
		std::map< String, std::any > m_Parameters;
		std::vector< AssetRef< TextureAsset > > m_TextureList;

		MaterialInstance();
		MaterialInstance( const AssetRef< MaterialAsset >& inAsset, bool bDynamic );

	public:

		inline bool IsDynamic() const { return bIsDynamic; }

		float GetFloat( const String& inKey, float inDefault );
		int32 GetInt( const String& inKey, int32 inDefault );
		bool GetBool( const String& inKey, bool inDefault );
		AssetRef< TextureAsset > GetTexture( const String& inKey );

		bool SetValue( const String& inKey, float inValue );
		bool SetValue( const String& inKey, int32 inValue );
		bool SetValue( const String& inKey, bool inValue );
		bool SetValue( const String& inKey, const AssetRef< TextureAsset >& inTexture );

		bool ClearValue( const String& inKey );


		std::vector< uint64 > BuildNeededTextureList()
		{
			std::vector< uint64 > Output;
			return Output;
		}

	};

}