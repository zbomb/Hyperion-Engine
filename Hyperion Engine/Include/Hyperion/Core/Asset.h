/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Asset.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"
#include "Hyperion/File/FilePath.h"
#include "Hyperion/File/FileSystem.h"
#include "Hyperion/Core/AssetType.h"



namespace Hyperion
{

	/*
	*	TODO: Eventually, I want to make AssetBase derived from Object
	*	- Issue though, we then have to switch to using HypPtr's
	*/
	class AssetBase 
	{

	protected:

		String m_Path;
		uint64 m_Offset;
		uint64 m_Length;
		uint32 m_Identifier;

	public:

		AssetBase( uint32 inIdentifier, const String& inPath, uint64 inOffset, uint64 inLength )
			: m_Path( inPath ), m_Identifier( inIdentifier ), m_Offset( inOffset ), m_Length( inLength )
		{}

		AssetBase() = delete;
		AssetBase( const AssetBase& ) = delete;
		AssetBase( AssetBase&& ) = delete;
		AssetBase& operator=( const AssetBase& ) = delete;
		AssetBase& operator=( AssetBase&& ) = delete;

		virtual ~AssetBase()
		{
		}

		inline String GetPath() const { return m_Path; }
		inline uint32 GetIdentifier() const { return m_Identifier; }
		inline uint64 GetFileOffset() const { return m_Offset; }
		inline uint64 GetFileLength() const { return m_Length; }

	};

}