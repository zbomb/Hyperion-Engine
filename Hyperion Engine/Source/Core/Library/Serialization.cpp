/*==================================================================================================
	Hyperion Engine
	Source/Core/Serialization.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/Library/Serialization.h"
#include "Hyperion/Core/Library/Binary.h"

#include <iostream>


namespace Hyperion
{

	/*---------------------------------------------------------------------------
		ArchiveWriter Implementation
	---------------------------------------------------------------------------*/

	/*
		Constructor
	*/
	ArchiveWriter::ArchiveWriter( std::vector< byte >& TargetBuffer, ByteOrder Endianess /* = ByteOrder::BigEndian */ )
		: m_TargetBuffer( TargetBuffer ), m_ByteOrder( Endianess )
	{
		m_HeaderBuffer.reserve( 32 );
		m_ContentBuffer.reserve( 128 );

		m_CurrentBlock = BlockType::Base;
	}

	/*
		Destructor
	*/
	ArchiveWriter::~ArchiveWriter()
	{
	}

	void ArchiveWriter::Clear()
	{
		// Clear all buffers
		std::vector< byte >().swap( m_HeaderBuffer );
		std::vector< byte >().swap( m_ContentBuffer );
	}

	void ArchiveWriter::Flush()
	{
		// Write the header and body into the target buffer
		// First though, clear out anything currently in the target buffer
		std::vector< byte >().swap( m_TargetBuffer );
		m_TargetBuffer.reserve( 10 + m_HeaderBuffer.size() + m_ContentBuffer.size() );

		// Next we want to write a unique 4 byte sequence, which we can check for in a buffer before attempting to process it
		static const uint32 ArchiveByte = 0xF02CF99A;
		Binary::SerializeUInt32( ArchiveByte, m_TargetBuffer, m_ByteOrder == ByteOrder::LittleEndian );

		// Then, we want to write the offset at which the content starts, so when read, we can split it up
		uint32 HeaderSize = 10 + (uint32) m_HeaderBuffer.size();
		Binary::SerializeUInt32( HeaderSize, m_TargetBuffer, m_ByteOrder == ByteOrder::LittleEndian );

		// Write control byte to denote the start of the header info
		static const uint8 HeaderStart = (uint8) ArchiveControlByte::HeaderStart;
		Binary::SerializeUInt8( HeaderStart, m_TargetBuffer );

		// Flush all header data into target buffer
		std::move(
			m_HeaderBuffer.begin(),
			m_HeaderBuffer.end(),
			std::back_inserter( m_TargetBuffer )
		);

		// Clear Header Buffer
		std::vector< byte >().swap( m_HeaderBuffer );

		// Write control byte to denote the end of the header data
		static const uint8 HeaderEnd = (uint8) ArchiveControlByte::HeaderEnd;
		Binary::SerializeUInt8( HeaderEnd, m_TargetBuffer );

		// Flush main content into the target buffer
		std::move(
			m_ContentBuffer.begin(),
			m_ContentBuffer.end(),
			std::back_inserter( m_TargetBuffer )
		);

		// Clear Content Buffer
		std::vector< byte >().swap( m_ContentBuffer );

	}

	size_t ArchiveWriter::GetHeaderSize() const
	{
		return m_HeaderBuffer.size();
	}

	size_t ArchiveWriter::GetContentSize() const
	{
		return m_ContentBuffer.size();
	}

	size_t ArchiveWriter::GetAllocatedHeaderSize() const
	{
		return m_HeaderBuffer.capacity();
	}

	size_t ArchiveWriter::GetAllocatedContentSize() const
	{
		return m_ContentBuffer.capacity();
	}

	void ArchiveWriter::AllocateHeaderSpace( size_t AdditionalSpace )
	{
		m_HeaderBuffer.reserve( m_HeaderBuffer.capacity() + AdditionalSpace );
	}

	void ArchiveWriter::AllocateContentSpace( size_t AdditionalSpace )
	{
		m_ContentBuffer.reserve( m_ContentBuffer.capacity() + AdditionalSpace );
	}

	void ArchiveWriter::WriteHeaderByte( ArchiveControlByte Control, ArchiveType Type )
	{
		uint8 FinalByte = (uint8) Control | (uint8) Type;
		Binary::SerializeUInt8( FinalByte, m_HeaderBuffer );
	}

	void ArchiveWriter::WriteTableHeaderByte( bool bIsStart, ArchiveTableType Type )
	{
		uint8 FinalByte = (uint8)( bIsStart ? ArchiveControlByte::TableStart : ArchiveControlByte::TableEnd ) | (uint8) Type;
		Binary::SerializeUInt8( FinalByte, m_HeaderBuffer );
	}

	void ArchiveWriter::Impl_WriteHeader( ArchiveType Type, bool bIsKey )
	{
		if( m_CurrentBlock == BlockType::Table )
		{
			WriteHeaderByte( bIsKey ? ArchiveControlByte::TableKey : ArchiveControlByte::TableValue, Type );
		}
		else if( m_CurrentBlock == BlockType::Object )
		{
			WriteHeaderByte( ArchiveControlByte::ObjData, Type );
		}
		else
		{
			WriteHeaderByte( ArchiveControlByte::Value, Type );
		}
	}


	void ArchiveWriter::WriteImpl( const bool& In, bool bIsKey /* = false */ )
	{
		Impl_WriteHeader( ArchiveType::Boolean, bIsKey );
		Binary::SerializeBoolean( In, m_ContentBuffer );
	}

	void ArchiveWriter::WriteImpl( const double& In, bool bIsKey /* = false */ )
	{
		Impl_WriteHeader( ArchiveType::Double, bIsKey );
		Binary::SerializeDouble( In, m_ContentBuffer, m_ByteOrder == ByteOrder::LittleEndian );
	}

	void ArchiveWriter::WriteImpl( const float& In, bool bIsKey /* = false */ )
	{
		Impl_WriteHeader( ArchiveType::Float, bIsKey );
		Binary::SerializeFloat( In, m_ContentBuffer, m_ByteOrder == ByteOrder::LittleEndian );
	}

	void ArchiveWriter::WriteImpl( const uint8& In, bool bIsKey /* = false */ )
	{
		Impl_WriteHeader( ArchiveType::UInt8, bIsKey );
		Binary::SerializeUInt8( In, m_ContentBuffer );
	}

	void ArchiveWriter::WriteImpl( const int8& In, bool bIsKey /* = false */ )
	{
		Impl_WriteHeader( ArchiveType::Int8, bIsKey );
		Binary::SerializeInt8( In, m_ContentBuffer );
	}

	void ArchiveWriter::WriteImpl( const uint16& In, bool bIsKey /* = false */ )
	{
		Impl_WriteHeader( ArchiveType::UInt16, bIsKey );
		Binary::SerializeUInt16( In, m_ContentBuffer, m_ByteOrder == ByteOrder::LittleEndian );
	}

	void ArchiveWriter::WriteImpl( const int16& In, bool bIsKey /* = false */ )
	{
		Impl_WriteHeader( ArchiveType::Int16, bIsKey );
		Binary::SerializeInt16( In, m_ContentBuffer, m_ByteOrder == ByteOrder::LittleEndian );
	}

	void ArchiveWriter::WriteImpl( const uint32& In, bool bIsKey /* = false */ )
	{
		Impl_WriteHeader( ArchiveType::UInt32, bIsKey );
		Binary::SerializeUInt32( In, m_ContentBuffer, m_ByteOrder == ByteOrder::LittleEndian );
	}

	void ArchiveWriter::WriteImpl( const int32& In, bool bIsKey /* = false */ )
	{
		Impl_WriteHeader( ArchiveType::Int32, bIsKey );
		Binary::SerializeInt32( In, m_ContentBuffer, m_ByteOrder == ByteOrder::LittleEndian );
	}

	void ArchiveWriter::WriteImpl( const uint64& In, bool bIsKey /* = false */ )
	{
		Impl_WriteHeader( ArchiveType::UInt64, bIsKey );
		Binary::SerializeUInt64( In, m_ContentBuffer, m_ByteOrder == ByteOrder::LittleEndian );
	}

	void ArchiveWriter::WriteImpl( const int64& In, bool bIsKey /* = false */ )
	{
		Impl_WriteHeader( ArchiveType::Int64, bIsKey );
		Binary::SerializeInt64( In, m_ContentBuffer, m_ByteOrder == ByteOrder::LittleEndian );
	}

	void ArchiveWriter::WriteImpl( ISerializable& In )
	{
		// Store the current block type, so we can reset it when were done
		auto PreviousBlock = m_CurrentBlock;
		m_CurrentBlock = BlockType::Object;

		// If were in a table, or another object we need to write a value denoting that we have an Obj as value
		if( PreviousBlock == BlockType::Table )
		{
			WriteHeaderByte( ArchiveControlByte::TableValue, ArchiveType::Object );
		}
		else if( PreviousBlock == BlockType::Object )
		{
			WriteHeaderByte( ArchiveControlByte::ObjData, ArchiveType::Object );
		}
		else
		{
			WriteHeaderByte( ArchiveControlByte::Value, ArchiveType::Object );
		}
		
		// Write byte signifying the start of the object
		WriteHeaderByte( ArchiveControlByte::ObjStart, ArchiveType::Object );

		// Serialize Object
		In.Serialize( *this );

		// Write byte signifying the end of the object
		WriteHeaderByte( ArchiveControlByte::ObjEnd, ArchiveType::Object );

		// Reset block type
		m_CurrentBlock = PreviousBlock;
	}


	/*-----------------------------------------------------------------------------
		Archive Reader
	-----------------------------------------------------------------------------*/

	ArchiveReader::ArchiveReader( std::vector< byte >& SourceBuffer )
		: m_SourceBuffer( SourceBuffer )
	{
		m_State			= State::Invalid;
		m_ByteOrder		= ByteOrder::BigEndian;
		m_Block			= BlockType::Base;

		if( IngestData() )
		{
			if( std::distance( m_HeaderBegin, m_HeaderEnd ) <= 0 )
			{
				m_State = State::EndOfBuffer;
			}
			else
			{
				m_State = State::Reading;
			}
		}
	}


	ArchiveReader::~ArchiveReader()
	{
		m_State = State::Invalid;
	}

	bool ArchiveReader::IngestData()
	{
		// We need to verify the source buffer, and get the starting position of the header and body
		if( m_SourceBuffer.size() < 10 )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: Failed to read archive from source buffer... not enough data!" );
			return false;
		}

		// First, lets read the first four bytes, so we can verify if this buffer contains an archive, and the byte order
		byte uB1, uB2, uB3, uB4;
		auto It = m_SourceBuffer.begin();

		uB1 = *( It++ );
		uB2 = *( It++ );
		uB3 = *( It++ );
		uB4 = *( It++ );

		if( uB1 == 0xF0 && uB2 == 0x2C && uB3 == 0xF9 && uB4 == 0x9A )
		{
			m_ByteOrder = ByteOrder::BigEndian;
		}
		else if( uB1 == 0x9A && uB2 == 0xF9 && uB3 == 0x2C && uB4 == 0xF0 )
		{
			m_ByteOrder = ByteOrder::LittleEndian;
		}
		else
		{
			// Identifier Byte Not Found!
			Console::WriteLine( "[ERROR] ArchiveReader: Failed to read archive from source buffer.. invalid format (header indentifier missing)!" );
			return false;
		}

		// Now that we know the byte order, we can call our read functions properly
		// Next, we need to read the length of the header, so we can figure out where the data starts
		uint32 HeaderLength = 0;
		if( !Binary::DeserializeUInt32( It, It + 4, HeaderLength, m_ByteOrder == ByteOrder::LittleEndian ) )
		{
			// Couldnt read header length.. we could possibly recover from this, but for now were going to error out
			Console::WriteLine( "[ERROR] ArchiveReader: Failed to read archive from source buffer.. invalid header length" );
			return false;
		}

		// Advance iterator to where the header start byte should be
		It += 4;

		byte StartByte = *( It++ );
		if( StartByte != (byte) ArchiveControlByte::HeaderStart )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: Failed to read archive from source buffer.. missing header start byte!" );
			return false;
		}

		// Now we can setup all of our iterator positions for ingestion
		// The current iterator were using is going to be at the start of the header data
		m_HeaderBegin = It;

		// Now, the end of the header can be calculate from the uint32 we read, and the content start is the same iterator
		m_HeaderEnd = m_SourceBuffer.begin() + HeaderLength;
		m_ContentBegin = m_HeaderEnd;

		// Finally, the content end is going to be the end of the vector
		m_ContentEnd = m_SourceBuffer.end();

		m_ContentPosition	= m_ContentBegin;
		m_HeaderPosition	= m_HeaderBegin;

		return true;
	}


	bool ArchiveReader::IsValidType( byte In )
	{
		static const byte BoolType = (byte) ArchiveType::Boolean;
		static const byte FloatType = (byte) ArchiveType::Float;
		static const byte DoubleType = (byte) ArchiveType::Double;
		static const byte Int8Type = (byte) ArchiveType::Int8;
		static const byte UInt8Type = (byte) ArchiveType::UInt8;
		static const byte Int16Type = (byte) ArchiveType::Int16;
		static const byte UInt16Type = (byte) ArchiveType::UInt16;
		static const byte Int32Type = (byte) ArchiveType::Int32;
		static const byte UInt32Type = (byte) ArchiveType::UInt32;
		static const byte Int64Type = (byte) ArchiveType::Int64;
		static const byte UInt64Type = (byte) ArchiveType::UInt64;
		static const byte ObjType = (byte) ArchiveType::Object;
		static const byte TableType = (byte) ArchiveType::Table;
		static const byte StringType = (byte) ArchiveType::String;

		switch( In )
		{
		case BoolType:
		case FloatType:
		case DoubleType:
		case Int8Type:
		case UInt8Type:
		case Int16Type:
		case UInt16Type:
		case Int32Type:
		case UInt32Type:
		case Int64Type:
		case UInt64Type:
		case ObjType:
		case TableType:
		case StringType:
			return true;
		default:
			return false;
		}
	}

	bool ArchiveReader::IsValidControl( byte In )
	{
		static const byte Value = (byte) ArchiveControlByte::Value;
		static const byte HeaderStart = (byte) ArchiveControlByte::HeaderStart;
		static const byte HeaderEnd = (byte) ArchiveControlByte::HeaderEnd;
		static const byte ObjStart = (byte) ArchiveControlByte::ObjStart;
		static const byte ObjEnd = (byte) ArchiveControlByte::ObjEnd;
		static const byte ObjData = (byte) ArchiveControlByte::ObjData;
		static const byte TableStart = (byte) ArchiveControlByte::TableStart;
		static const byte TableEnd = (byte) ArchiveControlByte::TableEnd;
		static const byte TableKey = (byte) ArchiveControlByte::TableKey;
		static const byte TableValue = (byte) ArchiveControlByte::TableValue;

		switch( In )
		{
		case Value:
		case HeaderStart:
		case HeaderEnd:
		case ObjStart:
		case ObjEnd:
		case ObjData:
		case TableStart:
		case TableEnd:
		case TableValue:
		case TableKey:
			return true;
		default:
			return false;
		}
	}

	bool ArchiveReader::IsValidTableType( byte In )
	{
		static const byte VectorType = (byte) ArchiveTableType::Vector;
		static const byte MapType = (byte) ArchiveTableType::Map;

		switch( In )
		{
		case VectorType:
		case MapType:
			return true;
		default:
			return false;
		}
	}

	void ArchiveReader::PopHeaderByte( uint32 Count /* = 1 */ )
	{
		// Pop the header, the specified number of times or until we hit the end of the header
		for( uint32 i = 0; i < Count; i++ )
		{
			if( m_HeaderPosition >= m_HeaderEnd )
				break;

			std::advance( m_HeaderPosition, 1 );
		}
	}

	ArchiveReader::HeaderByte ArchiveReader::PeekHeaderByte( uint32 Offset /* = 0 */ )
	{
		HeaderByte Output;
		Output.Func			= ArchiveControlByte::HeaderEnd;
		Output.Type			= ArchiveType::Boolean;
		Output.TableInfo	= ArchiveTableType::Vector;
		Output.Error		= false;

		auto targetIt = m_HeaderPosition;
		std::advance( targetIt, Offset );

		if( targetIt >= m_HeaderEnd )
		{
			// End Of Buffer
			Output.Func = ArchiveControlByte::HeaderEnd;
			return Output;
		}

		// Read the next byte from the header
		byte headerByte = *( targetIt );

		// Split into two halfs, one half holds the control byte, the other holds the type byte
		byte controlByte = ( headerByte & 0b11110000 );
		byte typeByte = ( headerByte & 0b00001111 );

		if( controlByte == (byte)ArchiveControlByte::HeaderEnd || targetIt >= m_HeaderEnd )
		{
			Output.Func = ArchiveControlByte::HeaderEnd;
			return Output;
		}

		// Now, lets try and read the control byte.. we want to make sure its a valid value first
		if( !IsValidControl( controlByte ) )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: Invalid header control byte found!" );
			Output.Error = true;
			return Output;
		}

		// Set the control byte
		Output.Func = static_cast< ArchiveControlByte >( controlByte );

		// If we have the start of a table, we need to read the other half as a table type, instead of a normal type
		if( Output.Func == ArchiveControlByte::TableStart )
		{
			if( !IsValidTableType( typeByte ) )
			{
				Console::WriteLine( "[ERROR] ArchiveReader: Invalid table type byte found!" );
				Output.Error = true;
				return Output;
			}

			Output.TableInfo = static_cast< ArchiveTableType >( typeByte );
		}
		else
		{
			if( !IsValidType( typeByte ) )
			{
				Console::WriteLine( "[ERROR] ArchiveReader: Invalid type byte found!" );
				Output.Error = true;
				return Output;
			}

			Output.Type = static_cast< ArchiveType >( typeByte );
		}

		return Output;
	}


	ArchiveReader::ReadResult ArchiveReader::ReadValueImpl( ArchiveType inType, uint8 inByteCount, std::vector< byte >::iterator& outBeginIter, std::vector< byte >::iterator& outEndIter )
	{
		// Validate byte count is not 0
		if( inByteCount == 0 )
			return ReadResult::BadHeader;

		// Check state
		if( m_State != State::Reading )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: Attempt to read value outside of reading state! (EndOfBuffer?)" );
			return ReadResult::EndOfBuffer;
		}

		// First, lets pop the next header byte off the stack
		HeaderByte headerInfo = PeekHeaderByte();
		if( headerInfo.Error )
		{
			PopHeaderByte();
			return ReadResult::BadHeader;
		}
		else if( headerInfo.Func == ArchiveControlByte::HeaderEnd )
		{
			// Hit the end of the header, so lets close up
			Close();
			return ReadResult::EndOfBuffer;
		}
		else if( headerInfo.Type != inType )
		{
			return ReadResult::BadType;
		}

		// So we know we have a valid header byte, with the correct type
		// Lets advance the header position, and read the byte we want from the content section
		PopHeaderByte();

		auto endIter = m_ContentPosition;
		std::advance( endIter, inByteCount );

		if( endIter > m_ContentEnd )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: Tried to read value that went past the end of the content buffer!" );
			Close();
			return ReadResult::EndOfBuffer;
		}

		// Now, lets just return the iterators for this data to the caller
		outBeginIter = m_ContentPosition;
		outEndIter = endIter;

		std::advance( m_ContentPosition, inByteCount );
		return ReadResult::Success;
	}



	ArchiveReader::ReadResult ArchiveReader::ReadValue_Internal( float& Out, std::function< bool( const float& ) > fCheckValue /* = nullptr */ )
	{
		// Lets validate this read and get the iterators to where the float is in the data section
		std::vector< byte >::iterator floatStart, floatEnd;
		auto Result = ReadValueImpl( ArchiveType::Float, 4, floatStart, floatEnd );

		if( Result != ReadResult::Success )
		{
			// Failed to read.. everything should already be taken care of though
			return Result;
		}

		if( !Binary::DeserializeFloat( floatStart, floatEnd, Out, m_ByteOrder == ByteOrder::LittleEndian ) )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: Failed to deserialize float from archive!" );
			return ReadResult::ReadFailed;
		}

		if( fCheckValue && !fCheckValue( Out ) )
		{
			return ReadResult::OutOfRange;
		}

		return ReadResult::Success;
	}

	ArchiveReader::ReadResult ArchiveReader::ReadValue_Internal( double& Out, std::function< bool( const double& ) > fCheckValue /* = nullptr */ )
	{
		// Validate header, and get iterators to the data we need to deserialize
		std::vector< byte >::iterator doubleStart, doubleEnd;
		auto Result = ReadValueImpl( ArchiveType::Double, 8, doubleStart, doubleEnd );

		if( Result != ReadResult::Success )
		{
			return Result;
		}

		if( !Binary::DeserializeDouble( doubleStart, doubleEnd, Out, m_ByteOrder == ByteOrder::LittleEndian ) )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: failed to deserialize double from archive!" );
			return ReadResult::ReadFailed;
		}

		if( fCheckValue && !fCheckValue( Out ) )
		{
			return ReadResult::OutOfRange;
		}

		return ReadResult::Success;
	}

	ArchiveReader::ReadResult ArchiveReader::ReadValue_Internal( bool& Out, std::function< bool( const bool& ) > fCheckValue /* = nullptr */ )
	{
		// Validate header, and get iterators to the data we need to deserialize
		std::vector< byte >::iterator boolStart, boolEnd;
		auto Result = ReadValueImpl( ArchiveType::Boolean, 1, boolStart, boolEnd );

		if( Result != ReadResult::Success )
		{
			return Result;
		}

		Binary::DeserializeBoolean( boolStart, Out );

		if( fCheckValue && !fCheckValue( Out ) )
		{
			return ReadResult::OutOfRange;
		}

		return ReadResult::Success;
	}

	ArchiveReader::ReadResult ArchiveReader::ReadValue_Internal( int8& Out, std::function< bool( const int8& ) > fCheckValue /* = nullptr */ )
	{
		// Validate header, and get iterators to the data we need to deserialize
		std::vector< byte >::iterator dataStart, dataEnd;
		auto Result = ReadValueImpl( ArchiveType::Int8, 1, dataStart, dataEnd );

		if( Result != ReadResult::Success )
		{
			return Result;
		}

		Binary::DeserializeInt8( dataStart, Out );

		if( fCheckValue && !fCheckValue( Out ) )
		{
			return ReadResult::OutOfRange;
		}

		return ReadResult::Success;
	}


	ArchiveReader::ReadResult ArchiveReader::ReadValue_Internal( uint8& Out, std::function< bool( const uint8& ) > fCheckValue /* = nullptr */ )
	{
		// Validate header, and get iterators to the data we need to deserialize
		std::vector< byte >::iterator dataStart, dataEnd;
		auto Result = ReadValueImpl( ArchiveType::UInt8, 1, dataStart, dataEnd );

		if( Result != ReadResult::Success )
		{
			return Result;
		}

		Binary::DeserializeUInt8( dataStart, Out );

		if( fCheckValue && !fCheckValue( Out ) )
		{
			return ReadResult::OutOfRange;
		}

		return ReadResult::Success;
	}


	ArchiveReader::ReadResult ArchiveReader::ReadValue_Internal( int16& Out, std::function< bool( const int16& ) > fCheckValue /* = nullptr */ )
	{
		// Validate header, and get iterators to the data we need to deserialize
		std::vector< byte >::iterator dataStart, dataEnd;
		auto Result = ReadValueImpl( ArchiveType::Int16, 2, dataStart, dataEnd );

		if( Result != ReadResult::Success )
		{
			return Result;
		}

		if( !Binary::DeserializeInt16( dataStart, dataEnd, Out, m_ByteOrder == ByteOrder::LittleEndian ) )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: failed to deserialize int16 from archive!" );
			return ReadResult::ReadFailed;
		}

		if( fCheckValue && !fCheckValue( Out ) )
		{
			return ReadResult::OutOfRange;
		}

		return ReadResult::Success;
	}


	ArchiveReader::ReadResult ArchiveReader::ReadValue_Internal( uint16& Out, std::function< bool( const uint16& ) > fCheckValue /* = nullptr */ )
	{
		// Validate header, and get iterators to the data we need to deserialize
		std::vector< byte >::iterator dataStart, dataEnd;
		auto Result = ReadValueImpl( ArchiveType::UInt16, 2, dataStart, dataEnd );

		if( Result != ReadResult::Success )
		{
			return Result;
		}

		if( !Binary::DeserializeUInt16( dataStart, dataEnd, Out, m_ByteOrder == ByteOrder::LittleEndian ) )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: failed to deserialize uint16 from archive!" );
			return ReadResult::ReadFailed;
		}

		if( fCheckValue && !fCheckValue( Out ) )
		{
			return ReadResult::OutOfRange;
		}

		return ReadResult::Success;
	}


	ArchiveReader::ReadResult ArchiveReader::ReadValue_Internal( int32& Out, std::function< bool( const int32& ) > fCheckValue /* = nullptr */ )
	{
		// Validate header, and get iterators to the data we need to deserialize
		std::vector< byte >::iterator dataStart, dataEnd;
		auto Result = ReadValueImpl( ArchiveType::Int32, 4, dataStart, dataEnd );

		if( Result != ReadResult::Success )
		{
			return Result;
		}

		if( !Binary::DeserializeInt32( dataStart, dataEnd, Out, m_ByteOrder == ByteOrder::LittleEndian ) )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: failed to deserialize int32 from archive!" );
			return ReadResult::ReadFailed;
		}

		if( fCheckValue && !fCheckValue( Out ) )
		{
			return ReadResult::OutOfRange;
		}

		return ReadResult::Success;
	}


	ArchiveReader::ReadResult ArchiveReader::ReadValue_Internal( uint32& Out, std::function< bool( const uint32& ) > fCheckValue /* = nullptr */ )
	{
		// Validate header, and get iterators to the data we need to deserialize
		std::vector< byte >::iterator dataStart, dataEnd;
		auto Result = ReadValueImpl( ArchiveType::UInt32, 4, dataStart, dataEnd );

		if( Result != ReadResult::Success )
		{
			return Result;
		}

		if( !Binary::DeserializeUInt32( dataStart, dataEnd, Out, m_ByteOrder == ByteOrder::LittleEndian ) )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: failed to deserialize int32 from archive!" );
			return ReadResult::ReadFailed;
		}

		if( fCheckValue && !fCheckValue( Out ) )
		{
			return ReadResult::OutOfRange;
		}

		return ReadResult::Success;
	}


	ArchiveReader::ReadResult ArchiveReader::ReadValue_Internal( int64& Out, std::function< bool( const int64& ) > fCheckValue /* = nullptr */ )
	{
		// Validate header, and get iterators to the data we need to deserialize
		std::vector< byte >::iterator dataStart, dataEnd;
		auto Result = ReadValueImpl( ArchiveType::Int64, 8, dataStart, dataEnd );

		if( Result != ReadResult::Success )
		{
			return Result;
		}

		if( !Binary::DeserializeInt64( dataStart, dataEnd, Out, m_ByteOrder == ByteOrder::LittleEndian ) )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: failed to deserialize int64 from archive!" );
			return ReadResult::ReadFailed;
		}

		if( fCheckValue && !fCheckValue( Out ) )
		{
			return ReadResult::OutOfRange;
		}

		return ReadResult::Success;
	}


	ArchiveReader::ReadResult ArchiveReader::ReadValue_Internal( uint64& Out, std::function< bool( const uint64& ) > fCheckValue /* = nullptr */ )
	{
		// Validate header, and get iterators to the data we need to deserialize
		std::vector< byte >::iterator dataStart, dataEnd;
		auto Result = ReadValueImpl( ArchiveType::UInt64, 8, dataStart, dataEnd );

		if( Result != ReadResult::Success )
		{
			return Result;
		}

		if( !Binary::DeserializeUInt64( dataStart, dataEnd, Out, m_ByteOrder == ByteOrder::LittleEndian ) )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: failed to deserialize uint64 from archive!" );
			return ReadResult::ReadFailed;
		}

		if( fCheckValue && !fCheckValue( Out ) )
		{
			return ReadResult::OutOfRange;
		}

		return ReadResult::Success;
	}


	uint32 ArchiveReader::DetermineOffsetImpl( byte inHeader )
	{
		// Basically, we want to read a header byte and determine how much data this will use in the main body
		byte TypeInfo = inHeader & 0b00001111;
		
		switch( TypeInfo )
		{
		case (byte) ArchiveType::Int8:
		case (byte) ArchiveType::UInt8:
		case (byte) ArchiveType::Boolean:
			return 1;
		case (byte) ArchiveType::Int16:
		case (byte) ArchiveType::UInt16:
			return 2;
		case (byte) ArchiveType::Int32:
		case (byte) ArchiveType::UInt32:
		case (byte) ArchiveType::Float:
			return 4;
		case (byte) ArchiveType::Int64:
		case (byte) ArchiveType::UInt64:
		case (byte) ArchiveType::Double:
			return 8;
		case (byte) ArchiveType::Object:
		case (byte) ArchiveType::Table:
		case (byte) ArchiveType::String: // TODO
		default:
			return 0;
		}
	}


	ArchiveReader::ReadResult ArchiveReader::ReadComplexTypeStart( ArchiveType inType, ArchiveControlByte startByte, ArchiveTableType optTableType /* = ArchiveTableType::None */ )
	{
		// First, ensure we are in the reading state
		if( m_State != State::Reading )
		{
			return ReadResult::EndOfBuffer;
		}

		// Read the next two header bytes
		HeaderByte firstByte	= PeekHeaderByte( 0 );
		HeaderByte secondByte	= PeekHeaderByte( 1 );

		if( firstByte.Error || secondByte.Error )
		{
			return ReadResult::BadHeader;
		}

		// Ensure we didnt hit the end of the header
		if( firstByte.Func == ArchiveControlByte::HeaderEnd || secondByte.Func == ArchiveControlByte::HeaderEnd )
		{
			return ReadResult::EndOfBuffer;
		}

		// Ensure the first byte has the proper type info
		if( firstByte.Type != inType )
		{
			return ReadResult::BadType;
		}

		// Ensure the second byte has the proper start code
		if( secondByte.Func != startByte )
		{
			return ReadResult::BadType;
		}

		// If a table type was specified, check if that matches as well
		if( optTableType != ArchiveTableType::None && secondByte.TableInfo != optTableType )
		{
			return ReadResult::BadType;
		}

		// Pop these two bytes off the header, so we start reading at the first data member
		PopHeaderByte( 2 );

		return ReadResult::Success;
	}


	ArchiveReader::ReadResult ArchiveReader::SeekComplexTypeEnd( ArchiveControlByte endByte )
	{
		uint32 SeekOffset = 0;
		std::vector< byte >::iterator EndBytePosition;

		for( auto It = m_HeaderPosition;; )
		{
			// Check if we ran past the end of the header
			if( It >= m_HeaderEnd )
			{
				return ReadResult::EndOfBuffer;
			}

			// Read byte components in
			byte ControlCode	= *It & 0b11110000;
			byte TypeCode		= *It & 0b00001111;

			// Check for end of header byte
			if( ControlCode == (byte)ArchiveControlByte::HeaderEnd )
			{
				return ReadResult::EndOfBuffer;
			}
			else if( ControlCode == (byte)endByte )
			{
				EndBytePosition = It;
				break;
			}
			else
			{
				// If we have a value type, then summate the number of bytes it takes up
				// so when were done seeking, we can advance the data iterator to the end of this complex type
				SeekOffset += DetermineOffsetImpl( TypeCode );
				It++;
			}
		}

		// If we made it to this point, we found the control code and didnt hit the end of the header buffer
		// So, lets advance our member iterators to the end of this complex type
		m_HeaderPosition = EndBytePosition;
		std::advance( m_HeaderPosition, 1 );
		std::advance( m_ContentPosition, SeekOffset );

		return ReadResult::Success;
	}


	ArchiveReader::ReadResult ArchiveReader::ReadValue_Internal( ISerializable& In )
	{
		// First, lets try and read the header info for this object
		auto Result = ReadComplexTypeStart( ArchiveType::Object, ArchiveControlByte::ObjStart );

		// Check what happened
		if( Result == ReadResult::BadHeader || Result == ReadResult::EndOfBuffer )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: Failed to read start bytes for serializable object.. closing!" );
			Close();
			return Result;
		}
		else if( Result != ReadResult::Success )
		{
			return Result;
		}

		// Now, we need to update the current block type
		auto previousBlock = m_Block;
		m_Block = BlockType::Object;

		// Read in the object
		In.Deserialize( *this );

		// Finally, we need to find the end of this object block, and advance all iterators to match
		// this position.. for this we have a helper function
		auto SeekResult = SeekComplexTypeEnd( ArchiveControlByte::ObjEnd );

		if( SeekResult != ReadResult::Success )
		{
			Console::WriteLine( "[ERROR] ArchiveReader: Failed to seek end of object! Hit end of buffer? Closing..." );
			Close();
			return SeekResult;
		}

		// Set block type back, and return success!
		m_Block = previousBlock;
		return ReadResult::Success;
	}

	void ArchiveReader::Close()
	{
		// Close the reader, update state, set block type back to base
		m_State				= State::EndOfBuffer;
		m_Block				= BlockType::Base;
		m_HeaderPosition	= m_HeaderEnd;
		m_ContentPosition	= m_ContentEnd;
	}



}