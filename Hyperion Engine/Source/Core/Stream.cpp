/*==================================================================================================
	Hyperion Engine
	Source/Core/Stream.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Core/Stream.h"



namespace Hyperion
{

	/*============================================================================================
		BinaryStreamBase Class
	============================================================================================*/

	/*
		BinaryStreamBase Constructor
	*/
	BinaryStreamBase::BinaryStreamBase( IDataBuffer& In )
		: m_Target( In )
	{}

	/*
		BinaryStreamBase Destructor
	*/
	BinaryStreamBase::~BinaryStreamBase()
	{}

	/*
		BinaryStreamBase::Begin
	*/
	BinaryStreamBase::iterator BinaryStreamBase::Begin() const
	{
		return iterator( m_Target.GetBuffer().begin() );
	}

	/*
		BinaryStreamBase::End
	*/
	BinaryStreamBase::iterator BinaryStreamBase::End() const
	{
		return iterator( m_Target.GetBuffer().begin() );
	}

	/*
		BinaryStreamBase::At
	*/
	byte BinaryStreamBase::At( size_t Index ) const
	{
		auto& buffer = m_Target.GetBuffer();

		if( buffer.size() <= Index )
		{
			// Out of range!
			std::cout << "[ERROR] BinaryStream: Attempt to access out of range value!\n";
			return 0;
		}

		return buffer.at( Index );
	}

	/*
		BinaryStreamBase::Copy
	*/
	void BinaryStreamBase::Copy( iterator Begin, iterator End, std::vector< byte >& Output, std::vector< byte >::iterator Where ) const
	{
		Output.insert( Where, Begin._get(), End._get() );
	}

	/*
		BinaryStreamBase::Copy
	*/
	void BinaryStreamBase::Copy( iterator Begin, iterator End, std::vector< byte >& Output ) const
	{
		Copy( Begin, End, Output, Output.end() );
	}

	/*
		BinaryStreamBase::Copy
	*/
	void BinaryStreamBase::Copy( std::vector< byte >& Output, std::vector< byte >::iterator Where ) const
	{
		auto& buffer = m_Target.GetBuffer();
		Output.insert( Where, buffer.begin(), buffer.end() );
	}

	/*
		BinaryStreamBase::Copy
	*/
	void BinaryStreamBase::Copy( std::vector< byte >& Output ) const
	{
		Copy( Output, Output.end() );
	}

	/*
		BinaryStreamBase::Copy
	*/
	void BinaryStreamBase::Copy( size_t Start, size_t Count, std::vector< byte >& Output, std::vector< byte >::iterator Where ) const
	{
		auto& buffer = m_Target.GetBuffer();
		if( buffer.size() < ( Start + Count ) )
		{
			std::cout << "[ERROR] BinaryStream: Attempt to copy, but the request was out of range!\n";
			return;
		}

		auto begin = buffer.begin();
		std::advance( begin, Start );

		auto end = begin;
		std::advance( end, Count );

		Output.insert( Where, begin, end );
	}

	/*
		BinaryStreamBase::Copy
	*/
	void BinaryStreamBase::Copy( size_t Start, size_t Count, std::vector< byte >& Output ) const
	{
		return Copy( Start, Count, Output, Output.end() );
	}

	/*
		BinaryStreamBase::Size
	*/
	size_t BinaryStreamBase::Size() const
	{
		return m_Target.GetBuffer().size();
	}


	/*============================================================================================
		BinaryWriter Class
	============================================================================================*/

	/*
		BinaryWriter::InsertAt
	*/
	BinaryWriter::iterator BinaryWriter::InsertAt( iterator Where, byte In )
	{
		return iterator( m_Target.GetBuffer().insert( Where._get(), In ) );
	}

	/*
		BinaryWriter::InsertAt
	*/
	BinaryWriter::iterator BinaryWriter::InsertAt( iterator Where, std::initializer_list< byte > In )
	{
		return iterator( m_Target.GetBuffer().insert( Where._get(), In ) );
	}

	/*
		BinaryWriter::InsertAt
	*/
	BinaryWriter::iterator BinaryWriter::InsertAt( iterator Where, std::vector< byte >::const_iterator Begin, std::vector< byte >::const_iterator End )
	{
		return iterator( m_Target.GetBuffer().insert( Where._get(), Begin, End ) );
	}

	/*
		BinaryWriter::InsertAt
	*/
	BinaryWriter::iterator BinaryWriter::InsertAt( iterator Where, iterator Begin, iterator End )
	{
		return iterator( m_Target.GetBuffer().insert( Where._get(), Begin._get(), End._get() ) );
	}

	/*
		BinaryWriter::InsertFront
	*/
	void BinaryWriter::InsertFront( byte In )
	{
		InsertAt( Begin(), In );
	}

	/*
		BinaryWriter::InsertFront
	*/
	void BinaryWriter::InsertFront( std::initializer_list< byte > In )
	{
		InsertAt( Begin(), In );
	}

	/*
		BinaryWriter::InsertFront
	*/
	void BinaryWriter::InsertFront( std::vector< byte >::const_iterator Start, std::vector< byte >::const_iterator End )
	{
		InsertAt( Begin(), Start, End );
	}

	/*
		BinaryWriter::InsertFront
	*/
	void BinaryWriter::InsertFront( iterator Start, iterator End )
	{
		InsertAt( Begin(), Start, End );
	}

	/*
		BinaryWriter::InsertBack
	*/
	void BinaryWriter::InsertBack( byte In )
	{
		InsertAt( End(), In );
	}

	/*
		BinaryWriter::InsertBack
	*/
	void BinaryWriter::InsertBack( std::initializer_list< byte > In )
	{
		InsertAt( End(), In );
	}

	/*
		BinaryWriter::InsertBack
	*/
	void BinaryWriter::InsertBack( std::vector< byte >::const_iterator Start, std::vector< byte >::const_iterator vecEnd )
	{
		InsertAt( End(), Start, vecEnd );
	}

	/*
		BinaryWriter::InsertBack
	*/
	void BinaryWriter::InsertBack( iterator Start, iterator binEnd )
	{
		InsertAt( End(), Start, binEnd );
	}

	/*
		BinaryWriter::Erase
	*/
	BinaryWriter::iterator BinaryWriter::Erase( iterator Begin, iterator End )
	{
		return iterator( m_Target.GetBuffer().erase( Begin._get(), End._get() ) );
	}

	/*
		BinaryWriter::Erase
	*/
	BinaryWriter::iterator BinaryWriter::Erase( iterator Where )
	{
		return iterator( m_Target.GetBuffer().erase( Where._get() ) );
	}

	/*
		BinaryWriter::Erase
	*/
	BinaryWriter::iterator BinaryWriter::Erase( size_t Index, size_t Count )
	{
		auto& buffer = m_Target.GetBuffer();

		if( buffer.size() < ( Index + Count ) )
		{
			std::cout << "[ERROR] BinaryStream: Attempt to erase out of bounds range!\n";
			return End();
		}

		auto begin = buffer.begin();
		std::advance( begin, Index );

		auto end = begin;
		std::advance( end, Count );

		return iterator( buffer.erase( begin, end ) );
	}







}