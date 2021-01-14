using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Reflection;
using System.Text.RegularExpressions;


namespace Hyperion
{
    class CommandInfo
    {
        public string Base;
        public string Usage;
        public string Description;
        public int MinArgs;
        public int MaxArgs;
        public Action< string[] > Callback;
    }

    static class Core
    {
        /*
         * Private Members
        */
        private static Dictionary< string, CommandInfo > m_Commands = new Dictionary<string, CommandInfo>();


        public static UInt32 CalculateAssetIdentifier( string inPath )
		{
            if( ( inPath?.Length ?? 0 ) == 0 )
            {
                return 0;
            }

            var lowerPath = inPath.Replace( "\\\\", "/" ).Replace( "\\", "/" );
            if( lowerPath.StartsWith( "content/" ) || lowerPath.StartsWith( "Content/" ) ) { lowerPath = lowerPath.Substring( 8 ); }

            byte[] strData = Encoding.UTF8.GetBytes( lowerPath );
            return Hashing.ELFHash( strData );
        }

        /*
         * Write Functions
        */
        public static void Write()
        { }

        public static void Write( object o )
        {
            Program.GetWindow()?.Write( o?.ToString() ?? "null" );
        }

        public static void Write( params object[] vs )
        {
            string[] strs = new string[ vs.Length ];
            for( int i = 0; i < vs.Length; i++ ) { strs[ i ] = vs[ i ]?.ToString() ?? "null"; }
            Program.GetWindow()?.Write( strs );
        }

        public static void WriteLine()
        {
            Program.GetWindow()?.Write( "\r\n" );
        }

        public static void WriteLine( object o )
        {
            Program.GetWindow()?.Write( o?.ToString() ?? "null", "\r\n" );
        }

        public static void WriteLine( params object[] vs )
        {
            string[] strs = new string[ vs.Length + 1 ];
            for( int i = 0; i < vs.Length; i++ ) { strs[ i ] = vs[ i ]?.ToString() ?? "null"; }
            strs[ vs.Length ] = "\r\n";
            Program.GetWindow()?.Write( strs );
        }


        /*
         * Command System
        */
        public static void OnCommand( string inCommand )
        {
            // First, ignore empty or whitespace only input
            inCommand = inCommand.Trim();
            if( inCommand.Length == 0 ) { return; }

            Core.WriteLine( "=> ", inCommand );

            // Now, we want to explode the text into arguments, which is a little complicated, because normally, we are going to
            // use spaces to split arguments, but, we want to allow quotes to wrap around an argument to ignore spaces
            var splitStr = new Regex( "('.*?'|\".*?\"|\\S+)", RegexOptions.Compiled ).Matches( inCommand );
            string stem = null;
            List< string > args = new List< string >();

            foreach( Match m in splitStr )
            {
                string arg = m.Value.Trim( '\'', '"' );
                if( arg.Length > 0 )
                {
                    if( stem == null )
                    {
                        stem = arg.ToLower();
                    }
                    else
                    {
                        args.Add( arg );
                    }
                }
            }

            // Now, lets validate this before continuing
            if( stem == null || stem.Length == 0 )
            {
                WriteLine( "Invalid Command: \"", inCommand, "\"" );
                return;
            }

            // Next, lets see if we can find the target command
            CommandInfo targetCommand = null;
            foreach( var cmd in m_Commands )
            {
                if( cmd.Key.Equals( stem ) )
                {
                    targetCommand = cmd.Value;
                    break;
                }
            }

            if( targetCommand == null )
            {
                WriteLine( "Command not found \"", stem, "\"" );
                return;
            }

            // Next, we want to ensure the argument count is valid
            if( targetCommand.MinArgs > 0 && args.Count < targetCommand.MinArgs )
            {
                WriteLine( "Not enough arguments for \"", targetCommand.Base, "\"! Requires at least ", targetCommand.MinArgs, " arguments" );
                WriteLine( "Example: ", targetCommand.Usage );
                return;
            }
            else if( targetCommand.MaxArgs >= 0 && args.Count > targetCommand.MaxArgs )
            {
                WriteLine( "Too many arguments for \"", targetCommand.Base, "\"! Can only use ", targetCommand.MaxArgs, " arguments at most" );
                WriteLine( "Example: ", targetCommand.Usage );
                return;
            }

            // Finally, lets call the function itself
            if ( targetCommand.Callback == null )
            {
                WriteLine( "[ERROR] Core: Command callback delegate is null for \"", targetCommand.Base, "\"!" );
            }
            else
            {
                targetCommand.Callback?.Invoke( args.ToArray() );
            }
        }

        public static bool RegisterCommand( CommandInfo inCommand )
        {
            var lowerKey = inCommand.Base.ToLower();
            if( m_Commands.ContainsKey( lowerKey ) )
            {
                WriteLine("[ERROR] Attempt to register a command \"", lowerKey, "\" but this command is already registered");
                return false;
            }

            m_Commands[ lowerKey ] = inCommand;
            return true;
        }

        private static bool InitializeCommands()
        {
            // We want to use the reflection system to find all of the commands to register automatically
            // We do it this way, so the console system can initialize before these are registered, so errors can be printed
            var assembly = Assembly.GetExecutingAssembly();
            if ( assembly != null )
            {
                foreach( var t in assembly.GetTypes() )
                {
                    var func = t.GetMethod( "RegisterConsoleCommands" );
                    if ( func != null && func.IsStatic )
                    {
                        try
                        {
                            func.Invoke( null, null );
                        }
                        catch ( Exception e )
                        {
                            WriteLine( "[ERROR] Core: Failed to register console commands in module \"", t.FullName, "\", Error: ", e.Message );
                        }
                    }

                    var otherFunc = t.GetMethod( "RegisterAssetTypes" );
                    if( otherFunc != null && otherFunc.IsStatic )
                    {
                        try
                        {
                            otherFunc.Invoke( null, null );
                        }
                        catch( Exception Ex )
                        {
                            WriteLine( "[ERROR] Core: Failed to register asset types in module \"", t.FullName, "\" Error: ", Ex.Message );
                        }
                    }
                }

                return true;
            }

            return false;
        }

        internal static Dictionary<string, CommandInfo> GetCommandList() => m_Commands;


        /*
         * Inititalization & Shutdown
        */
        public static void Initialize()
        {
            // First, we want to print the startup message
            WriteLine("========================= Startup ========================= ");
            WriteLine( "Time: ", DateTime.Now.ToString() );
            WriteLine( "Hyperion Utility Console initialiizing..." );
            WriteLine( "-> Initializing command system..." );

            if( !InitializeCommands() )
            {
                WriteLine( "---> Failed to load the command system!" );
            }
            else
            {
                WriteLine( "---> Command system initialized! ", m_Commands.Count, " commands loaded!" );
            }

            // Perform asset discovery
            AssetIdentifierCache.PerformDiscovery();
        }


        public static void Shutdown()
        {
            WriteLine( "\r\nShutting down..." );
        }

    }
}
