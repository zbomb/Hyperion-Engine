/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Stream.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <vector>
#include <atomic>
#include <iostream>
#include <boost/interprocess/streams/vectorstream.hpp>

#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/String.h"
#include "Hyperion/Console/Console.h"


namespace Hyperion
{
	/*-----------------------------------------------------------------------------
		IDataSource Interface
	-----------------------------------------------------------------------------*/
	class IDataSource
	{

	public:

		virtual std::basic_streambuf< byte >* GetStreamBuffer() = 0;

		virtual bool CanWriteStream() const	= 0;
		virtual bool CanReadStream() const	= 0;
		virtual bool IsClamped() const = 0;
		virtual size_t GetClampedRangeBegin() const { return 0; }
		virtual size_t GetClampedRangeEnd() const { return 0; }
	};


	class GenericBuffer : public IDataSource
	{
		using _CharType = byte;
		using _VecType = std::vector< _CharType, std::allocator< _CharType > >;

	protected:

		boost::interprocess::basic_vectorbuf< _VecType > m_Buffer;
		const bool bAllowRead;
		const bool bAllowWrite;

	public:

		GenericBuffer( bool bCanRead = true, bool bCanWrite = true )
			:m_Buffer( ( bCanRead ? std::ios_base::out : 0x00 ) | ( bCanWrite ? std::ios_base::in : 0x00 ) ), bAllowRead( bCanRead ), bAllowWrite( bCanWrite )
		{}

		GenericBuffer( const std::vector< byte >& In, bool bCanRead = true, bool bCanWrite = true )
			: m_Buffer( In, ( bCanRead ? std::ios_base::out : 0x00 ) | ( bCanWrite ? std::ios_base::in : 0x00 ) ), bAllowRead( bCanRead ), bAllowWrite( bCanWrite )
		{
		}

		// For now.. no copying or assigning
		GenericBuffer( const GenericBuffer& ) = delete;
		GenericBuffer& operator=( const GenericBuffer& ) = delete;

		void Reserve( _VecType::size_type Size )
		{
			m_Buffer.reserve( Size );
		}

		void Clear()
		{
			m_Buffer.clear();
		}

		const _VecType& GetVector() const
		{
			return m_Buffer.vector();
		}

		virtual std::basic_streambuf< byte >* GetStreamBuffer() override final
		{
			return &m_Buffer;
		}

		virtual bool CanWriteStream() const override final
		{
			return bAllowWrite;
		}

		virtual bool CanReadStream() const override final
		{
			return bAllowRead;
		}

		virtual bool IsClamped() const override final
		{
			return false;
		}

		_VecType::size_type Size()
		{
			return m_Buffer.vector().size();
		}

	};


	class DataReader
	{

		using _StreamType = std::basic_istream< byte >;

	protected:

		IDataSource& m_Target;
		_StreamType m_Stream;
		const bool m_Alive;

	public:

		DataReader() = delete;
		DataReader( const DataReader& ) = delete;
		DataReader& operator=( const DataReader& ) = delete;

		DataReader( IDataSource& Target )
			: m_Target( Target ), m_Stream( Target.CanReadStream() ? Target.GetStreamBuffer() : nullptr ), m_Alive( Target.CanReadStream() )
		{
			if( !m_Alive )
			{
				Console::WriteLine( "[ERROR] DataReader: Attempt to read from a stream that doesnt allow reading!" );
			}

			if( Target.IsClamped() )
			{
				m_Stream.seekg( Target.GetClampedRangeBegin(), _StreamType::beg );
			}
		}

		DataReader( const std::unique_ptr< IDataSource >& Target )
			: DataReader( *Target )
		{}

		DataReader( const std::shared_ptr< IDataSource >& Target )
			: DataReader( *Target )
		{}

		template< typename T >
		DataReader( const std::unique_ptr< T >& Target )
			: DataReader( *Target )
		{}

		void SeekBegin( _StreamType::pos_type Offset = 0 )
		{
			if( m_Alive )
			{
				if( Offset < 0 )
				{
					Console::WriteLine( "[WARNING] DataReader: Attempt to seek before the begining of the stream?" );
					Offset = 0;
				}

				size_t finalOffset = (size_t) Offset;

				if( m_Target.IsClamped() )
				{
					finalOffset +=  m_Target.GetClampedRangeBegin();
					size_t endOffset = m_Target.GetClampedRangeEnd();

					if( endOffset > 0 && finalOffset > endOffset )
					{
						Console::WriteLine( "[WARNING] DataReader: Attempt to seek past the end of a clamped stream!" );
						finalOffset = endOffset;
					}
				}

				m_Stream.seekg( finalOffset, _StreamType::beg );
			}
		}

		void SeekEnd( _StreamType::pos_type Offset = 0 )
		{
			if( m_Alive )
			{
				if( Offset > 0 )
				{
					Console::WriteLine( "[WARNING] DataReader: Attempt to seek past the end of the stream?" );
					Offset = 0;
				}

				if( m_Target.IsClamped() )
				{
					size_t clampedEnd		= m_Target.GetClampedRangeEnd();
					size_t clampedBegin		= m_Target.GetClampedRangeBegin();

					if( clampedEnd > 0 )
					{
						// Seek back from the clamped 'end' position
						long long actualOffset = clampedEnd + Offset;
						if( (size_t) actualOffset < clampedBegin )
						{
							Console::WriteLine( "[WARNING] DataReader: Attempt to seek before the begining clamped stream range!" );
							actualOffset = clampedBegin;
						}

						m_Stream.seekg( actualOffset, _StreamType::beg );
					}
					else
					{
						// Ensure we dont seek before the begining
						m_Stream.seekg( Offset, _StreamType::end );
						if( (size_t) m_Stream.tellg() < clampedBegin )
						{
							Console::WriteLine( "[WARNING] DataReader: Attempt to seek before the begining clamped stream range!" );
							m_Stream.seekg( clampedBegin, _StreamType::beg );
						}
					}
				}
				else
				{
					m_Stream.seekg( Offset, _StreamType::end );
				}
			}
		}

		void SeekOffset( _StreamType::pos_type Offset )
		{
			SeekBegin( Offset );
		}

		_StreamType::pos_type GetOffset()
		{
			return m_Alive ? m_Stream.tellg() : static_cast< _StreamType::pos_type >( 0 );
		}

		enum class ReadResult
		{
			Fail,
			End,
			Success
		};

		ReadResult ReadBytes( std::vector< byte >& Output, size_t Count, size_t& outBytesRead )
		{
			if( !m_Alive )
			{
				return ReadResult::Fail;
			}

			auto res				= ReadResult::Success;
			bool bHitClampedEnd		= false;

			// First, if were clamped, we need to check if were going past the end of the stream
			if( m_Target.IsClamped() )
			{
				size_t currentOffset	= m_Stream.tellg();
				auto endRange			= m_Target.GetClampedRangeEnd();

				if( currentOffset + Count > endRange )
				{
					// Hit the end of the clamped stream, so we need to change the 'Count'
					Count = endRange - currentOffset;
					bHitClampedEnd = true;
				}
			}

			// We need to resize the vector to fit the expected number of bytes being read from the stream
			auto startSize = Output.size();
			auto newSize = startSize + Count;

			if( newSize < startSize )
			{
				// Overflow!
				Console::WriteLine( "[ERROR] DataReader: Attempt to read into a vector.. but the vector size overflowed!" );
				return ReadResult::Fail;
			}

			Output.resize( newSize );

			// Now we need to get a pointer to where we can start writing the data
			byte* insPtr = Output.data() + startSize;

			// Read from the stream...
			m_Stream.read( insPtr, static_cast< std::streamsize >( Count ) );

			// Check for success
			if( m_Stream )
			{
				outBytesRead = static_cast< size_t >( Count );
			}
			else
			{
				// Resize the vector to fit the data
				outBytesRead = static_cast< size_t >( m_Stream.gcount() );
				Output.resize( startSize + outBytesRead );

				// Determine reason for failure.. error or just eof
				if( m_Stream.eof() )
				{
					res = ReadResult::End;

					// Reset EOF bit so we can continue using this stream
					if( !m_Stream.bad() )
					{
						m_Stream.clear();
					}
				}
				else
				{
					res = ReadResult::Fail;
				}
			}

			if( res == ReadResult::Success && bHitClampedEnd )
			{
				res = ReadResult::End;
			}

			return res;
		}

		ReadResult ReadBytes( std::vector< byte >& Output, size_t Count )
		{
			size_t _dummy;
			return ReadBytes( Output, Count, _dummy );
		}

		ReadResult PeekBytes( std::vector< byte >& Output, size_t Count, size_t& outBytesRead )
		{
			auto cur_pos = GetOffset();
			auto res = ReadBytes( Output, Count, outBytesRead );
			SeekOffset( cur_pos );
			return res;
		}

		ReadResult PeekBytes( std::vector< byte >& Output, size_t Count )
		{
			size_t _dummy;
			return PeekBytes( Output, Count, _dummy );
		}

		_StreamType::pos_type Size()
		{
			if( !m_Alive )
			{
				return static_cast< _StreamType::pos_type >( 0 );
			}

			// Store current offset, and seek to the end of the stream
			auto curOffset = GetOffset();
			SeekEnd();

			// Since were at the end of the stream, we only need to get the offset from the begining to determine size
			auto outSize = GetOffset();

			// Reset offset to where we were before, and return the result
			SeekBegin( curOffset );
			return outSize;
		}
	};


	class DataWriter
	{

		using _StreamType = std::basic_ostream< byte >;
		using _Iter = std::vector< byte >::const_iterator;

	protected:

		IDataSource& m_Target;
		_StreamType m_Stream;
		const bool m_Alive;

	public:

		DataWriter() = delete;
		DataWriter( const DataWriter& ) = delete;
		DataWriter& operator=( const DataWriter& ) = delete;

		DataWriter( IDataSource& Target )
			: m_Target( Target ), m_Stream( Target.CanWriteStream() ? Target.GetStreamBuffer() : nullptr ), m_Alive( Target.CanWriteStream() )
		{
			if( !m_Alive )
			{
				Console::WriteLine( "[ERROR] DataWriter: Attempt to write to a stream that doesnt allow writing" );
			}
			else if( m_Target.IsClamped() )
			{
				Console::WriteLine( "[ERROR] DataWriter: Attempt to write to a clamped stream?? This stream should not allow writing!" );
			}
		}

		DataWriter( const std::unique_ptr< IDataSource >& Target )
			: DataWriter( *Target )
		{}

		DataWriter( const std::shared_ptr< IDataSource >& Target )
			: DataWriter( *Target )
		{}

		template< typename T >
		DataWriter( const std::unique_ptr< T >& Target )
			: DataWriter( *Target )
		{}

		~DataWriter()
		{
			if( m_Alive )
			{
				m_Stream.flush();
			}
		}

		bool WriteBytes( _Iter Start, _Iter End )
		{
			if( !m_Alive )
			{
				return false;
			}

			auto byteCount	= std::distance( Start, End );
			auto bytePtr	= std::addressof( *Start );

			// Validate iter distance
			if( byteCount <= 0 )
				return true;

			// Write to stream
			m_Stream.write( bytePtr, static_cast< std::streamsize >( byteCount ) );

			// Check for failure
			return m_Stream ? true : false;
		}

		bool WriteBytes( const std::vector< byte >& In )
		{
			return WriteBytes( In.begin(), In.end() );
		}

		bool WriteBytes( const std::vector< byte >& In, size_t Count )
		{
			auto Distance = In.size() > Count ? Count : In.size();
			return WriteBytes( In.begin(), In.begin() + Distance );
		}

		bool WriteString( const String& In, StringEncoding Enc = StringEncoding::UTF8 )
		{
			// Write string data as requested type into a vector
			std::vector< byte > Data;
			In.CopyData( Data, Enc );

			return WriteBytes( Data );
		}

		void Flush()
		{
			if( m_Alive )
			{
				m_Stream.flush();
			}
		}

		_StreamType::pos_type GetOffset()
		{
			return m_Alive ? m_Stream.tellp() : static_cast< _StreamType::pos_type >( 0 );
		}

		void SeekEnd( _StreamType::off_type Off = 0 )
		{
			if( m_Alive )
			{
				m_Stream.seekp( Off, _StreamType::end );
			}
		}

		void SeekBegin( _StreamType::off_type Off = 0 )
		{
			if( m_Alive )
			{
				m_Stream.seekp( Off, _StreamType::beg );
			}
		}

		void SeekOffset( _StreamType::off_type Off )
		{
			if( m_Alive )
			{
				m_Stream.seekp( Off );
			}
		}
	};


}