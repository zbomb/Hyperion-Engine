/*==================================================================================================
	Hyperion Engine
	Source/Tools/HMATWriter.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Tools/HSMReader.h"
#include "Hyperion/Library/Binary.h"
#include "Hyperion/Assets/MaterialAsset.h"
#include "Hyperion/Tools/HMATReader.h"

#define HYPERION_HSM_MIN_SIZE 220


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

	// Structs that represent the file structure
	struct HSM_GlobalData
	{
		uint16 toolsetVersion;
		uint8 lodCount;
		uint8 materialCount;
		uint32 _rsvd1;

		float boundSphereX;
		float boundSphereY;
		float boundSphereZ;
		float boundSphereRadius;

		float aabbMinX;
		float aabbMinY;
		float aabbMinZ;
		float aabbMaxX;
		float aabbMaxY;
		float aabbMaxZ;

		// Size 48 bytes
	};

	struct HSM_MaterialInfo
	{
		uint32 length;
		uint32 value;

		// 8 bytes
	};

	struct HSM_LOD
	{
		float minScreenSize;
		uint32 subObjectOffset;
		uint32 collisionOffset;
		uint32 collisionLength;

		uint8 materialCount;
		uint8 subObjectCount;
		uint16 _rsvd1;
		uint32 _rsvd2;
		uint32 _rsvd3;
		uint32 _rsvd4;

		std::vector< HSM_MaterialInfo > materialList;

		// 0 Materials: 32 bytes
	};

	struct HSM_Vertex
	{
		float x;
		float y;
		float z;
		float normal;
		float tangent;
		float binormal;
		float u;
		float v;

		// 32 bytes
	};

	struct HSM_Object
	{
		uint32 objectIndex; // Should we make this a single byte instead?
		uint32 vertexCount;
		uint32 vertexOffset;
		uint32 indexCount;
		uint32 indexOffset;
		uint8 materialSlot;
		uint8 _rsvd1;
		uint16 _rsvd2;

		// 24 bytes
	};

	struct HSM_BakedMaterial
	{
		std::vector< byte > rawData;
	};

	struct HSM_CollisionData
	{
		std::vector< byte > rawData;
	};

	// We dont actually use this structure in the program, its really just here to visualize the layout of the file
	struct HSM_File
	{
		uint64 validBytes;
		HSM_GlobalData globalData;
		std::vector< HSM_MaterialInfo > globalMaterials;
		std::vector< HSM_LOD > lodList;
		std::vector< HSM_Vertex > vertexList;
		std::vector< uint32 > indexList;
		std::vector< HSM_Object > objectList;
		std::vector< HSM_BakedMaterial > materialList; // Min Offset: 220
		std::vector< HSM_CollisionData > collisionList; // Min Offset: 220

		// Minimum Size:
		// 8 + 48 + 0 + 32 + 96 + 12 + 24 + 0 + 0 = 220 bytes
	};

	// Helper functions used to read structures from the file
	bool readGlobalData( DataReader& inReader, HSM_GlobalData& outData, bool bLittleEndian )
	{
		// Read raw data from the file
		std::vector< byte > rawData;
		if( inReader.ReadBytes( rawData, 48 ) != DataReader::ReadResult::Success || rawData.size() != 48 )
		{
			return false;
		}

		// Deserialize each value
		auto it = rawData.begin();
		Binary::DeserializeUInt16( it, outData.toolsetVersion, bLittleEndian );
		std::advance( it, 2 );

		Binary::DeserializeUInt8( it, outData.lodCount );
		std::advance( it, 1 );

		Binary::DeserializeUInt8( it, outData.materialCount );
		std::advance( it, 5 ); // 4 bytes of padding here
		
		Binary::DeserializeFloat( it, outData.boundSphereX, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeFloat( it, outData.boundSphereY, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeFloat( it, outData.boundSphereZ, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeFloat( it, outData.boundSphereRadius, bLittleEndian );
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


	bool readMaterialInfoList( DataReader& inReader, uint8 inCount, std::vector< HSM_MaterialInfo >& outList, bool bLittleEndian )
	{
		// Break out if there are no materials to read
		if( inCount == 0 ) { return true; }

		std::vector< byte > rawData;
		if( inReader.ReadBytes( rawData, inCount * 8 ) != DataReader::ReadResult::Success || rawData.size() != inCount * 8 )
		{
			return false;
		}

		auto it = rawData.begin();

		// Loop through and read each material entry
		for( uint8 i = 0; i < inCount; i++ )
		{
			HSM_MaterialInfo mat;

			Binary::DeserializeUInt32( it, mat.length, bLittleEndian );
			std::advance( it, 4 );

			Binary::DeserializeUInt32( it, mat.value, bLittleEndian );
			std::advance( it, 4 );

			outList.push_back( mat );
		}
		
		return true;
	}


	bool validateMaterialInfoList( std::vector< HSM_MaterialInfo >& inList, DataReader& inReader )
	{
		auto fileSize = inReader.Size();

		for( auto It = inList.begin(); It != inList.end(); It++ )
		{
			if( It->length == 0 )
			{
				// Material is externa, value = asset id
				if( It->value == 0 ) return false;
			}
			else
			{
				// Material is baked into this file, value = offset
				if( It->value == 0 || It->value < HYPERION_HSM_MIN_SIZE ) return false;
				if( It->value + It->length >= fileSize ) return false;
			}
		}

		return true;
	}


	// Helper function to read raw LOD data in
	bool readLODData( DataReader& inReader, HSM_LOD& outData, bool bLittleEndian )
	{
		// First, were going to read all of the static data into a buffer and deserialize it
		std::vector< byte > staticData;
		if( inReader.ReadBytes( staticData, 32 ) != DataReader::ReadResult::Success || staticData.size() != 32 )
		{
			return false;
		}

		auto it = staticData.begin();

		Binary::DeserializeFloat( it, outData.minScreenSize, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeUInt32( it, outData.subObjectOffset, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeUInt32( it, outData.collisionOffset, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeUInt32( it, outData.collisionLength, bLittleEndian );
		std::advance( it, 4 );

		Binary::DeserializeUInt8( it, outData.materialCount );
		std::advance( it, 1 );

		Binary::DeserializeUInt8( it, outData.subObjectCount );
		std::advance( it, 1 );

		// The next 14 bytes are reserved for future use
		// TODO: Other static members per-lod need to be read here

		std::advance( it, 14 );

		// Now, we have a material list we need to read in for this LOD
		if( !readMaterialInfoList( inReader, outData.materialCount, outData.materialList, bLittleEndian ) )
		{
			return false;
		}

		return true;
	}


	bool validateLODData( DataReader& inReader, HSM_LOD& inLOD )
	{
		// First, lets check offsets to see if they fall in the proper range
		auto fSize = inReader.Size();

		if( inLOD.collisionOffset != 0 )
		{
			if( inLOD.collisionOffset < HYPERION_HSM_MIN_SIZE )
			{
				return false;
			}
			else if( inLOD.collisionOffset + inLOD.collisionLength > fSize )
			{
				return false;
			}
		}

		if( inLOD.subObjectCount == 0 )
		{
			return false;
		}
		else
		{
			if( inLOD.subObjectOffset < 204 || // HYPERION_HSM_MIN_SIZE
				inLOD.subObjectOffset + ( inLOD.subObjectCount * 24 ) > fSize )
			{
				return false;
			}
		}

		return validateMaterialInfoList( inLOD.materialList, inReader );
	}

	std::shared_ptr< MaterialAsset > readBakedMaterial( DataReader& inReader, const FilePath& inPath, uint32 inOffset, uint32 inLength )
	{
		// Baked material asset
		// First, lets generate an asset name and an identifier
		String assetPath = String( "materials/__baked__/" ).Append( inPath.ToString() );
		if( assetPath.EndsWith( ".hsm" ) ) { assetPath = assetPath.SubStr( 0, assetPath.Length() - 4 ); }

		uint32 assetId = AssetManager::CalculateIdentifier( assetPath.Append( ".hmat" ) );
		int err_num = 0;

		// Ensure this doesnt exist already
		while( AssetManager::Exists( assetId ) )
		{
			err_num++;

			String newPath = assetPath.Append( std::to_string( err_num ) ).Append( ".hmat" );
			assetId = AssetManager::CalculateIdentifier( newPath );

			if( err_num > 100 )
			{
				Console::WriteLine( "[ERROR] HSMReader: Failed to generate a valid asset id for a baked material!" );
				return nullptr;
			}
		}

		// We need to build the material, using the HMATReader, create a material asset instance, and 
		// add it to the asset cache
		std::vector< byte > buffer;

		inReader.SeekOffset( inOffset );
		if( inReader.ReadBytes( buffer, inLength ) != DataReader::ReadResult::Success )
		{
			Console::WriteLine( "[Warning] HSMReader: Failed to read baked material from the static model file" );
			return nullptr;
		}

		GenericBuffer gbuf( buffer, true, false );
		DataReader reader( gbuf );

		auto mat_ptr = MaterialAssetLoader::Load( assetId, reader, FilePath( assetPath, LocalPath::Content, FileSystem::Disk ) );
		if( !mat_ptr )
		{
			Console::WriteLine( "[Warning] HSMReader: Failed to load baked material from static model file!" );
			return nullptr;
		}

		MaterialAssetCache::Store( assetId, mat_ptr );
		return mat_ptr;
	}


	HSMReader::Result HSMReader::ReadFile( IFile& inFile, std::shared_ptr< StaticModelAsset >& outAsset )
	{
		if( outAsset == nullptr )
		{
			writeReaderError( inFile, "the asset passed in was null!" );
			Result::Failed;
		}

		// Check the file is valid
		if( !inFile.IsValid() || !inFile.CanReadStream() )
		{
			writeReaderError( inFile, "the file wasnt able to be opened" );
			return Result::InvalidFile;
		}

		// Create a data reader
		DataReader reader( inFile );
		
		// Compare against the minimum size
		const long s_MinimumSize = HYPERION_HSM_MIN_SIZE;
		if( reader.Size() < s_MinimumSize )
		{
			writeReaderError( inFile, "the file wasnt large enough!" );
			return Result::InvalidFile;
		}

		// Now, lets read the validation bytes, which ensures this is a static model file, and tells us the endian order of the data stroed inside
		const uint64 s_ValidBytes = 0x1AA1FF289DD90010;
		const uint64 s_ReverseValidBytes = 0x1000D99D28FFA11A;
		std::vector< byte > headerBlockData;

		if( reader.ReadBytes( headerBlockData, 8 ) != DataReader::ReadResult::Success || headerBlockData.size() != 8 )
		{
			writeReaderError( inFile, "the file couldnt be read from" );
			return Result::InvalidFile;
		}

		uint64 fileValidBytes = 0;
		Binary::DeserializeUInt64( headerBlockData.begin(), headerBlockData.begin() + 8, fileValidBytes, false ); // Read it as big endian

		bool bLittleEndian = false;
		if( fileValidBytes == s_ReverseValidBytes )
		{
			// The file type is correct, but, we need to flip the byte order while reading in values that take up multiple bytes
			bLittleEndian = true;
		}
		else if( fileValidBytes != s_ValidBytes )
		{
			writeReaderError( inFile, "the file is not a static model asset!" );
			return Result::InvalidFile;
		}

		// Now, read the global settings from the file
		HSM_GlobalData globalData;
		memset( &globalData, 0, sizeof( globalData ) );

		if( !readGlobalData( reader, globalData, bLittleEndian ) )
		{
			writeReaderError( inFile, "the global data couldnt be read!" );
			return Result::InvalidFile;
		}

		// Check if this file toolset is compatible with the reader
		if( !isToolsetCompatible( globalData.toolsetVersion, ToolsetVersion ) )
		{
			writeReaderError( inFile, "the file toolset version is not compatible" );
			return Result::InvalidVersion;
		}

		// Now, we need to load in the 'global' materials
		std::vector< HSM_MaterialInfo > globalMaterials;
		if( !readMaterialInfoList( reader, globalData.materialCount, globalMaterials, bLittleEndian ) )
		{
			writeReaderError( inFile, "the global material list couldnt be read!" );
			return Result::InvalidFile;
		}

		// Validate this material list
		if( !validateMaterialInfoList( globalMaterials, reader ) )
		{
			writeReaderError( inFile, "the global material list isnt valid" );
			return Result::InvalidMaterial;
		}

		// Now, we want to start reading the list of LODs
		std::vector< HSM_LOD > lodList;
		for( uint8 index = 0; index < globalData.lodCount; index++ )
		{
			HSM_LOD lodData;
			memset( &lodData, 0, sizeof( lodData ) );

			if( !readLODData( reader, lodData, bLittleEndian ) )
			{
				std::string err_msg( "the lod at index " );
				err_msg.append( std::to_string( (int) index ) ).append( " couldnt be read!" );

				writeReaderError( inFile, err_msg );
				return Result::InvalidFile;
			}
			else if( !validateLODData( reader, lodData ) )
			{
				std::string err_msg( "the lod at index " );
				err_msg.append( std::to_string( (int) index ) ).append( " was invalid!" );

				writeReaderError( inFile, err_msg );
				return Result::InvalidMaterial;
			}

			lodList.emplace_back( std::move( lodData ) );
		}

		// Now, lets fill out the asset with our data, start with bounds info
		outAsset->m_BoundingBox.Min			= Vector3D( globalData.aabbMinX, globalData.aabbMinY, globalData.aabbMinZ );
		outAsset->m_BoundingBox.Max			= Vector3D( globalData.aabbMaxX, globalData.aabbMaxY, globalData.aabbMaxZ );
		outAsset->m_BoundingSphere.Center	= Vector3D( globalData.boundSphereX, globalData.boundSphereY, globalData.boundSphereZ );
		outAsset->m_BoundingSphere.Radius	= globalData.boundSphereRadius;

		// Ensure we resize the LOD vector, so exceptions arent thrown when we index them
		outAsset->m_LODs.resize( globalData.lodCount );

		// Next, fill out global material slots
		for( int i = 0; i < globalMaterials.size(); i++ )
		{
			if( globalMaterials[ i ].length == 0 )
			{
				// External material asset
				auto ptr = AssetManager::Get< MaterialAsset >( globalMaterials[ i ].value );
				if( !ptr )
				{
					Console::WriteLine( "[Warning] HSMReader: Failed to get material for static model asset! (ID#", globalMaterials[ i ].value, ")" );
					outAsset->m_MaterialSlots[ (uint8) i ] = nullptr;
				}
				else
				{
					outAsset->m_MaterialSlots[ (uint8) i ] = ptr;
				}
			}
			else
			{
				outAsset->m_MaterialSlots[ (uint8) i ] = readBakedMaterial( reader, inFile.GetPath(), globalMaterials[ i ].value, globalMaterials[ i ].length );
			}
		}

		// Now that the global materials are loaded, lets move onto the LOD levels
		for( uint8 i = 0; i < lodList.size(); i++ )
		{
			auto& data = lodList.at( i );

			StaticModelAssetLOD newLOD;

			newLOD.Index = i;
			newLOD.MinScreenSize = data.minScreenSize;
			
			// Go through, and generate proper material list
			for( uint8 mi = 0; mi < data.materialList.size(); mi++ )
			{
				if( data.materialList[ mi ].length == 0 )
				{
					if( data.materialList[ mi ].value == 0 )
					{
						// Null Asset Slot
						newLOD.MaterialSlots[ mi ] = nullptr;
					}
					else
					{
						// External Asset
						auto ptr = AssetManager::Get< MaterialAsset >( data.materialList[ mi ].value );
						if( ptr == nullptr )
						{
							Console::WriteLine( "[Warning] HSMReader: Failed to get external material for static model!" );
							newLOD.MaterialSlots[ mi ] = nullptr;
						}
						else
						{
							newLOD.MaterialSlots[ mi ] = ptr;
						}
					}
				}
				else
				{
					// Baked Asset
					newLOD.MaterialSlots[ mi ] = readBakedMaterial( reader, inFile.GetPath(), data.materialList[ mi ].value, data.materialList[ mi ].length );
				}
			}
			
			// Now, we need to read all of the subobject data from the file
			std::vector< byte > objData;

			reader.SeekOffset( data.subObjectOffset );
			if( reader.ReadBytes( objData, data.subObjectCount * 24 ) != DataReader::ReadResult::Success )
			{
				writeReaderError( inFile, "failed to read sub-object(s) for LOD" );
				return Result::MissingData;
			}

			auto It = objData.begin();
			
			// Now, loop through and de-serialize each of them
			for( uint8 oi = 0; oi < data.subObjectCount; oi++ )
			{
				StaticModelAssetSubModel obj;

				// objectIndex, vertexCount, vertexOffset, indexCount, indexOffset, materialSlot
				uint32 obj_index;
				Binary::DeserializeUInt32( It, obj_index, bLittleEndian );
				std::advance( It, 4 );

				obj.Index = (uint8) obj_index;

				Binary::DeserializeUInt32( It, obj.VertexLength, bLittleEndian );
				std::advance( It, 4 );

				Binary::DeserializeUInt32( It, obj.VertexOffset, bLittleEndian );
				std::advance( It, 4 );

				Binary::DeserializeUInt32( It, obj.IndexLength, bLittleEndian );
				std::advance( It, 4 );

				Binary::DeserializeUInt32( It, obj.IndexOffset, bLittleEndian );
				std::advance( It, 4 );

				Binary::DeserializeUInt8( It, obj.MaterialIndex );
				
				outAsset->m_LODs[ i ].SubObjects[ oi ] = obj;
			}
		}

		return Result::Success;
	}

}