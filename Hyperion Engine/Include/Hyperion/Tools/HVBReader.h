/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Tools/HVBReader.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include "Hyperion/File/PhysicalFileSystem.h"



namespace Hyperion
{


	/*=====================================================================================================
		HVBReader => Hyperion Virtual Bundle Reader
		* This class is used to ingest header data from a .hvb file
	=======================================================================================================*/
	class HVBReader
	{

	public:

		/*
			Public Structures
		*/
		struct HeaderData
		{
			uint32 BlobOffset;
			uint32 TotalSize;
			uint32 FileCount;
		};

		struct FileInfo
		{
			uint16 PathLengthBytes;
			uint32 FileOffset;
			uint32 FileLength;
			uint32 AssetIdentifier;

			String Path;
		};

		/*
			Enumerators
		*/
		enum class ReadHeaderResult
		{
			Success = 0,
			BadValidation = 1,
			MissingData = 2,
			Invalid = 3
		};

		enum class ReadFileResult
		{
			Success = 0,
			InvalidPath = 1,
			NotEnoughData = 2,
			Failed = 3
		};

		/*
			Deleted Functions
		*/
		HVBReader() = delete;
		HVBReader( const HVBReader& ) = delete;
		HVBReader( HVBReader&& ) = delete;
		HVBReader& operator=( const HVBReader& ) = delete;
		HVBReader& operator=( HVBReader&& ) = delete;

		/*
			Constructor/Destructor
		*/
		HVBReader( PhysicalFile& inFile );
		~HVBReader();

		/*
			Member Functions
		*/
		ReadHeaderResult ReadHeader( HeaderData& Out );
		bool StartFiles();
		bool NextFile();
		ReadFileResult ReadFileInfo( FileInfo& Out );


	protected:

		/*
			Data Members
		*/
		DataReader m_Reader;
		uint32 m_BlobBegin;

		/*	
			Helper Functions
		*/
		bool CheckValidationSequence( const std::vector< byte >& inData );

	};

}