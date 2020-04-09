/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Serialization.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include "Hyperion/Hyperion.h"
#include <vector>
#include <map>
#include <functional>


namespace Hyperion
{
	/*
		Forward Declarations
	*/
	class ISerializable;

	/*
		Compile Time Checks
	*/
	template< class _Ty >
	inline constexpr bool is_serializable_v =
		std::is_arithmetic_v< _Ty > || std::is_base_of_v< ISerializable, _Ty >;

	template< class _Ty >
	struct is_serializable : std::bool_constant< is_serializable_v< _Ty > > {};

	enum class ArchiveType : uint8
	{
		Boolean		= 0b00000000,
		Float		= 0b00000001,
		Double		= 0b00000010,
		Int8		= 0b00000011,
		Int16		= 0b00000100,
		Int32		= 0b00000101,
		Int64		= 0b00000110,
		UInt8		= 0b00000111,
		UInt16		= 0b00001000,
		UInt32		= 0b00001001,
		UInt64		= 0b00001010,
		String		= 0b00001011,
		Object		= 0b00001100,
		Table		= 0b00001101
	};

	enum class ArchiveControlByte : uint8
	{
		Value			= 0b00000000,
		HeaderStart		= 0b10000000,
		HeaderEnd		= 0b01000000,
		TableStart		= 0b11000000,
		TableEnd		= 0b00100000,
		TableKey		= 0b10100000,
		TableValue		= 0b11100000,
		ObjStart		= 0b00010000,
		ObjEnd			= 0b10010000,
		ObjData			= 0b10110000
	};

	enum class ArchiveTableType : uint8
	{
		None	= 0b00000000,
		Vector	= 0b00000001,
		Map		= 0b00000010
	};


	class ArchiveWriter
	{

	public:

		/*
			Constructor/Destructor
		*/
		ArchiveWriter() = delete;
		ArchiveWriter( std::vector< byte >& TargetBuffer, ByteOrder Endianess = ByteOrder::BigEndian );
		~ArchiveWriter();

	protected:

		/*
			Data Members
		*/
		std::vector< byte >& m_TargetBuffer;
		const ByteOrder m_ByteOrder;

		std::vector< byte > m_HeaderBuffer;
		std::vector< byte > m_ContentBuffer;

		enum class BlockType
		{
			Base,
			Table,
			Object
		};

		BlockType m_CurrentBlock;

		/*
			Helper Methods
		*/
		void WriteHeaderByte( ArchiveControlByte Control, ArchiveType Type );
		void WriteTableHeaderByte( bool bIsStart, ArchiveTableType Type );
		void Impl_WriteHeader( ArchiveType Type, bool bIsKey );

		enum WriteParam
		{
			StandaloneValue,
			TableKey,
			TableValue
		};

		void WriteImpl( const float& In, bool bIsKey = false );
		void WriteImpl( const double& In, bool bIsKey = false );
		void WriteImpl( const int8& In, bool bIsKey = false );
		void WriteImpl( const uint8& In, bool bIsKey = false );
		void WriteImpl( const int16& In, bool bIsKey = false );
		void WriteImpl( const uint16& In, bool bIsKey = false );
		void WriteImpl( const int32& In, bool bIsKey = false );
		void WriteImpl( const uint32& In, bool bIsKey = false );
		void WriteImpl( const int64& In, bool bIsKey = false );
		void WriteImpl( const uint64& In, bool bIsKey = false );
		void WriteImpl( const bool& In, bool bIsKey = false );
		void WriteImpl( ISerializable& In );

		template< typename T, std::enable_if_t< is_serializable< T >::value > >
		void WriteVector( const std::vector< T >& In )
		{
			// Store current block type so we can reset it when were done
			auto PreviousBlock = m_CurrentBlock;
			m_CurrentBlock = BlockType::Table;

			// If were in a table already, we need to write a value byte to say were starting a new table as a value
			if( PreviousBlock == BlockType::Table )
			{
				WriteHeaderByte( ArchiveControlByte::TableValue, ArchiveType::Table );
			}
			else if( PreviousBlock == BlockType::Object )
			{
				WriteHeaderByte( ArchiveControlByte::ObjData, ArchiveType::Table );
			}
			else
			{
				WriteHeaderByte( ArchiveControlByte::Value, ArchiveType::Table );
			}

			// Now, we need to write the vector start byte
			WriteTableHeaderByte( true, ArchiveTableType::Vector );

			// Loop through the contents and write them out to the buffers
			for( auto It = In.begin(); It != In.end(); It++ )
			{
				WriteImpl( *It );
			}

			// Write table end byte
			WriteTableHeaderByte( false, ArchiveTableType::Vector );

			// Reset block type
			m_CurrentBlock = PreviousBlock;
		}

		template< typename T, typename U, std::enable_if_t< std::is_integral< T >::value && is_serializable< U >::value > >
		void WriteMap( const std::map< T, U >& In )
		{
			// Store current block type so we can reset it when were done
			auto PreviousBlock = m_CurrentBlock;
			m_CurrentBlock = BlockType::Table;

			// If were in a table already, we need to write a value byte to say were starting a new table as a value
			if( PreviousBlock == BlockType::Table )
			{
				WriteHeaderByte( ArchiveControlByte::TableValue, ArchiveType::Table );
			}
			else if( PreviousBlock == BlockType::Object )
			{
				WriteHeaderByte( ArchiveControlByte::ObjData, ArchiveType::Table );
			}
			else
			{
				WriteHeaderByte( ArchiveControlByte::Value, ArchiveType::Table );
			}

			// Write table start byte in header
			WriteTableHeaderByte( true, ArchiveTableType::Map );

			for( auto It = In.begin(); It != In.end(); It++ )
			{
				// Write out the key first
				WriteImpl( It->first, true );

				// Then, write out the value
				WriteImpl( It->second );
			}

			// Write table end byte in header
			WriteTableHeaderByte( false, ArchiveTableType::Map );

			// Reset block type
			m_CurrentBlock = PreviousBlock;
		}

		// We dont want to have any implicit conversions going on when calling these functions,
		// so as a result, were going to delete any implementations were not using
		template< typename T >
		void WriteImpl( const T& In ) = delete;


	public:

		/*
			Write Methods
		*/

		inline void WriteFloat( const float& In )					{ WriteImpl( In ); }
		inline void WriteDouble( const double& In )					{ WriteImpl( In ); }
		inline void WriteInt8( const int8& In )						{ WriteImpl( In ); }
		inline void WriteUInt8( const uint8& In )					{ WriteImpl( In ); }
		inline void WriteInt16( const int16& In )					{ WriteImpl( In ); }
		inline void WriteUInt16( const uint16& In )					{ WriteImpl( In ); }
		inline void WriteInt32( const int32& In )					{ WriteImpl( In ); }
		inline void WriteUInt32( const uint32& In )					{ WriteImpl( In ); }
		inline void WriteInt64( const int64& In )					{ WriteImpl( In ); }
		inline void WriteUInt64( const uint64& In )					{ WriteImpl( In ); }
		inline void WriteBoolean( const bool& In )					{ WriteImpl( In ); }
		inline void WriteSerializable( ISerializable& In )			{ WriteImpl( In ); }
		
		/*
			Collection Write Methods
		*/
		template< typename T >
		inline void WriteVector( const std::vector< T >& In )	{ WriteImpl( In ); }

		template< typename T, typename U >
		inline void WriteMap( const std::map< T, U >& In )		{ WriteImpl( In ); }


		/*
			'<<' Operator Overloads
		*/
		inline ArchiveWriter& operator<<( const float& In )				{ WriteFloat( In );		return *this; }
		inline ArchiveWriter& operator<<( const double& In )			{ WriteDouble( In );	return *this; }
		inline ArchiveWriter& operator<<( const int8& In )				{ WriteInt8( In );		return *this; }
		inline ArchiveWriter& operator<<( const uint8& In )				{ WriteUInt8( In );		return *this; }
		inline ArchiveWriter& operator<<( const int16& In )				{ WriteInt16( In );		return *this; }
		inline ArchiveWriter& operator<<( const uint16& In )			{ WriteUInt16( In );	return *this; }
		inline ArchiveWriter& operator<<( const int32& In )				{ WriteInt32( In );		return *this; }
		inline ArchiveWriter& operator<<( const uint32& In )			{ WriteUInt32( In );	return *this; }
		inline ArchiveWriter& operator<<( const int64& In )				{ WriteInt64( In );		return *this; }
		inline ArchiveWriter& operator<<( const uint64& In )			{ WriteUInt64( In );	return *this; }
		inline ArchiveWriter& operator<<( const bool& In )				{ WriteBoolean( In );	return *this; }
		inline ArchiveWriter& operator<<( ISerializable& In )			{ WriteSerializable( In ); return *this; }

		template< typename T, std::enable_if_t< is_serializable< T >::value > >
		inline ArchiveWriter& operator<<( const std::vector< T >& In ) { WriteVector( In ); return *this; }

		template< typename T, typename U, std::enable_if_t< is_serializable< T >::value && is_serializable< U >::value > >
		inline ArchiveWriter& operator<<( const std::map< T, U >& In ) { WriteMap( In ); return *this; }



		/*
			Methods
		*/
		void Clear();
		void Flush();

		size_t GetHeaderSize() const;
		size_t GetContentSize() const;

		size_t GetAllocatedHeaderSize() const;
		size_t GetAllocatedContentSize() const;

		void AllocateHeaderSpace( size_t AdditionalSpace );
		void AllocateContentSpace( size_t AdditionalSpace );

	};

	class ArchiveReader
	{

	public:

		/*
			Constructor/Destructor
		*/
		ArchiveReader() = delete;
		ArchiveReader( std::vector< byte >& SourceBuffer );
		~ArchiveReader();

		enum class ReadResult
		{
			BadType,
			EndOfBuffer,
			BadHeader,
			OutOfRange,
			ReadFailed,
			Success
		};

	protected:

		/*
			Data Members
		*/
		std::vector< byte >& m_SourceBuffer;
		ByteOrder m_ByteOrder;

		std::vector< byte >::iterator m_HeaderBegin;
		std::vector< byte >::iterator m_HeaderPosition;
		std::vector< byte >::iterator m_HeaderEnd;
		std::vector< byte >::iterator m_ContentBegin;
		std::vector< byte >::iterator m_ContentPosition;
		std::vector< byte >::iterator m_ContentEnd;

		enum class State
		{
			Invalid,
			Reading,
			EndOfBuffer

		} m_State;

		enum class BlockType
		{
			Base,
			Table,
			Object

		} m_Block;

	private:

		/*
			Helper Methods
		*/
		bool IngestData();

		struct HeaderByte
		{
			ArchiveType Type;
			ArchiveControlByte Func;
			ArchiveTableType TableInfo;
			bool Error;
		};

		bool IsValidType( byte In );
		bool IsValidControl( byte In );
		bool IsValidTableType( byte In );
		HeaderByte PeekHeaderByte( uint32 Offset = 0 );
		void PopHeaderByte( uint32 Count = 1 );
		ReadResult ReadValueImpl( ArchiveType inType, uint8 inByteCount, std::vector< byte >::iterator& outBeginIter, std::vector< byte >::iterator& outEndIter );
		uint32 DetermineOffsetImpl( byte inHeaderByte );

		ReadResult ReadComplexTypeStart( ArchiveType inType, ArchiveControlByte startCode, ArchiveTableType optTableType = ArchiveTableType::None );
		ReadResult SeekComplexTypeEnd( ArchiveControlByte endCode );

		template< typename _Ty >
		inline bool DoesTypeCodeMatch( ArchiveType inType ) { return inType == ArchiveType::Object && std::is_base_of< ISerializable, _Ty >::value; }

		template< typename _Ty, std::enable_if_t< std::is_aggregate< _Ty >::value > >
		inline bool DoesTypeCodeMatch( ArchiveType inType ) { return inType == ArchiveType::Table; }

		template<>
		inline bool DoesTypeCodeMatch< bool >( ArchiveType inType ) { return inType == ArchiveType::Boolean; }

		template<>
		inline bool DoesTypeCodeMatch< float >( ArchiveType inType ) { return inType == ArchiveType::Float; }

		template<>
		inline bool DoesTypeCodeMatch< double >( ArchiveType inType ) { return inType == ArchiveType::Double; }

		template<>
		inline bool DoesTypeCodeMatch< int8 >( ArchiveType inType ) { return inType == ArchiveType::Int8; }

		template<>
		inline bool DoesTypeCodeMatch< uint8 >( ArchiveType inType ) { return inType == ArchiveType::UInt8; }

		template<>
		inline bool DoesTypeCodeMatch< int16 >( ArchiveType inType ) { return inType == ArchiveType::Int16; }

		template<>
		inline bool DoesTypeCodeMatch< uint16 >( ArchiveType inType ) { return inType == ArchiveType::UInt16; }

		template<>
		inline bool DoesTypeCodeMatch< int32 >( ArchiveType inType ) { return inType == ArchiveType::Int32; }

		template<>
		inline bool DoesTypeCodeMatch< uint32 >( ArchiveType inType ) { return inType == ArchiveType::UInt32; }

		template<>
		inline bool DoesTypeCodeMatch< int64 >( ArchiveType inType ) { return inType == ArchiveType::Int64; }

		template<>
		inline bool DoesTypeCodeMatch< uint64 >( ArchiveType inType ) { return inType == ArchiveType::UInt64; }

		ReadResult ReadValue_Internal( float& Out, std::function< bool( const float& ) > fCheckValue = nullptr );
		ReadResult ReadValue_Internal( double& Out, std::function< bool( const double& ) > fCheckValue = nullptr );
		ReadResult ReadValue_Internal( int8& Out, std::function< bool( const int8& ) > fCheckValue = nullptr );
		ReadResult ReadValue_Internal( uint8& Out, std::function< bool( const uint8& ) > fCheckValue = nullptr );
		ReadResult ReadValue_Internal( int16& Out, std::function< bool( const int16& ) > fCheckValue = nullptr );
		ReadResult ReadValue_Internal( uint16& Out, std::function< bool( const uint16& ) > fCheckValue = nullptr );
		ReadResult ReadValue_Internal( int32& Out, std::function< bool( const int32& ) > fCheckValue = nullptr );
		ReadResult ReadValue_Internal( uint32& Out, std::function< bool( const uint32& ) > fCheckValue = nullptr );
		ReadResult ReadValue_Internal( int64& Out, std::function< bool( const int64& ) > fCheckValue = nullptr );
		ReadResult ReadValue_Internal( uint64& Out, std::function< bool( const uint64& ) > fCheckValue = nullptr );
		ReadResult ReadValue_Internal( bool& Out, std::function< bool( const bool& ) > fCheckValue = nullptr );
		ReadResult ReadValue_Internal( ISerializable& Out );
		
		template< typename T, std::enable_if_t< is_serializable< T >::value > >
		ReadResult ReadValue_Internal( std::vector< T >& Out, std::function< bool( const T& ) > fCheckElement = nullptr )
		{
			// Clear the output vector
			Out.clear();

			// First, lets read the complex data start byte sequence
			auto StartResult = ReadComplexTypeStart( ArchiveType::Table, ArchiveControlByte::TableStart, ArchiveTableType::Vector );
			if( StartResult == ReadResult::BadHeader || StartResult == ReadResult::EndOfBuffer )
			{
				Console::WriteLine( "[ERROR] ArchiveReader: Failed to read start of vector.. end of buffer? Closing..." );
				Close();
				return StartResult;
			}
			else if( StartResult != ReadResult::Success )
			{
				return StartResult;
			}

			// Now, lets update the block type
			auto previousBlock = m_Block;
			m_Block = BlockType::Table;

			// Loop through, and read the values until we hit the table end byte
			bool bError = false;

			for( ;; )
			{
				auto valueInfo = PeekHeaderByte( 0 );
				if( valueInfo.Error )
				{
					Console::WriteLine( "[ERROR] ArchiveReader: Failed to read vector contents... Hit end of buffer? Non-recoverable..." );
					Close();
					return ReadResult::EndOfBuffer;
				}

				// Check for the end of the vector
				if( valueInfo.Func == ArchiveControlByte::TableEnd )
				{
					break;
				}

				if( valueInfo.Func != ArchiveControlByte::TableValue )
				{
					Console::WriteLine( "[ERROR] ArchiveReader: Failed to read vector contents... control code wasnt TABLE_VALUE..." );
					Close();
					return ReadResult::BadHeader;
				}

				// We want to ensure the type matches the type we expect
				if( !DoesTypeCodeMatch< T >( valueInfo.Type ) )
				{
					Console::WriteLine( "[ERROR] ArchiveReader: Failed to read vector contents.. type doesnt match expected type!" );
					bError = true;
					break;
				}

				// Now, we need to read this value into the vector were going to output
				Out.emplace_back( T() );
				auto ElemResult = ReadValue_Internal( Out.back() );

				// Check if this was successful
				if( ElemResult != ReadResult::Success )
				{
					Console::WriteLine( "[ERROR] ArchiveReader: Failed to read vector contents.. element read failed!" );
					bError = true;
					break;
				}
			}

			// Now, were going to seek to the end of the vector.. this helps us recover in case of an error
			auto SeekResult = SeekComplexTypeEnd( ArchiveControlByte::TableEnd );
			if( SeekResult != ReadResult::Success )
			{
				Console::WriteLine( "[ERROR] ArchiveReader: Failed to skeep end of vector! Closing..." );
				Close();
				return ReadResult::BadHeader;
			}

			// So now, we should be all set to continue reading, set the block type back and return success
			m_Block = previousBlock;
			return bError ? ReadResult::BadType : ReadResult::Success;
		}

		template< typename T, typename U, std::enable_if_t< std::is_integral<  T >::value && is_serializable< U >::value > >
		ReadResult ReadValue_Internal( std::map< T, U >& Out, std::function< bool( const std::pair< T, U >& ) > fCheckElement = nullptr )
		{
			// Clear the output map
			Out.clear();

			// First read the complex data start byte sequence
			auto StartResult = ReadComplexTypeStart( ArchiveType::Table, ArchiveControlByte::TableStart, ArchiveTableType::Map );
			if( StartResult == ReadResult::BadHeader || StartResult == ReadResult::EndOfBuffer )
			{
				Console::WriteLine( "[ERROR] ArchiveReader: Failed to read start of map.. closing..." );
				Close();
				return StartResult;
			}
			else if( StartResult != ReadResult::Success )
			{
				return StartResult;
			}

			// Update block type
			auto previousBlock = m_Block;
			m_Block = BlockType::Table;

			// Loop through and read contents
			bool bError = false;
			for( ;; )
			{
				auto keyInfo = PeekHeaderByte( 0 );
				auto valueInfo = PeekHeaderByte( 1 );

				// Check if the bytes were read in properly
				if( keyInfo.Error || valueInfo.Error )
				{
					Console::WriteLine( "[ERROR] Failed to read map contents.. Hit end of buffer? Closing..." );
					Close();
					return ReadResult::EndOfBuffer;
				}

				// Check for the end of the map
				if( keyInfo.Func == ArchiveControlByte::TableEnd || valueInfo.Func == ArchiveControlByte::TableEnd )
				{
					break;
				}

				if( keyInfo.Func != ArchiveControlByte::TableKey || valueInfo.Func != ArchiveControlByte::TableValue )
				{
					Console::WriteLine( "[ERROR] Failed to read map contents... key/value pairs arent aligned? Closing..." );
					Close();
					return ReadResult::BadHeader;
				}

				// Ensure types match properly
				if( !DoesTypeCodeMatch< T >( keyInfo.Type ) || !DoesTypeCodeMatch< U >( valueInfo.Type ) )
				{
					Console::WriteLine( "[ERROR] Failed to read map contents... key/value types arent correct! Skipping..." );
					bError = true;
					break;
				}

				// Now lets read this key/value pair
				T Key = T();
				auto KeyResult = ReadValue_Internal( Key );
				if( KeyResult != ReadResult::Success )
				{
					Console::WriteLine( "[ERROR] ArchiveReader: Failed to read key for map! Skipping..." );
					bError = true;
					break;
				}

				auto ValueResult = ReadValue_Internal( Out[ Key ] );
				if( ValueResult != ReadResult::Success )
				{
					Console::WriteLine( "[ERROR] ArchiveReader: Failed to read value for map! Skipping..." );
					bError = true;
					break;
				}
			}

			// Ensure we reached the end of the map, by seeking to the table_end header byte
			auto SeekResult = SeekComplexTypeEnd( ArchiveControlByte::TableEnd );
			if( SeekResult != ReadResult::Success )
			{
				Console::WriteLine( "[ERROR] ArchiveReader: Failed to seek to end of map! Closing..." );
				Close();
				return ReadResult::BadHeader;
			}

			// Restore block, and return result
			m_Block = previousBlock;
			return bError ? ReadResult::BadType : ReadResult::Success;
		}

	public:

		/*
			Read Functions
		*/
		inline ReadResult ReadFloat( float& Out, std::function< bool( const float& ) > fCheckValue = nullptr )		{ return ReadValue_Internal( Out, fCheckValue ); }
		inline ReadResult ReadDouble( double& Out, std::function< bool( const double& ) > fCheckValue = nullptr )	{ return ReadValue_Internal( Out, fCheckValue ); }
		inline ReadResult ReadInt8( int8& Out, std::function< bool( const int8& ) > fCheckValue = nullptr )			{ return ReadValue_Internal( Out, fCheckValue ); }
		inline ReadResult ReadUInt8( uint8& Out, std::function< bool( const uint8& ) > fCheckValue = nullptr )		{ return ReadValue_Internal( Out, fCheckValue ); }
		inline ReadResult ReadInt16( int16& Out, std::function< bool( const int16& ) > fCheckValue = nullptr )		{ return ReadValue_Internal( Out, fCheckValue ); }
		inline ReadResult ReadUInt16( uint16& Out, std::function< bool( const uint16& ) > fCheckValue = nullptr )	{ return ReadValue_Internal( Out, fCheckValue ); }
		inline ReadResult ReadInt32( int32& Out, std::function< bool( const int32& ) > fCheckValue = nullptr )		{ return ReadValue_Internal( Out, fCheckValue ); }
		inline ReadResult ReadUInt32( uint32& Out, std::function< bool( const uint32& ) > fCheckValue = nullptr )	{ return ReadValue_Internal( Out, fCheckValue ); }
		inline ReadResult ReadInt64( int64& Out, std::function< bool( const int64& ) > fCheckValue = nullptr )		{ return ReadValue_Internal( Out, fCheckValue ); }
		inline ReadResult ReadUInt64( uint64& Out, std::function< bool( const uint64& ) > fCheckValue = nullptr )	{ return ReadValue_Internal( Out, fCheckValue ); }
		inline ReadResult ReadBoolean( bool& Out, std::function< bool( const bool& ) > fCheckValue = nullptr )		{ return ReadValue_Internal( Out, fCheckValue ); }
		inline ReadResult ReadSerializable( ISerializable& Out )													{ return ReadValue_Internal( Out ); }

		// Special methods for vectors and maps
		template< typename T, std::enable_if_t< is_serializable< T >::value > >
		inline ReadResult ReadVector( std::vector< T >& Out, std::function< bool( T& ) > fCheckElement = nullptr ) { return ReadValue_Internal( Out, fCheckElement ); }

		template< typename T, typename U, std::enable_if_t< std::is_integral< T >::value && is_serializable< U >::value > >
		inline ReadResult ReadMap( std::map< T, U >& Out, std::function< bool( std::pair< T&, U& > ) > fCheckElement = nullptr ) { return ReadValue_Internal( Out, fCheckElement ); }


		void Close();
	};



	class ISerializable
	{

	public:

		virtual void Serialize( ArchiveWriter& ) = 0;
		virtual bool Deserialize( ArchiveReader& ) = 0;

	};


}