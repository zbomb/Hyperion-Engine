/*==================================================================================================
	Hyperion Engine
	Include/Hyperion/Core/Stream.h
	© 2019, Zachary Berry
==================================================================================================*/

#pragma once

#include <vector>
#include "Hyperion/Hyperion.h"
#include "Hyperion/Core/Library/UTF8.hpp"
#include "Hyperion/Core/Library/UTF16.hpp"
#include "Hyperion/Core/String.h"


namespace Hyperion
{
	template< typename _Iter >
	class _wrap_iterator_const
	{
	protected:

		_Iter m_Iter;

	public:

		/*
			Iterator Traits
		*/
		using difference_type		= typename _Iter::difference_type;
		using value_type			= typename _Iter::value_type;
		using pointer				= typename const value_type*;
		using reference				= typename const value_type&;
		using iterator_category		= typename _Iter::iterator_category;

		/*
			Constructors
		*/
		_wrap_iterator_const( const _Iter& Base )
			: m_Iter( Base )
		{}

		_wrap_iterator_const( const _wrap_iterator_const& Other )
			: m_Iter( Other.m_Iter )
		{}

		void operator=( const _wrap_iterator_const& Other )
		{
			m_Iter = Other.m_Iter;
		}

		/*
			Increment/Decrement/Add/Subtract
		*/
		_wrap_iterator_const& operator+=( const difference_type Offset )	{ m_Iter += Offset; return *this; }
		_wrap_iterator_const& operator++()									{ m_Iter++; return *this; }
		_wrap_iterator_const operator++( int )								{ _wrap_iterator_const ret( *this ); ++( *this ); return ret; }
		_wrap_iterator_const& operator+( const difference_type Offset )		{ _wrap_iterator_const ret( *this ); return ret += Offset; }
		_wrap_iterator_const& operator-=( const difference_type Offset )	{ m_Iter -= Offset; return *this; }
		_wrap_iterator_const& operator--()									{ m_Iter--; return *this; }
		_wrap_iterator_const operator--( int )								{ _wrap_iterator_const ret( *this ); --( *this ); return ret; }
		_wrap_iterator_const operator-( const difference_type Offset )		{ _wrap_iterator_const ret( *this ); return ret -= Offset; }

		/*
			Range
		*/
		difference_type operator-( const _wrap_iterator_const& Other )		{ return m_Iter - Other.m_Iter; }

		/*
			Equality Checks
		*/
		bool operator==( const _wrap_iterator_const& Other ) const	{ return m_Iter == Other.m_Iter; }
		bool operator!=( const _wrap_iterator_const& Other ) const	{ return !( *this == Other.m_Iter ); }
		bool operator<( const _wrap_iterator_const& Other ) const	{ return m_Iter < Other.m_Iter; }
		bool operator<=( const _wrap_iterator_const& Other ) const	{ return m_Iter <= Other.m_Iter; }
		bool operator>( const _wrap_iterator_const& Other ) const	{ return m_Iter > Other.m_Iter; }
		bool operator>=( const _wrap_iterator_const& Other ) const	{ return m_Iter >= Other.m_Iter; }

		_Iter _get() { return m_Iter; }

	};

	template< typename _Iter >
	class _wrap_iterator : public _wrap_iterator_const< _Iter >
	{
	protected:

		using _base = _wrap_iterator_const;

	public:

		/*
			Iterator Traits
		*/
		using difference_type		= typename _Iter::difference_type;
		using value_type			= typename _Iter::value_type;
		using pointer				= typename value_type*;
		using reference				= typename value_type&;
		using iterator_category		= typename _Iter::iterator_category;

		/*
			Constructors
		*/
		_wrap_iterator() 
		{}

		_wrap_iterator( const _Iter& Base )
			: _base( Base )
		{}

		_wrap_iterator( const _wrap_iterator& Other )
			: m_Iter( Other.m_Iter )
		{}

		void operator=( const _wrap_iterator& Other )
		{
			m_Iter = Other.m_Iter;
		}

		/*
			Underlying Value Access
		*/
		inline reference operator*()		{ return *m_Iter; }
		inline pointer operator->()			{ return m_Iter.operator->(); }

	};


	enum class BufferMode
	{
		Read,
		Write,
		ReadWrite
	};


	/*----------------------------------------------------------------------------------------
		IDataBuffer Interface
		* Used for things like files, and memory buffers
	----------------------------------------------------------------------------------------*/
	class IDataBuffer
	{

	public:

		virtual std::vector< byte >& GetBuffer() = 0;
		virtual BufferMode GetMode() = 0;

	};

	/*----------------------------------------------------------------------------------------
		BinaryStreamBase Class
	----------------------------------------------------------------------------------------*/
	class BinaryStreamBase
	{

	protected:

		IDataBuffer& m_Target;

	public:

		BinaryStreamBase() = delete;
		BinaryStreamBase( IDataBuffer& );
		~BinaryStreamBase();

		using iterator = _wrap_iterator_const< std::vector< byte >::const_iterator >;

		iterator Begin() const;
		iterator End() const;

		byte At( size_t Index ) const;
		void Copy( std::vector< byte >& Target ) const;
		void Copy( std::vector< byte >& Target, std::vector< byte >::iterator Where ) const;
		void Copy( iterator Start, iterator End, std::vector< byte >& Target ) const;
		void Copy( iterator Start, iterator End, std::vector< byte >& Target, std::vector< byte >::iterator Where ) const;
		void Copy( size_t StartIndex, size_t Count, std::vector< byte >& Target ) const;
		void Copy( size_t StartIndex, size_t Count, std::vector< byte >& Target, std::vector< byte >::iterator Where ) const;
		size_t Size() const;

	};

	/*----------------------------------------------------------------------------------------
		BinaryReader Class
	----------------------------------------------------------------------------------------*/
	class BinaryReader : public BinaryStreamBase
	{

	public:

		BinaryReader() = delete;

		BinaryReader( IDataBuffer& In )
			: BinaryStreamBase( In )
		{}

		~BinaryReader()
		{}

	};

	/*----------------------------------------------------------------------------------------
		BinaryWriter Class
	----------------------------------------------------------------------------------------*/
	class BinaryWriter : public BinaryStreamBase
	{

	public:

		BinaryWriter() = delete;

		BinaryWriter( IDataBuffer& In )
			: BinaryStreamBase( In )
		{}

		~BinaryWriter()
		{}

		iterator InsertAt( iterator Where, byte In );
		iterator InsertAt( iterator Where, std::initializer_list< byte > In );
		iterator InsertAt( iterator Where, std::vector< byte >::const_iterator, std::vector< byte >::const_iterator );
		iterator InsertAt( iterator Where, iterator, iterator );

		void InsertFront( byte );
		void InsertFront( std::initializer_list< byte > );
		void InsertFront( std::vector< byte >::const_iterator, std::vector< byte >::const_iterator );
		void InsertFront( iterator, iterator );

		void InsertBack( byte );
		void InsertBack( std::initializer_list< byte > );
		void InsertBack( std::vector< byte >::const_iterator, std::vector< byte >::const_iterator );
		void InsertBack( iterator, iterator );

		iterator Erase( size_t Index, size_t Count );
		iterator Erase( iterator, iterator );
		iterator Erase( iterator );

	};

	/*----------------------------------------------------------------------------------------
		TextStreamBase Class
	----------------------------------------------------------------------------------------*/
	class TextStreamBase
	{

	protected:

		IDataBuffer& m_Target;

	public:

		TextStreamBase() = delete;
		TextStreamBase( IDataBuffer& );
		~TextStreamBase();

		class char_iterator
		{

		protected:
		
			StringEncoding m_Encoding;
			std::vector< byte >* m_Target;
			std::vector< byte >::const_iterator m_Position;
			std::vector< byte >::const_iterator m_NextChar;
			bool m_LittleEndian;
			uint32 m_CodePoint;

		public:

			/*
				Iterator Traits
			*/
			using difference_type		= size_t;
			using value_type			= Char;
			using pointer				= const Char*;
			using reference				= const Char&;
			using iterator_category		= std::forward_iterator_tag;

			char_iterator()
				: m_Encoding( StringEncoding::UTF8 ), m_Target( nullptr ), m_LittleEndian( false )
			{}

			char_iterator( std::vector< byte >& v, std::vector< byte >::const_iterator i, StringEncoding e, bool bLittleEndian = false )
				: m_Position( i ), m_Target( std::addressof( v ) ), m_Encoding( e ), m_LittleEndian( bLittleEndian )
			{}

			~char_iterator()
			{
				m_Target = nullptr;
			}


			void Advance()
			{
				// Check if were invalid or hit the end of the data buffer (or invalid string encoding)
				if( !m_Target || m_Position == m_Target->end() || m_Encoding == StringEncoding::AUTO )
					return;

				// Now.. we need to read the next character from the source buffer using the provided encoding
				if( m_Encoding == StringEncoding::ASCII )
				{
					// First move the current position up to the next character
					m_Position = m_NextChar;

					// Next, read in this character
					m_CodePoint = static_cast< Char >( *m_Position );

					// Finally.. advance m_NextChar to the start of the next character
					m_NextChar++;
				}
				else if( m_Encoding == StringEncoding::UTF8 )
				{
					Encoding::UTF8::ReadNextCode( *m_Target, m_Position, m_CodePoint );
				}

			}



		};





	};


	/*----------------------------------------------------------------------------------------
		TextReader Class
	----------------------------------------------------------------------------------------*/
	class TextReader
	{

	protected:

		IDataBuffer& m_Target;

	public:

		TextReader() = delete;
		TextReader( IDataBuffer&, StringEncoding );
		~TextReader();

		class iterator
		{

		};

		iterator Begin() const;
		iterator End() const;
	};

	/*----------------------------------------------------------------------------------------
		TextWriter Class
	----------------------------------------------------------------------------------------*/
	class TextWriter
	{

	protected:

		IDataBuffer& m_Target;

	public:

		TextWriter() = delete;
		TextWriter( IDataBuffer& );
		~TextWriter();

	};

}