/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Tools/HMATWriter.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/File/IFile.h"
#include "Hyperion/Library/Binary.h"

#include <any>


namespace Hyperion
{
	// Forward Declarations
	class TextureAsset;


	class HMATWriter
	{

	private:

		IFile& m_File;
		std::map< String, std::any > m_Values;

		bool _CheckKey( const String& inKey );

		struct TextureWrapper
		{
			uint32 value;
		};

		enum class ValueType
		{
			Boolean = 0,
			Int32 = 1,
			UInt32 = 2,
			Float = 3,
			String = 4,
			Texture = 5
		};

	public:

		HMATWriter( IFile& inFile );
		~HMATWriter();

		HMATWriter() = delete;
		HMATWriter( const HMATWriter& ) = delete;
		HMATWriter( HMATWriter&& ) = delete;
		HMATWriter& operator=( const HMATWriter& ) = delete;
		HMATWriter& operator=( HMATWriter&& ) = delete;

		enum class Result
		{
			Success = 0,
			InvalidFile = 1,
			WriteFailed = 2,
			ValueOverflow = 3,
			InvalidValue = 4,
			Failed = 5
		};

		bool AddEntry( const String& inKey, bool inValue, bool bOverride = false );
		bool AddEntry( const String& inKey, int32 inValue, bool bOverride = false );
		bool AddEntry( const String& inKey, uint32 inValue, bool bOverride = false );
		bool AddEntry( const String& inKey, float inValue, bool bOverride = false );
		bool AddEntry( const String& inKey, const String& inValue, bool bOverride = false );
		bool AddEntry( const String& inKey, const std::shared_ptr< TextureAsset >& inTexture, bool bOverride = false );

		Result Flush();
	};

}