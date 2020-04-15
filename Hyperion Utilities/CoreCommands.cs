using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Hyperion
{
    class CoreCommands
    {

        public static void RegisterConsoleCommands()
        {
            CommandInfo helpCommand = new CommandInfo
            {
                Base = "help",
                Usage = "help | help [command/search string]",
                Description = "Prints info about command(s). No argument => List of all commands. One Argument => List of commands that start with specified string (if only one found, prints all info about it)",
                MinArgs = 0,
                MaxArgs = 1,
                Callback = (a) => HelpCommand( a )
            };

            CommandInfo clearCommand = new CommandInfo
            {
                Base = "clear",
                Usage = "clear",
                Description = "Clears the text in the console",
                MinArgs = 0,
                MaxArgs = 0,
                Callback = (a) => ClearConsole()
            };

            CommandInfo closeCommand = new CommandInfo
            {
                Base = "close",
                Usage = "close",
                Description = "Exits the application",
                MinArgs = 0,
                MaxArgs = 0,
                Callback = (a) => CloseConsole()
            };

            CommandInfo exitCommand = new CommandInfo
            {
                Base = "exit",
                Usage = "exit",
                Description = "Exits the application",
                MinArgs = 0,
                MaxArgs = 0,
                Callback = (a) => CloseConsole()
            };

            Core.RegisterCommand( helpCommand );
            Core.RegisterCommand( clearCommand );
            Core.RegisterCommand( closeCommand );
            Core.RegisterCommand( exitCommand );
        }


        public static void HelpCommand( string[] inArgs )
        {
            var cmdList = Core.GetCommandList();

            if( inArgs.Length == 0 )
            {
                // No arguments, so print full command list
                Core.WriteLine( "Full console command list:" );

                foreach( var cmd in cmdList )
                {
                    Core.WriteLine( "\t", cmd.Value.Usage );
                }

                Core.WriteLine();
            }
            else
            {
                // We want to search for commands using the specified string
                string search = inArgs[ 0 ].ToLower();
                List< CommandInfo > results = new List< CommandInfo >();

                foreach( var cmd in cmdList )
                {
                    if( cmd.Key.StartsWith( search ) )
                    {
                        results.Add( cmd.Value );
                    }
                }

                if( results.Count == 0 )
                {
                    // No results found
                    Core.WriteLine( "No commands found starting with \"", search, "\"" );
                }
                else if( results.Count == 1 )
                {
                    // One result found so we want to print full info about this command
                    var target = cmdList.First();

                    Core.WriteLine( "Info about command \"", target.Key, "\":" );
                    Core.WriteLine( "\tUsage: ", target.Value.Usage );
                    Core.WriteLine( "\tDescription: ", target.Value.Description );
                    Core.WriteLine( "\tMin/Max Args: ", target.Value.MinArgs, "/", target.Value.MaxArgs );
                    Core.WriteLine();
                }
                else
                {
                    // Multiple results found, print out the ones we have
                    Core.WriteLine( "Console commands found for \"", search, "\":" );

                    foreach( var cmd in results )
                    {
                        Core.WriteLine( "\t", cmd.Usage );
                    }

                    Core.WriteLine();
                }
            }
        }


        public static void ClearConsole()
        {
            Program.GetWindow()?.Clear();
            Core.WriteLine( "================= Console Cleared ==================" );
        }


        public static void CloseConsole()
        {
            Program.GetWindow()?.Close();
        }

    }
}
