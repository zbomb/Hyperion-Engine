/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Tools/HMATReader.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/File/IFile.h"

#include <any>


namespace Hyperion
{

	struct TextureReference
	{
		uint32 Identifier;
	};


	class HMATReader
	{

	private:

		DataReader& m_Reader;
		bool m_bInvalidFormat;
		uint64 m_Size;
		std::streampos m_StartPos;

		uint64 m_Offset;

		void _ReadFormat();

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

		HMATReader( DataReader& inReader, uint64 inStart, uint64 inLength );
		~HMATReader();

		HMATReader() = delete;
		HMATReader( const HMATReader& ) = delete;
		HMATReader( HMATReader&& ) = delete;
		HMATReader& operator=( const HMATReader& ) = delete;
		HMATReader& operator=( HMATReader&& ) = delete;

		enum class Result
		{
			Success = 0,
			InvalidFormat = 1,
			ReadFailed = 2,
			InvalidValue = 3,
			InvalidKey = 4,
			Failed = 5
		};

		Result GetEntryCount( uint16& outCount );

		void Begin();
		bool Next();
		Result ReadEntry( std::string& outKey, std::any& outValue );
	};

}