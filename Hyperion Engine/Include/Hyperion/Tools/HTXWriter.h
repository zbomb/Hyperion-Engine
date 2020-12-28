/*==================================================================================================
	Hyperion Engine
	Hyperion/Include/Tools/HTXWriter.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/File/IFile.h"
#include "Hyperion/Core/Stream.h"
#include "Hyperion/File/FileSystem.h"
#include "Hyperion/Assets/TextureAsset.h"


namespace Hyperion
{

	class HTXWriter
	{

	public:

		HTXWriter() = delete;
		HTXWriter( const HTXWriter& ) = delete;
		HTXWriter( HTXWriter&& ) = delete;
		HTXWriter& operator=( const HTXWriter& ) = delete;
		HTXWriter& operator=( HTXWriter&& ) = delete;

		struct LODInfo
		{
			std::vector< byte > Data;

			uint16 Width;
			uint16 Height;
			uint32 RowSize;
		};

		struct Input
		{
			TextureFormat Format;
			uint8 LevelPadding;

			std::vector< LODInfo > LODs;
		};

		enum class Result
		{
			Success = 0,
			InvalidInput = 1,
			InvalidFile = 2,
			WriteFailed = 3,
			Failed = 4
		};

		static Result Write( std::unique_ptr< File >& inFile, Input& inData );
	};

}