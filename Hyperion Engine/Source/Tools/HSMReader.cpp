/*==================================================================================================
	Hyperion Engine
	Source/Tools/HMATWriter.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Tools/HSMReader.h"
#include "Hyperion/Library/Binary.h"
#include "Hyperion/Assets/MaterialAsset.h"
#include "Hyperion/Tools/HMATReader.h"



namespace Hyperion
{
	// Helper function to save us a little typing
	void writeReaderError( IFile& inFile, const std::string& inMessage )
	{
		// Later on, we will comment this out, so no error messages are printed from here
		Console::WriteLine( "[Warning] HSMReader: Failed to read static model file \"", inFile.GetPath().ToString(), "\" because ", inMessage );
	}

	// Helper function to determine static model toolset version compatibility
	bool isToolsetCompatible( uint16 fileVersion, uint16 readerVersion )
	{
		switch( fileVersion )
		{
		case 0x0001:
			return readerVersion > 0x0000;
		default:
			return false;
		}
	}

	
	/*
	*	Data Structures
	*/

	struct HSM_Vertex
	{
		// ----- 0 -----

		float x;
		float y;
		float z;
		float nx;
		float ny;
		float nz;
		float u;
		float v;

		// ----- 32 -----
	};

	struct HSM_SubObject
	{
		// ----- 0 -----

		uint32 objectID;
		uint8 materialSlot;
		uint8 _rsvd1;
		uint16 _rsvd2;

		// ----- 8 ------

		uint32 vertexOffset;
		uint32 vertexCount;
		uint32 indexOffset;
		uint32 indexCount;

		// ----- 24 -----
	};

	struct HSM_LOD
	{
		// ----- 0 -----

		float minScreenSize; // Not yet implemented
		uint32 subObjectOffset;
		uint8 subObjectCount; 
		uint8 _rsvd1;
		uint16 _rsvd2;

		// ----- 12 ------

		uint32 collisionOffset;
		uint32 collisionSize;
		uint32 _rsvd3;

		// ----- 24 ----- [End of file]

		std::vector< HSM_SubObject > subObjects;

	};

	struct HSM_StaticData
	{
		// ----- 0 -----

		uint16 toolsetVersion;
		uint8 lodCount;
		uint8 _rsvd1;
		uint32 _rsvd2;

		// ----- 8 -----

		float boundsSphereX;
		float boundsSphereY;
		float boundsSphereZ;
		float boundsSphereRadius;

		// ----- 24 ------

		float aabbMinX;
		float aabbMinY;
		float aabbMinZ;
		float aabbMaxX;
		float aabbMaxY;
		float aabbMaxZ;

		// ----- 48 -----

	};

	struct HSM_File
	{
		// ----- 0 ------

		uint64 validationBytes;
		uint32 _rsvd1;
		uint32 _rsvd2;

		// ----- 16 -----

		HSM_StaticData staticData;

		// ------ 64 ------

		std::vector< HSM_LOD > lodList;

		//std::vector< HSM_SubObject > objectList;	In file only, we read it into a more vertical structure

		//std::vector< HSM_Vertex > vertexList;
		//std::vector< uint32 > indexList;

		//std::vector< byte > collisionData; // To be implemented in the future
	};



	/*
	*	Functions
	*/
	bool readSubObject( DataReader& inReader, HSM_SubObject& outData, bool bLittleEndian )
	{
		// Read the raw data in
		std::vector< byte > rawData;
		if( inReader.ReadBytes( rawData, 24 ) != DataReader::ReadResult::Success || rawData.size() != 24 )
		{
			return false;
		}

		// Deserialize the data
		auto it = rawData.begin();
		Binary::DeserializeUInt32( it, outData.objectID, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeUInt8( it, outData.materialSlot );
		std::advance( it, 4 ); // Skips over 3 bytes of reserved data

		Binary::DeserializeUInt32( it, outData.vertexOffset, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeUInt32( it, outData.vertexCount, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeUInt32( it, outData.indexOffset, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeUInt32( it, outData.indexCount, bLittleEndian );
		std::advance( it, 4 );

		return true;
	}


	bool readLOD( DataReader& inReader, HSM_LOD& outData, bool bLittleEndian )
	{
		// Read raw static data in
		std::vector< byte > rawData;
		if( inReader.ReadBytes( rawData, 24 ) != DataReader::ReadResult::Success || rawData.size() != 24 )
		{
			return false;
		}

		// Deserialize the static data
		auto it = rawData.begin();
		Binary::DeserializeFloat( it, outData.minScreenSize, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeUInt32( it, outData.subObjectOffset, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeUInt8( it, outData.subObjectCount );
		std::advance( it, 4 ); // Skipping 3 bytes of reserved data

		Binary::DeserializeUInt32( it, outData.collisionOffset, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeUInt32( it, outData.collisionSize, bLittleEndian );
		std::advance( it, 4 );
		std::advance( it, 4 ); // Skipping 4 bytes of reserved data

		// Now, lets read in the subobjects
		for( uint8 i = 0; i < outData.subObjectCount; i++ )
		{
			uint32 offset = outData.subObjectOffset + ( i * 24 );
			inReader.SeekOffset( offset );

			auto& subObj = outData.subObjects.emplace_back( HSM_SubObject() );
			if( !readSubObject( inReader, subObj, bLittleEndian ) )
			{
				return false;
			}
		}

		return true;
	}


	bool readStaticData( DataReader& inReader, HSM_StaticData& outData, bool bLittleEndian )
	{
		// Read raw data in
		std::vector< byte > rawData;
		if( inReader.ReadBytes( rawData, 48 ) != DataReader::ReadResult::Success || rawData.size() != 48 )
		{
			return false;
		}

		// Deserialize the structure
		auto it = rawData.begin();
		Binary::DeserializeUInt16( it, outData.toolsetVersion, bLittleEndian );
		std::advance( it, 2 );

		Binary::DeserializeUInt8( it, outData.lodCount );
		std::advance( it, 6 ); // Skipping 5 bytes of reserved data

		Binary::DeserializeFloat( it, outData.boundsSphereX, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeFloat( it, outData.boundsSphereY, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeFloat( it, outData.boundsSphereZ, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeFloat( it, outData.boundsSphereRadius, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeFloat( it, outData.aabbMinX, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeFloat( it, outData.aabbMinY, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeFloat( it, outData.aabbMinZ, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeFloat( it, outData.aabbMaxX, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeFloat( it, outData.aabbMaxY, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeFloat( it, outData.aabbMaxZ, bLittleEndian );
		std::advance( it, 4 );

		return true;
	}


	HSMReader::Result HSMReader::ReadFile( IFile& inFile, std::shared_ptr< StaticModelAsset >& outAsset )
	{
		// Validate the parameters
		if( outAsset == nullptr )
		{
			writeReaderError( inFile, "asset was null" );
			return Result::Failed;
		}

		// TODO: Calcualte actual minimum file size for a valid static model
		if( !inFile.IsValid() || !inFile.CanReadStream() || inFile.GetSize() < 100 )
		{
			writeReaderError( inFile, "invalid file" );
			return Result::InvalidFile;
		}
		
		// Now, lets check if the validation bytes are correct, and also determine endian order
		const uint64 s_ValidBytes			= 0x1AA1FF289DD90010;
		const uint64 s_ReverseValidBytes	= 0x1000D99D28FFA11A;

		std::vector< byte > headerData;
		DataReader reader( inFile );
		reader.SeekBegin();

		if( reader.ReadBytes( headerData, 8 ) != DataReader::ReadResult::Success || headerData.size() != 8 )
		{
			writeReaderError( inFile, "the file couldnt be read from" );
			return Result::InvalidFile;
		}

		HSM_File fileData;
		memset( &fileData, 0, sizeof( HSM_File ) );

		bool bLittleEndian		= false;

		Binary::DeserializeUInt64( headerData.begin(), fileData.validationBytes, false );
		if( fileData.validationBytes == s_ReverseValidBytes )
		{
			bLittleEndian = true;
		}
		else if( fileData.validationBytes != s_ValidBytes )
		{
			writeReaderError( inFile, "file isnt a valid static model" );
			return Result::InvalidFile;
		}

		// Start read main file data in
		reader.SeekOffset( 16 );
		if( !readStaticData( reader, fileData.staticData, bLittleEndian ) )
		{
			writeReaderError( inFile, "failed to read static data" );
			return Result::MissingData;
		}

		// Ensure the toolset is compatible
		if( !isToolsetCompatible( fileData.staticData.toolsetVersion, ToolsetVersion ) )
		{
			writeReaderError( inFile, "The toolset version is not compatible" );
			return Result::InvalidVersion;
		}

		// Now, lets read the LOD data in
		for( uint8 i = 0; i < fileData.staticData.lodCount; i++ )
		{
			reader.SeekOffset( 64 + ( i * 24 ) );

			HSM_LOD lod;
			memset( &lod, 0, sizeof( HSM_LOD ) );

			if( !readLOD( reader, lod, bLittleEndian ) )
			{
				writeReaderError( inFile, "Failed to read LOD from file!" );
				return Result::InvlaidLOD;
			}

			fileData.lodList.push_back( std::move( lod ) );
		}
		
		// Now that the data is read in, we can move it into the asset
		outAsset->m_BoundingBox.Min		= Vector3D( fileData.staticData.aabbMinX, fileData.staticData.aabbMinY, fileData.staticData.aabbMinZ );
		outAsset->m_BoundingBox.Max		= Vector3D( fileData.staticData.aabbMaxX, fileData.staticData.aabbMaxY, fileData.staticData.aabbMaxZ );

		outAsset->m_BoundingSphere.Center	= Vector3D( fileData.staticData.boundsSphereX, fileData.staticData.boundsSphereY, fileData.staticData.boundsSphereZ );
		outAsset->m_BoundingSphere.Radius	= fileData.staticData.boundsSphereRadius;

		outAsset->m_LODs.resize( fileData.staticData.lodCount );
		for( uint8 i = 0; i < fileData.staticData.lodCount; i++ )
		{
			outAsset->m_LODs[ i ].Index = i;
			outAsset->m_LODs[ i ].MinScreenSize = fileData.lodList[ i ].minScreenSize;
			
			auto& subObjList = outAsset->m_LODs[ i ].SubObjects;
			subObjList.resize( fileData.lodList[ i ].subObjectCount );

			for( uint8 o = 0; o < fileData.lodList[ i ].subObjectCount; o++ )
			{
				subObjList[ o ].Index = o;
				subObjList[ o ].IndexOffset		= fileData.lodList[ i ].subObjects[ o ].indexOffset;
				subObjList[ o ].IndexLength		= fileData.lodList[ i ].subObjects[ o ].indexCount * 4;
				subObjList[ o ].IndexCount		= fileData.lodList[ i ].subObjects[ o ].indexCount;
				subObjList[ o ].VertexOffset	= fileData.lodList[ i ].subObjects[ o ].vertexOffset;
				subObjList[ o ].VertexLength	= fileData.lodList[ i ].subObjects[ o ].vertexCount * 32;
				subObjList[ o ].VertexCount		= fileData.lodList[ i ].subObjects[ o ].vertexCount;
				subObjList[ o ].MaterialIndex	= fileData.lodList[ i ].subObjects[ o ].materialSlot;
			}
		}

		return Result::Success;
	}

}