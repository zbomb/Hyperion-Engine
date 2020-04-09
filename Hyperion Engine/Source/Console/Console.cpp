/*==================================================================================================
	Hyperion Engine
	Source/Console/Console.cpp
	© 2019, Zachary Berry
==================================================================================================*/

#include "Hyperion/Console/Console.h"
#include "Hyperion/File/PhysicalFileSystem.h"


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
		LoadConfig();

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


	bool Console::LoadConfig()
	{
		// Check to see if the file exists
		FilePath path( String( "config.hcf" ), LocalPath::Game );
		auto f = PhysicalFileSystem::OpenFile( path, FileMode::Read );

		if( !f || !f->IsValid() )
		{
			Console::WriteLine( "[Status] Console: No config file detected!" );
			return true;
		}
		
		DataReader Reader( f );

		// Now, we want to loop through the file, line by line and read in all console variables
		std::vector< byte > fileData;
		if( Reader.ReadBytes( fileData, Reader.Size() ) != DataReader::ReadResult::Success )
		{
			Console::WriteLine( "[WARNING] Console: Failed to read config file in! ({GAME}/config.hcf" );
			return false;
		}

		// The goal here is to split the file into seperate lines
		auto It			= fileData.begin();
		auto lineBegin	= It;

		while( true )
		{
			// First, check if we hit the end of the line, or end of the file
			bool bLineEnd = It == fileData.end();
			if( bLineEnd || *It == '\n' )
			{
				// Ignore empty lines
				if( It != lineBegin )
				{
					ProcessConfigLine( String( std::vector< byte >( lineBegin, It ), StringEncoding::ASCII ) );
				}

				// Check if we hit the end of the file
				if( bLineEnd ) { break; }
				
				// Now, we want to mark the next character as the begining of the next line
				lineBegin = It;
				lineBegin++;
			}

			It++;
		}

		return true;
	}


	void Console::ProcessConfigLine( const String& inStr )
	{
		if( !inStr.IsWhitespaceOrEmpty() )
		{
			// We want to split the line by the first space found in the string
			// The first part of the line becomes the console var name
			// The second part of the line becomes the value
			auto spacePos = inStr.Find( (Char) ' ' );
			
			if( spacePos == inStr.end() )
			{
				Console::WriteLine( "[WARNING] Console: Invalid line found in config.. there is no valid value! (", inStr, ")" );
				return;
			}

			auto valueStart = spacePos;
			valueStart++;

			if( valueStart == inStr.end() )
			{
				Console::WriteLine( "[WARNING] Console: Invalid line found in config.. there is no valid value! (", inStr, ")" );
				return;
			}

			String varName( inStr.begin(), spacePos );
			String varValue( valueStart, inStr.end() );

			// Now, we can process this console value
			SetVar( varName, varValue, false );
		}
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