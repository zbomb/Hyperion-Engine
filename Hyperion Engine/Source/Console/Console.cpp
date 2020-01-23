/*==================================================================================================
	Hyperion Engine
	Source/Console/Console.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Console/Console.h"


namespace Hyperion
{
	
	/*
		Static Definitions
	*/
	std::atomic< bool > Console::m_bRunning( false );
	std::atomic< bool > Console::m_bOSOutput( false );
	std::shared_mutex Console::m_VarListMutex;
	std::map< String, std::shared_ptr< ConsoleVarInstanceBase > > Console::m_Vars;
	std::mutex Console::m_PrintMutex;


	bool Console::Start( uint32 inFlags /* = 0 */ )
	{
		// Check if were already running
		if( m_bRunning )
		{
			WriteLine( "[ERROR] Console: Failed to start.. already running!" );
			return false;
		}

		m_bRunning = true;
		// TODO: Clear cache

		if( ( inFlags & FLAG_CONSOLE_OS_OUTPUT ) == FLAG_CONSOLE_OS_OUTPUT )
		{
			m_bOSOutput = true;
		}

		m_bOSOutput = ( ( inFlags & FLAG_CONSOLE_OS_OUTPUT ) == FLAG_CONSOLE_OS_OUTPUT );
		if( m_bOSOutput )
		{
			WriteLine( "[STATUS] Console: Outputting to operating system console..." );
		}

		// Load vars
		// Load persistent vars

		return true;
	}


	bool Console::Stop()
	{
		// Check if were running
		if( !m_bRunning )
		{
			return false;
		}

		m_bRunning		= false;
		m_bOSOutput		= false;
		// Clear Cache

		return true;
	}


	bool Console::CreateVar( const std::shared_ptr< ConsoleVarInstanceBase >& inVar )
	{
		// Validate
		if( !inVar )
		{
			WriteLine( "[ERROR] Console: Attempt to add a console var, but the var is null!\n" );
			return false;
		}

		if( inVar->GetKey().IsWhitespaceOrEmpty() )
		{
			WriteLine( "[ERROR] Console: Attempt to add a console var, but the key was invalid or empty" );
			return false;
		}

		{
			std::shared_lock< std::shared_mutex > lock( m_VarListMutex );
			auto existing = m_Vars.find( inVar->GetKey() );
			if( existing != m_Vars.end() )
			{
				WriteLine( "[ERROR] Console: Attempt to add console var '", inVar->GetKey(), "' but this key already exists!" );
				return false;
			}
		}

		// TODO: Check if function with this name exists

		// Now, we need to insert into the list of console vars
		{
			std::unique_lock< std::shared_mutex > lock( m_VarListMutex );
			m_Vars[ inVar->GetKey() ] = inVar;
		}

		return true;
	}


	bool Console::SetVar( const String& inKey, const String& stringValue, bool bCallback )
	{
		if( inKey.IsWhitespaceOrEmpty() )
		{
			WriteLine( "[ERROR] Console: Failed to set cvar.. the provided key was null or whitespace" );
			return false;
		}

		// Find this var
		std::shared_ptr< ConsoleVarInstanceBase > varInst;
		{
			std::shared_lock< std::shared_mutex > lock( m_VarListMutex );
			auto entry = m_Vars.find( inKey );
			if( entry == m_Vars.end() || !entry->second )
			{
				WriteLine( "[ERROR] Console: Failed to set '", inKey, "' because no cvar could be found with this key" );
				return false;
			}

			varInst = entry->second;
		}

		if( !varInst->SetValueFromString( stringValue, bCallback ) )
		{
			WriteLine( "[ERROR] Console: Failed to set '", inKey, "' to '", stringValue, "' because that value is invalid for this cvar type!" );
			return false;
		}

		WriteLine( "Console: CVar '", inKey, "' set to '", stringValue, "'" );
		return true;
	}


	bool Console::GetVarAsString( const String& inKey, String& outValue )
	{
		if( inKey.IsWhitespaceOrEmpty() )
		{
			WriteLine( "[ERROR] Console: Failed to get cvar.. provided key was null or whitespace" );
			return false;
		}

		std::shared_ptr< ConsoleVarInstanceBase > varInst;
		{
			std::shared_lock< std::shared_mutex > lock( m_VarListMutex );

			auto entry = m_Vars.find( inKey );
			if( entry == m_Vars.end() || !entry->second )
			{
				WriteLine( "[ERROR] Console: Failed to get '", inKey, "' because this cvar couldnt be found" );
				return false;
			}

			varInst = entry->second;
		}

		outValue = varInst->GetValueAsString();
		return true;
	}
}