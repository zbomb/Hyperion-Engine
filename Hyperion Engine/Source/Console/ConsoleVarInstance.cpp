/*==================================================================================================
	Hyperion Engine
	Source/Console/ConsoleVarInstance.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Console/ConsoleVarInstance.h"
#include "Hyperion/Core/ThreadManager.h"



namespace Hyperion
{

	/*-------------------------------------------------------------------------
		ConsoleVarInstance< uint32 >
	-------------------------------------------------------------------------*/

	ConsoleVarInstance< uint32 >::ConsoleVarInstance( const String& inKey, const String& inDescription, uint32 inDefault, uint32 inMin, uint32 inMax,
													  std::function< void( uint32 ) > inCallback, const std::string& callbackThread )
		: m_Key( inKey ), m_Description( inDescription ), m_Value( inDefault ), m_Min( inMin ), m_Max( inMax ), m_Callback( inCallback ), m_CallbackThread( callbackThread )
	{
	}

	String ConsoleVarInstance< uint32 >::GetValueAsString()
	{
		std::shared_lock< std::shared_mutex > lock( m_Mutex );
		return ToString( m_Value );
	}

	bool ConsoleVarInstance< uint32 >::SetValueFromString( const String& inStr, bool bCallback )
	{
		// We need to convert this string into a number
		uint32 val = 0;
		if( !String::ToUInt( inStr, val ) )
		{
			return false;
		}

		return SetValue( val, bCallback );
	}


	bool ConsoleVarInstance< uint32 >::SetValue( uint32 inValue, bool bCallback )
	{
		// Check if this value is valid
		if( m_Min != 0 && m_Max != 0 )
		{
			if( inValue < m_Min || inValue > m_Max )
			{
				return false;
			}
		}

		// Lock mutex and update value
		{
			std::unique_lock< std::shared_mutex > lock( m_Mutex );
			m_Value = inValue;
		}

		// Inject the callback onto the target thread
		if( m_Callback && bCallback )
		{
			ThreadManager::CreateTask< void >( std::bind( m_Callback, inValue ), m_CallbackThread );
		}

		return true;
	}

	uint32 ConsoleVarInstance< uint32 >::GetValue()
	{
		std::shared_lock< std::shared_mutex > lock( m_Mutex );
		return m_Value;
	}


	/*-------------------------------------------------------------------------
		ConsoleVarInstance< int32 >
	-------------------------------------------------------------------------*/

	ConsoleVarInstance< int32 >::ConsoleVarInstance( const String& inKey, const String& inDescription, int32 inDefault, int32 inMin, int32 inMax,
													  std::function< void( int32 ) > inCallback, const std::string& callbackThread )
		: m_Key( inKey ), m_Description( inDescription ), m_Value( inDefault ), m_Min( inMin ), m_Max( inMax ), m_Callback( inCallback ), m_CallbackThread( callbackThread )
	{
	}

	String ConsoleVarInstance< int32 >::GetValueAsString()
	{
		std::shared_lock< std::shared_mutex > lock( m_Mutex );
		return ToString( m_Value );
	}

	bool ConsoleVarInstance< int32 >::SetValueFromString( const String& inStr, bool bCallback )
	{
		// We need to convert this string into a number
		int32 val = 0;
		if( !String::ToInt( inStr, val ) )
		{
			return false;
		}

		return SetValue( val, bCallback );
	}


	bool ConsoleVarInstance< int32 >::SetValue( int32 inValue, bool bCallback )
	{
		// Check if this value is valid
		if( m_Min != 0 && m_Max != 0 )
		{
			if( inValue < m_Min || inValue > m_Max )
			{
				return false;
			}
		}

		// Lock mutex and update value
		{
			std::unique_lock< std::shared_mutex > lock( m_Mutex );
			m_Value = inValue;
		}

		// Inject the callback onto the target thread
		if( m_Callback && bCallback )
		{
			ThreadManager::CreateTask< void >( std::bind( m_Callback, inValue ), m_CallbackThread );
		}

		return true;
	}

	int32 ConsoleVarInstance< int32 >::GetValue()
	{
		std::shared_lock< std::shared_mutex > lock( m_Mutex );
		return m_Value;
	}


	/*-------------------------------------------------------------------------
		ConsoleVarInstance< String >
	-------------------------------------------------------------------------*/


	ConsoleVarInstance< String >::ConsoleVarInstance( const String& inKey, const String& inDescription, const String& inDefault, std::function< void( const String& ) > inCallback, const std::string& inCallbackThread )
		: m_Key( inKey ), m_Value( inDefault ), m_Callback( inCallback ), m_CallbackThread( inCallbackThread ), m_Description( inDescription )
	{
	}

	String ConsoleVarInstance< String >::GetValueAsString()
	{
		std::shared_lock< std::shared_mutex > lock( m_Mutex );
		return m_Value;
	}

	bool ConsoleVarInstance< String >::SetValueFromString( const String& inStr, bool bCallback )
	{
		{
			std::unique_lock< std::shared_mutex > lock( m_Mutex );
			m_Value = inStr;
		}

		if( m_Callback && bCallback )
		{
			ThreadManager::CreateTask< void >( std::bind( m_Callback, inStr ), m_CallbackThread );
		}

		return true;
	}

	String ConsoleVarInstance< String >::GetValue()
	{
		return GetValueAsString();
	}

	bool ConsoleVarInstance< String >::SetValue( const String& inStr, bool bCallback )
	{
		return SetValueFromString( inStr, bCallback );
	}


	/*-------------------------------------------------------------------------
		ConsoleVarInstance< float >
	-------------------------------------------------------------------------*/

	ConsoleVarInstance< float >::ConsoleVarInstance( const String& inKey, const String& inDescription, float inDefault, float inMin, float inMax,
													  std::function< void( float ) > inCallback, const std::string& callbackThread )
		: m_Key( inKey ), m_Description( inDescription ), m_Value( inDefault ), m_Min( inMin ), m_Max( inMax ), m_Callback( inCallback ), m_CallbackThread( callbackThread )
	{
	}

	String ConsoleVarInstance< float >::GetValueAsString()
	{
		std::shared_lock< std::shared_mutex > lock( m_Mutex );
		return ToString( m_Value );
	}

	bool ConsoleVarInstance< float >::SetValueFromString( const String& inStr, bool bCallback )
	{
		// We need to convert this string into a number
		float val = 0.f;
		if( !String::ToFloat( inStr, val ) )
		{
			return false;
		}

		return SetValue( val, bCallback );
	}


	bool ConsoleVarInstance< float >::SetValue( float inValue, bool bCallback )
	{
		// Check if this value is valid
		if( m_Min != 0.f && m_Max != 0.f )
		{
			if( inValue < m_Min || inValue > m_Max )
			{
				return false;
			}
		}

		// Lock mutex and update value
		{
			std::unique_lock< std::shared_mutex > lock( m_Mutex );
			m_Value = inValue;
		}

		// Inject the callback onto the target thread
		if( m_Callback && bCallback )
		{
			ThreadManager::CreateTask< void >( std::bind( m_Callback, inValue ), m_CallbackThread );
		}

		return true;
	}

	float ConsoleVarInstance< float >::GetValue()
	{
		std::shared_lock< std::shared_mutex > lock( m_Mutex );
		return m_Value;
	}


}