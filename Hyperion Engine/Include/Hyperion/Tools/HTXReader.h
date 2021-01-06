/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Tools/HTXReader.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/File/IFile.h"
#include "Hyperion/Core/Stream.h"
#include "Hyperion/Assets/TextureAsset.h"


namespace Hyperion
{

	class HTXReader
	{

	private:

		IFile& m_Target;
		uint64 m_Offset;
		uint64 m_Length;
		DataReader m_Reader;
		bool m_bValidFormat;

	public:

		HTXReader( IFile& inFile, uint64 offset = 0, uint64 length = 0 );
		~HTXReader();

		HTXReader() = delete;
		HTXReader( const HTXReader& ) = delete;
		HTXReader( HTXReader&& ) = delete;
		HTXReader& operator=( const HTXReader& ) = delete;
		HTXReader& operator=( HTXReader&& ) = delete;

		enum class Result
		{
			Success = 0,
			InvalidFormat = 1,
			InvalidHeader = 2,
			InvalidLOD = 3,
			InvalidParams = 4,
			Failed = 5
		};


		Result ReadHeader( TextureHeader& outHeader );
		Result ReadRawData( uint64 inOffset, uint32 inSize, std::vector< byte >& outData );
		Result ReadLODData( const TextureHeader& inHeader, uint8 inLevel, std::vector< byte >& outData );
		Result ReadLODRange( const TextureHeader& inHeader, std::vector< uint8 >& inLevels, std::map< uint8, std::vector< byte > >& outData );
	};

}
